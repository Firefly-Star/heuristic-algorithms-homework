// 启发函数：估计"从某格到终点还要花多少代价"，给 A* 指方向
//
// ============================ 两条性质 ============================
//
// 可采纳（admissible）：  h(n) ≤ h*(n)，对所有 n 都**不高估**最优剩余代价
//      ⇒ A* 保证返回最优解
//        前提有三条，缺一不可：
//          1. 终点在**弹出时**判定，不是在生成时判定
//          2. 实现允许重扩展（不能用硬 closed 集合）
//          3. h(goal) = 0 且 h(n) ≥ 0
//
// 一致（consistent）：    h(n) ≤ c(n,n') + h(n')，h 的落差不超过一步的真实代价
//      ⇒ 蕴含可采纳；并且额外保证每格最多扩展一次、f 沿路径单调不减
//
// 可采纳性管「对不对」，h 的大小管「快不快」：
// 两个都可采纳时，处处更大的那个永远不会扩展更多节点（支配关系）。
// 所以在这个文件里，曼哈顿在四邻域下是最强的，其余几何距离只会更慢。
//
// ======================= 和移动模型是绑死的 =======================
// 哪个 h「对」取决于 astar.h 里的邻域和代价：
//     四邻域 + 单位代价        → 真实距离就是曼哈顿，曼哈顿精确且最强
//     八邻域 + 对角代价 √2     → Octile 精确；曼哈顿变成**高估**，不再可采纳
// 下面每个函数都标了它适配哪个模型。
//
// ⚠ 依赖说明：几何启发只需要 coord.h，但预计算型的 DistanceField/ByName
//   要看地图，所以这个头文件也 include 了 grid.h。
//   astar.h 依然只认 coord.h，不认识这个文件、也不认识 Grid。
#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "coord.h"
#include "grid.h"

namespace heuristic {

// ============================ 几何启发 ============================

// 曼哈顿距离（L1）
//   四邻域 + 单位代价：可采纳 ✅ 一致 ✅，而且就是真实距离本身 —— 最强，默认
//   八邻域 + 对角 √2 ：不可采纳 ❌（会高估，实测能让 A* 返回次优解）
inline auto Manhattan(Coordinate goal)
{
    return [goal](Coordinate c) {
        return static_cast<float>(std::abs(int(c.first)  - int(goal.first))
                                + std::abs(int(c.second) - int(goal.second)));
    };
}

// 欧几里得距离（L2）
//   任何模型：可采纳 ✅ 一致 ✅（直线距离不可能超过任何一条路径的长度）
//   代价：很松，扩展数明显多于曼哈顿（实测 313 vs 255）
inline auto Euclidean(Coordinate goal)
{
    return [goal](Coordinate c) {
        double dr = double(c.first)  - double(goal.first);
        double dc = double(c.second) - double(goal.second);
        return static_cast<float>(std::sqrt(dr * dr + dc * dc));
    };
}

// 切比雪夫距离（L∞）
//   四邻域 + 单位代价：      可采纳 ✅ 一致 ✅（但比曼哈顿松）
//   八邻域 + 对角代价 1：    可采纳 ✅，且精确
//   八邻域 + 对角代价 √2：   可采纳 ✅（L∞ ≤ Octile），但比 Octile 松
inline auto Chebyshev(Coordinate goal)
{
    return [goal](Coordinate c) {
        return static_cast<float>(std::max(std::abs(int(c.first)  - int(goal.first)),
                                          std::abs(int(c.second) - int(goal.second))));
    };
}

// 对角距离 / Octile：hi + (√2 − 1)·lo
//   四邻域 + 单位代价：   可采纳 ✅ 一致 ✅（但比曼哈顿松，实测 309 vs 255）
//   八邻域 + 对角代价 √2：可采纳 ✅ 且**精确** —— 这个模型下的最佳选择
inline auto Octile(Coordinate goal)
{
    return [goal](Coordinate c) {
        double dr = std::abs(double(c.first)  - double(goal.first));
        double dc = std::abs(double(c.second) - double(goal.second));
        double lo = std::min(dr, dc), hi = std::max(dr, dc);
        return static_cast<float>(hi + (std::sqrt(2.0) - 1.0) * lo);
    };
}

// 恒零
//   可采纳 ✅ 一致 ✅，但等于完全没有启发 —— A* 退化成 Dijkstra
//   用处：作为「扩展数上界」的基线（实测扩展了 99.7% 的可通过格）
inline auto Zero()
{
    return [](Coordinate) { return 0.0f; };
}

// ============================== 组合 ==============================

// 加权：把任意启发函数乘一个系数 w
//   w = 1   等价于原函数
//   w > 1   **不再可采纳** ❌（会高估），最优性保证消失
//           换来的是扩展数大幅下降；理论上结果代价 ≤ w × 最优代价
//   w 很大  退化成贪婪最佳优先搜索（只看 h，不看 g）
//   ⚠ w > 1 同时破坏一致性，同一格可能被扩展多次 ——
//     这正是 astar.h 用「过期判断」而不是 closed 标记的原因
template<typename Base>
inline auto Weighted(float w, Base base)
{
    return [w, base](Coordinate c) { return w * base(c); };
}

// ============================ 预计算型 ============================

// 从 from 出发 BFS，得到每格到 from 的最短步数（不可达 = inf）
//   ⚠ 这里用的是**四邻域 + 单位代价**，必须和 astar.h 里的邻域保持一致；
//     哪天把邻域换成八邻域，这里也要跟着换成对应的 Dijkstra。
inline std::vector<float> DistanceField(Grid const& g, Coordinate from)
{
    const float INF = std::numeric_limits<float>::infinity();
    std::vector<float> dist(g.n * g.n, INF);
    if (!g.walkable(from)) return dist;

    static const int dx[4] = { -1, 1, 0, 0 };
    static const int dy[4] = {  0, 0,-1, 1 };

    std::vector<Coordinate> q;
    q.reserve(g.n * g.n);
    dist[g.index(from)] = 0.0f;
    q.push_back(from);

    for (size_t i = 0; i < q.size(); ++i)
    {
        Coordinate c = q[i];
        for (int k = 0; k < 4; ++k)
        {
            long long nr = (long long)c.first  + dx[k];
            long long nc = (long long)c.second + dy[k];
            if (nr < 0 || nc < 0 || nr >= (long long)g.n || nc >= (long long)g.n) continue;
            Coordinate nb{ (size_t)nr, (size_t)nc };
            if (!g.walkable(nb) || dist[g.index(nb)] < INF) continue;
            dist[g.index(nb)] = dist[g.index(c)] + 1.0f;
            q.push_back(nb);
        }
    }
    return dist;
}

// 查表型启发：直接返回到终点的**真实**剩余距离
//   可采纳 ✅（处处取等号，是最强的可采纳启发）一致 ✅
//   扩展数最少（实测 190 vs 曼哈顿 255）—— 这就是启发函数的上限，
//   再强的可采纳启发也不可能比它扩展更少。代价只是一次反向 BFS。
inline auto Table(std::vector<float> field, size_t n)
{
    return [field = std::move(field), n](Coordinate c) -> float {
        return field[c.first * n + c.second];
    };
}

// ====================== 按名字构造（给命令行用）======================

// 认识的名字：manhattan / euclidean / chebyshev / octile / zero / perfect
// 可以加权重后缀，例如 manhattan:1.5
// 名字不认识时返回空的 std::function
inline std::function<float(Coordinate)> ByName(std::string const& spec, Coordinate goal, Grid const& g)
{
    std::string name = spec;
    float w = 1.0f;

    auto colon = spec.find(':');
    if (colon != std::string::npos)
    {
        name = spec.substr(0, colon);
        try { w = std::stof(spec.substr(colon + 1)); }
        catch (...) { return {}; }
    }

    std::function<float(Coordinate)> h;
    if      (name == "manhattan") h = Manhattan(goal);
    else if (name == "euclidean") h = Euclidean(goal);
    else if (name == "chebyshev") h = Chebyshev(goal);
    else if (name == "octile")    h = Octile(goal);
    else if (name == "zero")      h = Zero();
    else if (name == "perfect")   h = Table(DistanceField(g, goal), g.n);
    else return {};

    if (w != 1.0f) h = Weighted(w, h);
    return h;
}

}  // namespace heuristic

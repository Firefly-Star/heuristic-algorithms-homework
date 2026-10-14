// 算法：A* 搜索
//
// 这个头文件不认识 Grid、不做任何文件读写、也不知道启发函数具体是什么，
// 它只认 map + N + start + end + h + visitor 这几样东西。
#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

#include "coord.h"

// ---- 访问者 hook ----
// 默认观察者：两个空函数体，编译期就被优化掉。
// 所以不传 visitor 时，Astar 的行为和以前完全一致。
struct SilentVisitor
{
    void onPush  (Coordinate, float) const {}
    void onExpand(Coordinate, float) const {}
};

// 把搜索过程记下来的观察者（'P' = 刚被发现进入边界，'E' = 正在被扩展）
struct TraceVisitor
{
    std::vector<std::pair<char, size_t>>* events = nullptr;   // {事件类型, 线性下标}
    size_t n = 0;

    void onPush  (Coordinate c, float) { events->emplace_back('P', c.first * n + c.second); }
    void onExpand(Coordinate c, float) { events->emplace_back('E', c.first * n + c.second); }
};

template<typename Fn, typename Visitor = SilentVisitor>
std::vector<Coordinate> Astar(Matrix<bool> const& map, size_t N, Coordinate start, Coordinate end,
                              Fn&& h, Visitor&& visit = Visitor{})
{ // h(c): c->end的估计距离
    if (!map[start.first * N + start.second] || !map[end.first * N + end.second])
        return std::vector<Coordinate>{};
    static auto comp = [](std::tuple<Coordinate, float, float> const x, std::tuple<Coordinate, float, float> const y){
        return std::get<1>(x) > std::get<1>(y);
    };
    std::priority_queue<
        std::tuple<Coordinate, float, float>, // c, f, g
        std::vector<std::tuple<Coordinate, float, float>>,
        decltype(comp)> q(comp);
    Matrix<std::pair<float, Coordinate>>
        record(N*N, {std::numeric_limits<float>::infinity(), {-1, -1}}); // {gc, parent}

    auto get_result = [&](Coordinate end){
        std::vector<Coordinate> path;
        Coordinate cur = end;
        while(cur != Coordinate{SIZE_MAX, SIZE_MAX}){
            path.push_back(cur);
            cur = record[cur.first * N + cur.second].second;
        }
        std::reverse(path.begin(), path.end());
        return path;
    };

    q.push({start, h(start), 0.0f});
    record[start.first * N + start.second] = {0.0f, {-1, -1}};
    while(true){
        if (q.empty()) return std::vector<Coordinate>{};
        auto [coordinate, f, gc] = q.top();
        if (coordinate == end)
            return get_result(end);// 启发出来的最优路径, 由于在push的时候确保gc最小，所以一定是当前的最优路径
        auto [x, y] = coordinate;

        auto check = [&](long long cx, long long cy, float cg){
            if (cx < 0 || cy < 0 || cx >= (long long)N || cy >= (long long)N)
                return false;                                  // 越界
            float known = record[(size_t)cx * N + (size_t)cy].first;
            if (known == std::numeric_limits<float>::infinity()) return true;
            return cg < known;
        };

        q.pop();
        if (gc > record[(size_t)x * N + (size_t)y].first)
            continue;

        visit.onExpand(coordinate, gc);                        // ← hook：这一格正在被扩展

        static const int dx[4] = { -1, 1, 0, 0 };
        static const int dy[4] = {  0, 0,-1, 1 };

        for (int d = 0; d < 4; ++d)
        {
            long long nx = (long long)x + dx[d];
            long long ny = (long long)y + dy[d];

            if (!check(nx, ny, gc + 1)) continue;              // 越界 / 不是更优
            size_t ni = (size_t)nx * N + (size_t)ny;
            if (!map[ni]) continue;                            // 是墙

            Coordinate nb{ (size_t)nx, (size_t)ny };
            q.push({ nb, gc + 1 + h(nb), gc + 1 });
            record[ni] = { gc + 1, {x, y} };
            visit.onPush(nb, gc + 1);                          // ← hook：这一格刚被发现
        }
    }
}

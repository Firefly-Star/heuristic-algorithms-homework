// 主程序：解析命令行参数，把输入、算法、输出拼起来
#include "astar.h"
#include "coord.h"
#include "grid.h"
#include "heuristic.h"
#include "mapfile.h"
#include "render.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#  include <windows.h>
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // ---- 命令行参数 ----
    //   1.exe map.txt                     只打印结果
    //   1.exe map.txt --h=octile          选启发函数（默认 manhattan）
    //   1.exe map.txt --anim              顺便生成 anim.html（默认 60ms 一帧）
    //   1.exe map.txt --anim=120          指定每帧毫秒数
    //   1.exe map.txt --out=x.html        指定输出文件名
    std::string mapPath = "map.txt";
    std::string animOut = "anim.html";
    std::string hSpec   = "manhattan";
    bool        anim    = false;
    int         animMs  = 60;

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a.rfind("--anim", 0) == 0)
        {
            anim = true;
            auto eq = a.find('=');
            if (eq != std::string::npos)
            {
                try { animMs = std::max(1, std::stoi(a.substr(eq + 1))); }
                catch (...) { std::cerr << "无法解析 --anim 的毫秒数，用默认 60\n"; }
            }
        }
        else if (a.rfind("--h=", 0) == 0)
            hSpec = a.substr(4);
        else if (a.rfind("--out=", 0) == 0)
            animOut = a.substr(6);
        else
            mapPath = a;
    }

    // ---- 输入 ----
    auto g = LoadGrid(mapPath.c_str());
    if (!g) return 1;

    // ---- 启发函数（策略，和算法无关；astar.h 完全不认识它）----
    auto h = heuristic::ByName(hSpec, g->end, *g);
    if (!h)
    {
        std::cerr << "不认识的启发函数: " << hSpec << '\n'
                  << "可用: manhattan / euclidean / chebyshev / octile / zero / perfect\n"
                  << "可以加权重后缀，例如 manhattan:1.5（w>1 不再保证最优）\n";
        return 1;
    }

    PrintHeuristic(std::cout, hSpec);
    PrintLoadSummary(std::cout, *g);
    PrintGrid(std::cout, *g, nullptr);

    // ---- 算法（顺便把搜索过程记下来，用来统计扩展数 / 生成动画）----
    std::vector<std::pair<char, size_t>> events;
    TraceVisitor tv{&events, g->n};
    auto route = Astar(g->map, g->n, g->start, g->end, h, tv);

    const size_t expanded = (size_t)std::count_if(
        events.begin(), events.end(), [](auto const& e) { return e.first == 'E'; });

    if (anim)
    {
        WriteHtmlAnim(animOut, *g, events, route, animMs, hSpec);
        std::cout << "动画已写入 " << animOut << "（用浏览器打开）\n";
    }

    // ---- 输出 ----
    if (route.empty())
    {
        std::cout << "\n无路径\n";
        return 1;
    }
    PrintRouteLength(std::cout, route);
    PrintSearchStats(std::cout, expanded, (size_t)std::count(g->map.begin(), g->map.end(), true));
    PrintGrid(std::cout, *g, &route);

    return 0;
}

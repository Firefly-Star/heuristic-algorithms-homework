// 主程序：解析命令行参数，把输入、算法、输出拼起来
#include "astar.h"
#include "coord.h"
#include "grid.h"
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
    //   1.exe map.txt                 只打印结果
    //   1.exe map.txt --anim          顺便生成 anim.html（默认 60ms 一帧）
    //   1.exe map.txt --anim=120      指定每帧毫秒数
    //   1.exe map.txt --out=x.html    指定输出文件名
    std::string mapPath = "map.txt";
    std::string animOut = "anim.html";
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
        else if (a.rfind("--out=", 0) == 0)
            animOut = a.substr(6);
        else
            mapPath = a;
    }

    // ---- 输入 ----
    auto g = LoadGrid(mapPath.c_str());
    if (!g) return 1;

    PrintLoadSummary(std::cout, *g);
    PrintGrid(std::cout, *g, nullptr);

    // ---- 启发函数：曼哈顿距离（先转 int 再相减，避免无符号回绕）----
    Coordinate goal = g->end;
    auto h = [goal](Coordinate c) -> float {
        return static_cast<float>(
            std::abs(static_cast<int>(c.first)  - static_cast<int>(goal.first)) +
            std::abs(static_cast<int>(c.second) - static_cast<int>(goal.second)));
    };

    // ---- 算法（--anim 时顺便把搜索过程记下来）----
    std::vector<std::pair<char, size_t>> events;
    std::vector<Coordinate> route;
    if (anim)
    {
        TraceVisitor tv{&events, g->n};
        route = Astar(g->map, g->n, g->start, g->end, h, tv);
        WriteHtmlAnim(animOut, *g, events, route, animMs);
        std::cout << "动画已写入 " << animOut << "（用浏览器打开）\n";
    }
    else
        route = Astar(g->map, g->n, g->start, g->end, h);

    // ---- 输出 ----
    if (route.empty())
    {
        std::cout << "\n无路径\n";
        return 1;
    }
    PrintRouteLength(std::cout, route);
    PrintGrid(std::cout, *g, &route);

    return 0;
}

// 输出：控制台画图 + HTML 动画
#include "render.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "generated/assets.h"   // 由 tools/embed.py 生成：assets/anim.html + assets/anim.js

// ================= 控制台 =================

void PrintLoadSummary(std::ostream& os, Grid const& g)
{
    os << "规模: " << g.n
       << "  起点: (" << g.start.first << ", " << g.start.second << ")"
       << "  终点: (" << g.end.first << ", " << g.end.second << ")\n\n";
}

void PrintGrid(std::ostream& os, Grid const& g, std::vector<Coordinate> const* route)
{
    std::vector<char> canvas(g.n * g.n, ' ');
    for (size_t r = 0; r < g.n; ++r)
        for (size_t c = 0; c < g.n; ++c)
            canvas[r * g.n + c] = g.walkable({r, c}) ? '.' : '#';
    if (route)
        for (auto c : *route) canvas[g.index(c)] = '*';
    canvas[g.index(g.start)] = 'S';
    canvas[g.index(g.end)]   = 'E';

    for (size_t r = 0; r < g.n; ++r)
    {
        for (size_t c = 0; c < g.n; ++c) os << canvas[r * g.n + c] << ' ';
        os << '\n';
    }
}

void PrintRouteLength(std::ostream& os, std::vector<Coordinate> const& route)
{
    os << "\n路径长度: " << (route.size() - 1)
       << "（途经 " << route.size() << " 个格子）\n\n";
}

// ================= HTML 动画 =================

namespace {

// 把 text 里出现的所有 from 换成 to
void ReplaceAll(std::string& text, std::string const& from, std::string const& to)
{
    if (from.empty()) return;
    for (size_t pos = text.find(from); pos != std::string::npos; pos = text.find(from, pos + to.size()))
        text.replace(pos, from.size(), to);
}

// 组装页面要用到的数据：规模、起终点、地图、搜索事件、最终路径
std::string BuildData(Grid const& g,
                      std::vector<std::pair<char, size_t>> const& events,
                      std::vector<Coordinate> const& route,
                      int delayMs)
{
    std::ostringstream d;

    d << "const N = " << g.n << ";\n";
    d << "const START = " << g.index(g.start) << ", END = " << g.index(g.end) << ";\n";
    d << "const DELAY = " << delayMs << ";\n";

    d << "const MAP = \"";
    for (size_t i = 0; i < g.map.size(); ++i) d << (g.map[i] ? '1' : '0');
    d << "\";\n";

    d << "const EV = \"";
    for (size_t i = 0; i < events.size(); ++i)
    {
        if (i) d << ' ';
        d << events[i].first << events[i].second;
    }
    d << "\".split(' ').filter(Boolean);\n";

    d << "const ROUTE = [";
    for (size_t i = 0; i < route.size(); ++i)
    {
        if (i) d << ',';
        d << g.index(route[i]);
    }
    d << "];";                       // 末尾不留换行：模板里占位符那一行自带

    return d.str();
}

}  // namespace

void WriteHtmlAnim(std::string const& path,
                   Grid const& g,
                   std::vector<std::pair<char, size_t>> const& events,
                   std::vector<Coordinate> const& route,
                   int delayMs)
{
    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        std::cerr << "无法写入动画文件: " << path << '\n';
        return;
    }

    // 模板里有两个占位符：@@DATA@@ 和 @@SCRIPT@@
    std::string html = kAnimTemplate;
    ReplaceAll(html, "@@SCRIPT@@", kAnimScript);
    ReplaceAll(html, "@@DATA@@", BuildData(g, events, route, delayMs));
    out << html;
}

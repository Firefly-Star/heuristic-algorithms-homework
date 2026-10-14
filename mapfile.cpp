// 输入：地图文件的解析
#include "mapfile.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {   // 只在本文件里用的解析辅助

// 跳过冒号之前的全部内容（半角 ':' 和全角 '：' 都能吃掉）
// 注意：全角冒号 U+FF1A 的 UTF-8 是 EF BC 9A，末字节是 0x9A 而不是 0x3A，
// 所以用 in.ignore(n, ':') 是找不到全角冒号的，会一路吃到行尾。
inline void SkipLabel(std::istream& in)
{
    int prev1 = -1, prev2 = -1;
    while (in)
    {
        int c = in.get();
        if (c == EOF) break;
        if (c == ':') break;                                    // 半角 ':'
        if (prev2 == 0xEF && prev1 == 0xBC && c == 0x9A) break;  // 全角 '：'
        prev2 = prev1;
        prev1 = c;
    }
    if (!in) in.clear();                 // 整行没有冒号时，清掉 fail 位
}

// 跳过不是数字的字符（'(' ')' ',' ' ' 等）
// 注意：operator>> 只跳过空白字符，不会跳过括号和逗号，必须自己跳。
inline void SkipNonDigit(std::istream& in)
{
    while (in)
    {
        int c = in.peek();
        if (c == EOF) break;
        if (std::isdigit(static_cast<unsigned char>(c))) break;
        in.get();
    }
}

// 一行里的前两个整数：解析 "(x, y)" 里的 x 和 y
inline bool ReadPair(std::istream& in, size_t& a, size_t& b)
{
    SkipNonDigit(in);
    if (!(in >> a)) return false;
    SkipNonDigit(in);                    // 吃掉 x 和 y 之间的 ',' ')' 等
    return static_cast<bool>(in >> b);
}

// 读 N 行、每行 N 个 0/1，空格分隔
inline bool ReadMatrix(std::istream& in, size_t n, std::vector<bool>& out)
{
    out.assign(n * n, false);
    std::string line;
    size_t row = 0;
    int    guard = 0;

    while (row < n && std::getline(in, line))
    {
        std::istringstream ls(line);
        std::vector<int> cells;
        int v = 0;
        while (ls >> v) cells.push_back(v);

        if (cells.empty())                       // 空行放过，但别无限循环
        {
            if (++guard > 1000) return false;
            continue;
        }
        if (cells.size() != n)
        {
            std::cerr << "第 " << (row + 1) << " 行应有 " << n
                      << " 个数，实际 " << cells.size() << " 个\n";
            return false;
        }

        for (size_t col = 0; col < n; ++col)
            out[row * n + col] = (cells[col] != 0);

        ++row;
        guard = 0;
    }
    return row == n;
}

}  // namespace

std::optional<Grid> LoadGrid(char const* path)
{
    std::ifstream in(path);
    if (!in)
    {
        std::cerr << "无法打开文件: " << path << '\n';
        return std::nullopt;
    }

    Grid g;
    std::string line;

    // 第一行：规模: N
    if (!std::getline(in, line))
    {
        std::cerr << "文件为空\n";
        return std::nullopt;
    }
    {
        std::istringstream ls(line);
        SkipLabel(ls);
        if (!(ls >> g.n) || g.n == 0)
        {
            std::cerr << "第一行\"规模\"解析失败: " << line << '\n';
            return std::nullopt;
        }
    }

    // 第二行：起点: (x, y) 终点: (x, y)
    if (!std::getline(in, line))
    {
        std::cerr << "缺少起点/终点行\n";
        return std::nullopt;
    }
    {
        std::istringstream ls(line);
        SkipLabel(ls);                                   // 吃掉"起点:"
        if (!ReadPair(ls, g.start.first, g.start.second))
        {
            std::cerr << "起点解析失败: " << line << '\n';
            return std::nullopt;
        }
        SkipLabel(ls);                                   // 吃掉"终点:"
        if (!ReadPair(ls, g.end.first, g.end.second))
        {
            std::cerr << "终点解析失败: " << line << '\n';
            return std::nullopt;
        }
    }

    // 接下来 N 行：N 个 0/1
    if (!ReadMatrix(in, g.n, g.map))
    {
        std::cerr << "方格数据解析失败\n";
        return std::nullopt;
    }

    // 合法性检查
    if (!g.inside(g.start) || !g.inside(g.end))
    {
        std::cerr << "起点或终点越界\n";
        return std::nullopt;
    }
    if (!g.walkable(g.start) || !g.walkable(g.end))
    {
        std::cerr << "起点或终点是墙（值为 0）\n";
        return std::nullopt;
    }

    return g;
}

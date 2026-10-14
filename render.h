// 输出：控制台画图 + HTML 动画
#pragma once

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include "grid.h"

// ---- 控制台 ----
void PrintHeuristic(std::ostream& os, std::string const& name);                         // 这一轮用的启发函数
void PrintLoadSummary(std::ostream& os, Grid const& g);                                 // 规模 / 起点 / 终点
void PrintGrid(std::ostream& os, Grid const& g, std::vector<Coordinate> const* route);  // route 非空就把路径画成 '*'
void PrintRouteLength(std::ostream& os, std::vector<Coordinate> const& route);          // 路径长度
void PrintSearchStats(std::ostream& os, size_t expanded, size_t freeCells);             // 扩展格数

// ---- HTML ----
void WriteHtmlAnim(std::string const& path,
                   Grid const& g,
                   std::vector<std::pair<char, size_t>> const& events,
                   std::vector<Coordinate> const& route,
                   int delayMs,
                   std::string const& heuristicName);

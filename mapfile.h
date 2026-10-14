// 输入：把地图文件读成 Grid
#pragma once

#include <optional>

#include "grid.h"

// 文件格式：
//     规模: N
//     起点: (x, y) 终点: (x, y)
//     接下来 N 行，每行 N 个 0/1（1 = 可通过，0 = 墙）
// 半角 / 全角冒号、坐标带不带括号都能读。
// 解析失败时把原因打到 stderr 并返回 nullopt。
std::optional<Grid> LoadGrid(char const* path);

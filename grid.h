// 数据模型：一张方格地图
#pragma once

#include <cstddef>
#include <vector>

#include "coord.h"

struct Grid
{
    size_t            n = 0;              // 规模 N
    std::vector<bool> map;                // 长度 n*n，true 表示可通过
    Coordinate        start{0, 0};
    Coordinate        end{0, 0};

    size_t index(Coordinate c) const { return c.first * n + c.second; }
    bool   inside(Coordinate c) const { return c.first < n && c.second < n; }
    bool   walkable(Coordinate c) const { return inside(c) && map[index(c)]; }
};

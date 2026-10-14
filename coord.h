// 公共词汇：矩阵别名和坐标
#pragma once

#include <cstddef>
#include <utility>
#include <vector>

template<typename T>
using Matrix = std::vector<T>;

using Coordinate = std::pair<size_t, size_t>;   // (行, 列)

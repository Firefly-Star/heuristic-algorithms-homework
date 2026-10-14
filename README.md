# 1 — A* 寻路演示

## 用途

读入一张方格地图，用 A\*（曼哈顿距离作启发函数）求起点到终点的最短路径：

- 在控制台打印地图和路径，以及路径长度；
- 可选地把**整个搜索过程**（每一格何时进入边界、何时被扩展）导出成一个自包含的 HTML 动画，用浏览器打开即可播放，支持播放 / 暂停 / 单步 / 回退 / 调速。

## 文件结构

```
heuristic-algorithms-homework/
├── .gitignore       忽略构建产物（1.exe、生成的 anim.html）
├── LICENSE          MIT
├── README.md        本文件
├── coord.h          公共词汇：Matrix<T> 别名、Coordinate
├── grid.h           数据模型：Grid（地图、起终点、index / inside / walkable）
├── astar.h          算法：Astar 模板 + 访问者 hook（不依赖 Grid、不做文件读写）
├── mapfile.h        输入接口
├── mapfile.cpp      输入实现：解析地图文件
├── render.h         输出接口
├── render.cpp       输出实现：控制台画图 + 生成 HTML 动画
├── main.cpp         命令行参数解析，把输入 / 算法 / 输出拼起来
├── map.txt          示例地图
├── assets/
│   ├── anim.html    HTML + CSS 骨架模板（含 @@DATA@@ / @@SCRIPT@@ 占位符）
│   └── anim.js      动画的 JS 逻辑
├── tools/
│   └── embed.py     把 assets/ 嵌成 generated/assets.h
└── generated/
    └── assets.h     自动生成，请勿手改
```

## 构建命令

在仓库根目录（clone 下来的目录名就是 `heuristic-algorithms-homework`）执行：

```powershell
g++ -std=c++20 -Wall -Wextra -o 1.exe main.cpp mapfile.cpp render.cpp
```

改了 `assets/anim.html` 或 `assets/anim.js` 之后，要先重新嵌入再编译（`generated/assets.h` 已随工程保留，平时编译不需要这一步）：

```powershell
python tools\embed.py
g++ -std=c++20 -Wall -Wextra -o 1.exe main.cpp mapfile.cpp render.cpp
```

## 使用命令

```powershell
.\1.exe                                 # 读当前目录下的 map.txt
.\1.exe map.txt                         # 指定地图文件
.\1.exe map.txt --anim                  # 顺便生成 anim.html（默认 60ms 一帧）
.\1.exe map.txt --anim=120              # 指定每帧毫秒数
.\1.exe map.txt --anim --out=out.html   # 指定动画输出文件名
```

地图文件的格式（`1` 表示可通过，`0` 表示墙，坐标是「行, 列」）：

```
规模: 10
起点: (0, 0) 终点: (9, 9)
1 1 1 1 1 1 1 1 1 1
0 0 0 0 0 0 0 0 0 1
...（接下来共 10 行，每行 10 个 0/1）
```

半角 / 全角冒号都可以，坐标带不带括号都可以。

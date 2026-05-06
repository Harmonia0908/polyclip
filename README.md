# PolygonClipDemo

这是一个使用 **C++11 + Qt 5 Widgets** 编写的多边形裁切多边形图形界面演示项目。裁剪算法为手写的 **Weiler-Atherton polygon clipping algorithm**，没有调用 Boost.Geometry、CGAL、Clipper 等现成几何裁剪库。

项目同时提供不依赖 Qt 的命令行演示目标 `console_demo`，用于在没有 Qt 5 的环境中编译和验证几何/裁剪算法核心。

## 项目亮点

- 使用 C++11 + Qt5 Widgets 构建图形界面，兼容常见桌面平台。
- 手写 Weiler-Atherton 多边形裁切算法，不依赖 Boost.Geometry、CGAL、Clipper 等几何库。
- 支持拖拽顶点、动态新增顶点、删除顶点，便于观察输入变化对裁切结果的影响。
- 可视化显示交点、entry/exit 标记和半透明裁切结果。
- 支持多个预设案例，包括凹多边形裁切、凸多边形裁切、包含、不相交、顶点落边、边界相切等场景。
- 几何与算法核心可独立编译为纯 C++11 静态库，并提供无 Qt 的 `console_demo`。

## 功能

- 显示两个多边形：
  - Subject Polygon：被裁切多边形，默认是一个凹多边形
  - Clip Polygon：裁切多边形，默认是一个凸多边形
- 鼠标拖拽顶点实时修改两个多边形。
- 通过右侧下拉框选择当前编辑对象。
- 双击画布空白位置可向当前编辑对象新增顶点，新顶点会插入到距离鼠标最近的边之后。
- 右键点击顶点可删除顶点，Subject 和 Clip 都至少保留 3 个顶点。
- 顶点旁显示编号，例如 `S0`、`S1`、`C0`、`C1`。
- 按钮：
  - 重新裁切
  - 重置示例
  - 显示/隐藏交点
  - 显示/隐藏 entry/exit 标记
- 裁切结果支持一个或多个结果多边形，并用半透明颜色填充。
- 右侧显示算法步骤日志。

## 操作说明

- 使用右侧“当前编辑对象”下拉框选择要编辑的多边形：`Subject Polygon` 或 `Clip Polygon`。
- 拖拽当前编辑对象的顶点可实时移动顶点并重新裁切。
- 双击靠近当前编辑多边形边的位置可新增顶点；如果双击位置离边太远，则不会插入，日志会给出提示。
- 右键点击当前编辑对象的顶点可删除顶点；每个多边形至少保留 3 个顶点。
- 使用“显示交点”勾选框控制交点圆点是否显示。
- 使用“显示 entry/exit 标记”勾选框控制 entry/exit 文本标记是否显示。
- “预设案例”按钮用于快速加载典型演示场景：
  - 凹多边形裁切：展示凹 Subject 与 Clip 的多次穿入穿出。
  - 凸多边形裁切：展示两个凸多边形的常规裁切。
  - Subject 完全在 Clip 内：展示无交点但完全包含的情况。
  - 完全不相交：展示无交点且结果为空的情况。
  - 顶点落在边上：展示顶点位于另一多边形边界上的边界情况。
  - 边界相切：展示边界接触但面积结果退化的情况。

## 依赖

- CMake 3.10+
- C++ 编译器，支持 C++11
- Qt 5 Widgets（仅 GUI 目标需要）

注意：项目使用 Qt 5 API，避免了 Qt 6 专属接口。

## 编译运行

### 无 Qt 环境：只构建 console_demo

没有安装 Qt 5 时，可以关闭 GUI 构建，只编译纯 C++11 的算法库和命令行示例：

```bash
cmake -S . -B build -DBUILD_GUI=OFF
cmake --build build
./build/console_demo
```

`console_demo` 会打印裁切结果多边形数量、结果顶点坐标、交点数量和 entry/exit 标记。

### 有 Qt5 环境：构建 GUI

默认 `BUILD_GUI=ON`。如果 CMake 能找到 Qt 5，会同时构建 `PolygonClipDemo` GUI 和 `console_demo`；如果找不到 Qt 5，会给出 warning 并只构建 `console_demo`。

#### macOS

如果使用 Homebrew 安装 Qt 5，常见路径为 `/opt/homebrew/opt/qt@5` 或 `/usr/local/opt/qt@5`。推荐使用 `brew --prefix qt@5` 传给 `CMAKE_PREFIX_PATH`：

```bash
brew install qt@5
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt@5)"
cmake --build build
./build/PolygonClipDemo
```

#### Linux

以 Ubuntu/Debian 为例：

```bash
sudo apt install cmake g++ qtbase5-dev
cmake -S . -B build
cmake --build build
./build/PolygonClipDemo
```

#### Windows

1. 安装 Qt 5 和 CMake。
2. 使用 Qt Creator 打开本项目根目录的 `CMakeLists.txt`。
3. 选择 Qt 5 Kit 后构建运行。

也可以在 “x64 Native Tools Command Prompt” 中指定 Qt 5 的 CMake 前缀路径，例如：

```bat
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
cmake --build build --config Release
build\Release\PolygonClipDemo.exe
```

## 算法流程

本演示中的核心实现位于 `src/WeilerAtherton.cpp`，主要流程如下：

1. 枚举 Subject 与 Clip 两个多边形的边，并计算所有线段交点。
2. 将交点按边参数插入到 Subject 和 Clip 各自的顶点链表中。
3. 根据 Subject 边界穿入或穿出 Clip 的状态判断交点是 entry 还是 exit。
4. 从未访问的 entry 点开始追踪结果环。
5. 追踪过程中在 Subject 链表和 Clip 链表之间切换。
6. 清理重复点和退化环，输出一个或多个裁切结果多边形。

为了便于教学展示，右侧日志会输出交点插入、entry/exit 分类和结果追踪过程。

## 边界情况说明

项目用 `EPS = 1e-9` 处理浮点误差，并尽量处理以下情况：

- 没有交点但 Subject 完全在 Clip 内：结果为 Subject。
- 没有交点但 Clip 完全在 Subject 内：结果为 Clip。
- 没有交点且互不包含：结果为空。
- 顶点落在另一条边上：会把该顶点识别为交点并参与显示/分类。
- 交点接近顶点：会吸附到端点，降低重复交点带来的影响。

## 已知限制

- 输入多边形需要是简单多边形。
- 不支持自交多边形；GUI 会进行简单多边形检测，并在发现自交时阻止裁切。
- 不完整支持重合边；当前实现会尽量识别部分边界接触和端点落边情况，但不等价于完整的重合边布尔运算。
- 不支持带洞多边形。
- 当前实现面向教学演示和算法过程解释，不等价于工业级几何布尔运算库。

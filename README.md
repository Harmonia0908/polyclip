# PolygonClipDemo

这是一个使用 **C++11 + Qt 5 Widgets** 编写的多边形裁切多边形图形界面演示项目。裁剪算法为手写的 **Weiler-Atherton polygon clipping algorithm**，没有调用 Boost.Geometry、CGAL、Clipper 等现成几何裁剪库。

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

## 依赖

- CMake 3.10+
- C++ 编译器，支持 C++11
- Qt 5 Widgets

注意：项目使用 Qt 5 API，避免了 Qt 6 专属接口。

## 编译运行

### macOS

如果使用 Homebrew 安装 Qt 5：

```bash
brew install qt@5
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt@5)"
cmake --build build
./build/PolygonClipDemo
```

### Linux

以 Ubuntu/Debian 为例：

```bash
sudo apt install cmake g++ qtbase5-dev
cmake -S . -B build
cmake --build build
./build/PolygonClipDemo
```

### Windows

1. 安装 Qt 5 和 CMake。
2. 使用 Qt Creator 打开本项目根目录的 `CMakeLists.txt`。
3. 选择 Qt 5 Kit 后构建运行。

也可以在 “x64 Native Tools Command Prompt” 中指定 Qt 5 的 CMake 前缀路径，例如：

```bat
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
cmake --build build --config Release
build\Release\PolygonClipDemo.exe
```

## Weiler-Atherton 算法流程

本演示中的核心实现位于 `src/WeilerAtherton.cpp`，主要步骤如下：

1. 复制 Subject 和 Clip 多边形，并把两者方向归一化为逆时针。
2. 遍历 Subject 的每条边和 Clip 的每条边，手写线段相交检测。
3. 将交点按照边上的参数位置插入到两个多边形各自的循环链表中。
4. 沿 Subject 链表判断每个交点是 entry 还是 exit：
   - 沿 Subject 边从 clip 外进入 clip 内，标记为 entry。
   - 沿 Subject 边从 clip 内离开 clip 外，标记为 exit。
   - 切触点或边界点会标记为非穿越交点，主要用于显示。
5. 从未访问的 entry 点开始追踪：
   - 在 Subject 边界上前进，直到遇到 exit。
   - 切换到 Clip 边界继续前进，直到遇到 entry。
   - 再切回 Subject，直到回到起点。
6. 清理重复点、过滤面积过小的结果环，输出一个或多个裁剪结果多边形。

## 边界情况说明

项目用 `EPS = 1e-9` 处理浮点误差，并尽量处理以下情况：

- 没有交点但 Subject 完全在 Clip 内：结果为 Subject。
- 没有交点但 Clip 完全在 Subject 内：结果为 Clip。
- 没有交点且互不包含：结果为空。
- 顶点落在另一条边上：会把该顶点识别为交点并参与显示/分类。
- 交点接近顶点：会吸附到端点，降低重复交点带来的影响。

这份代码的目标是教学演示和过程可解释，不追求工业级几何鲁棒性。对于自交多边形、复杂重合边、多边形带洞等情况没有做完整支持。

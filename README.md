# vtk-line-editor — VTK 交互学习项目

从零实现的 2D 线段编辑器练习，用于学习 VTK 渲染与鼠标交互：不使用 `vtkInteractorStyle` 现成样式类的默认行为，而是以 `vtkCommand` 观察者模式派生自定义回调（`MyMouseCallback`），配合 `vtkCellPicker` 实现线段拾取。

> 学习背景：为工业软件（机器人臂加工 CAD 模块）的鼠标交互开发做的自学练习，思路同样适用于编辑器类工具的 3D 交互。

## 功能状态

- ✅ 场景搭建：renderer / renderWindow / interactor + 线段 actor（editor / edge / model 极简分层）
- ✅ 鼠标拾取：`LeftButtonPressEvent` 观察者 + `vtkCellPicker`（容差 0.005）命中检测，选中高亮（变色 + 线宽），再次点击恢复
- ✅ 显示坐标 → 世界坐标转换（`SetDisplayPoint` / `DisplayToWorld` / 齐次坐标归一化），为拖拽做准备
- 🔶 拖拽线：开发中

## 构建

要求：CMake ≥ 3.16、C++17 编译器（MSVC / GCC / Clang 均可）、VTK 9.x（含 CMake 配置文件）。

```bash
cmake -S . -B build -DVTK_DIR="<VTK安装目录>/lib/cmake/vtk-9.3"
cmake --build build --config Release
```

在 Windows + Visual Studio + VTK 9.3.0 下开发验证。

## 目录说明

| 文件 | 职责 |
|---|---|
| `main.cpp` | 场景组装与程序入口 |
| `editor.h/.cpp` | 编辑器：由两端点创建线段 actor |
| `edge.h/.cpp` | 数据模型：边（两对称点），拖拽位移接口 |
| `model.h/.cpp` | 数据模型：边集合 + actor 到边的映射 |
| `MyMouseCallback.h/.cpp` | 自定义交互回调：拾取 / 高亮 / 拖拽状态机 |

## License

MIT

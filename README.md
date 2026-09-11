# VTKLesson1 — VTK + OpenCASCADE 三维模型交互学习项目

在 C++/VTK 中读取 STEP 三维模型，并逐步实现**点 / 线 / 面 / 体的显示与交互选择**的自学项目。
用 OpenCASCADE（OCCT）解析 STEP 并三角化，桥接成 `vtkPolyData` 渲染，再自定义
`vtkInteractorStyle` 完成相机操作、鼠标拾取与逐图元高亮。在 Windows + Visual Studio +
VTK 9.3.0 + OpenCASCADE 下开发，使用 CMake 管理构建。

项目按学习顺序拆成三个递进的可执行目标（target），后一个建立在前一个之上。

## 三个示例程序

| Target | 依赖 | 说明 |
| --- | --- | --- |
| `vtk-line-editor` | VTK | 入门作业：派生 `vtkCommand` 观察者，实现线段的拾取、高亮、拖拽平移 |
| `step-info` | OCCT | 最小 STEP 读取：用 OCCT 读 STEP 并打印体 / 面 / 边 / 顶点数量 |
| `step-view` | VTK + OCCT | **当前主线**：STEP 三角化显示 + 拓扑索引 + 点选 / 面选 / 体选高亮 |

## step-view 交互方式

- **右键拖动**：旋转视角；**中键拖动**：平移；**滚轮**：缩放（继承 `vtkInteractorStyleTrackballCamera`）
- **左键点选**：面模式下高亮一个面，体模式下高亮整个实体；点其它处切换、点空白取消
- **键盘 `f` / `s`**：在「面选择 / 体选择」两种对象模式间切换
- 顶点、边的显示选择与框选仍在进行中

## 功能进度

- ✅ STEP → OCCT（`STEPControl_Reader`）→ `BRepMesh_IncrementalMesh` 三角化 → `vtkPolyData` 渲染
- ✅ `TopologyIndex`：三角形(cell) ↔ 面 ↔ 体的编号、邻接与反查索引，支持从渲染图元上卷到拓扑实体
- ✅ 自定义交互风格：左键选择、右键旋转、中键平移
- ✅ CellData 逐三角形 RGB 高亮，面 / 体两种选择模式（`f` / `s` 切换），通用选中恢复
- ⬜ 框选、边与顶点的显示和选择

## 构建

要求：CMake ≥ 3.16、支持 C++17 的 MSVC、一份 VTK 9.x 与一份 OpenCASCADE 安装树
（目录下含 `include/`、`lib/`、`bin/`）。本机依赖为手动整理的安装树、不含官方
`*Config.cmake`，因此通过缓存变量 `VTK_ROOT` / `OCCT_ROOT` 指定依赖根，而非 `find_package`。

```bash
cmake -S . -B build -DVTK_ROOT="D:/path/to/VTK-9.3.0" -DOCCT_ROOT="D:/path/to/OCCT"
cmake --build build --config Release
```

运行时需要能找到 VTK / OCCT 的动态库：把两者的 `bin` 目录加入 `PATH`，或把所需 dll
拷到 exe 同目录。源文件为 UTF-8，构建脚本已对 MSVC 开启 `/utf-8`。

## 运行

`models/` 目录自带示例模型 `TCMT1107_4.stp`（一枚数控刀片，约 1 体 / 70 面 / 180 边 /
112 顶点，三角化后 540 个三角形）。不传参数时默认加载它：

```bash
Output/Release/step-view.exe        # 弹出三维窗口
Output/Release/step-info.exe        # 控制台打印拓扑数量
```

也可以命令行传入任意 STEP 文件：`step-view.exe path/to/your.step`。
在 Visual Studio 中 F5 调试时，工作目录已通过 `VS_DEBUGGER_WORKING_DIRECTORY`
设为仓库根，因此默认相对路径 `models/...` 可直接找到。

## 目录结构

| 路径 | 职责 |
| --- | --- |
| `Source/LineEditor`、`Source/Include/LineEditor` | 线段拖拽作业，对应 `vtk-line-editor` |
| `Source/StepInfo/stepinfo.cpp` | STEP 读取与拓扑数量统计，对应 `step-info` |
| `Source/StepViewer/stepview.cpp` | step-view 入口：组装读取、转换、渲染、交互管线 |
| `Source/StepViewer/ShapeToPolyData.*` | OCCT `TopoDS_Shape` 三角化并转换为 `vtkPolyData` |
| `Source/StepViewer/TopologyIndex.h` | 拓扑编号 / 邻接 / 反查索引（header-only） |
| `Source/StepViewer/StepInteractorStyle.*` | 自定义交互风格：相机操作 + 拾取 + 高亮 + 模式切换 |
| `models/` | 自带示例 STEP 模型 |

## 技术要点

1. **OCCT → VTK 桥接**：`STEPControl_Reader` 先把 STEP 解析为交换模型，`TransferRoots`
   得到 `TopoDS_Shape`；`BRepMesh_IncrementalMesh` 离散为三角网格后，逐面读取
   `BRep_Tool::Triangulation` 的点表与三角形表，处理每个面的 `Location` 坐标变换、
   以及局部点编号到全局点编号的偏移，拼成全局 `vtkPoints` / `vtkPolyData`。
2. **拓扑层与渲染层解耦**：三角形(cell)唯一的直接父级是面，用 `cellToFace / faceToCell`
   双向映射、`faceToSolid` 上卷、边 / 顶点邻接表组织拓扑网；选择时从渲染 cell 进入拓扑网
   上卷到目标层级，高亮时再下钻收集该层级覆盖的全部 cell。
3. **逐图元高亮**：`vtkUnsignedCharArray` 三通道 RGB 挂到 `CellData`，mapper 设
   `UseCellData + DirectScalars`；对收集到的 cell 集合 `SetTuple3` 改色，并用
   `m_lastCells` 记录上一次高亮的 cell 实现通用恢复，使上色逻辑与「面 / 体 / 未来框选」解耦。
4. **交互风格**：继承 `vtkInteractorStyleTrackballCamera` 直接获得相机操作，只重写需要
   改变的按键（左键做选择、右键换绑旋转）；`vtkCellPicker` 以射线拾取屏幕处的 cellId。
5. **CMake 多目标组织**：以 target 为中心声明源文件、包含目录与链接库，依赖根用 `-D`
   注入，MSVC 下一次性 GLOB 导入库并统一 `/utf-8`。

## License

MIT

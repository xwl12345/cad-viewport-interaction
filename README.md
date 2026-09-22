# VTKLesson1 — VTK + OpenCASCADE 三维模型交互学习项目

在 C++/VTK 中读取 STEP 三维模型，实现**点 / 线 / 面 / 体四类对象的显示与交互选择**
（左键点选与矩形框选、右键旋转、中键平移）的自学项目。用 OpenCASCADE（OCCT）解析
STEP 并离散，桥接成 `vtkPolyData` 渲染，再自定义 `vtkInteractorStyle` 完成相机操作、
鼠标拾取、框选与逐图元高亮。在 Windows + Visual Studio + VTK 9.3.0 + OpenCASCADE 下
开发，使用 CMake 管理构建。

项目按学习顺序拆成三个递进的可执行目标（target），后一个建立在前一个之上。

## 三个示例程序

| Target | 依赖 | 说明 |
| --- | --- | --- |
| `vtk-line-editor` | VTK | 入门作业：派生 `vtkCommand` 观察者，实现线段的拾取、高亮、拖拽平移 |
| `step-info` | OCCT | 最小 STEP 读取：用 OCCT 读 STEP 并打印体 / 面 / 边 / 顶点数量 |
| `step-view` | VTK + OCCT | **当前主线**：STEP 的面 / 边 / 点三层显示，点选 + 框选，面 / 体 / 边 / 点四种选择模式与高亮 |

## step-view 交互方式

- **右键拖动**：旋转视角；**中键拖动**：平移；**滚轮**：缩放（继承 `vtkInteractorStyleTrackballCamera`）
- **左键单击**：点选鼠标下的一个对象；**按住左键拖出矩形**：框选矩形内的对象；**点空白处**：取消选择
- **键盘 `f` / `s` / `e` / `v`**：在 **面 / 体 / 边 / 点** 四种选择对象模式间切换
- 选择 **体** 时自动上卷到所属实体并高亮该实体的全部面；选择面 / 边 / 点时高亮对应图元
- 选中对象以黄色高亮，再次选择自动恢复原色

## 显示与选择的总体思路

一个 B-Rep 模型在渲染端被拆成**三层相互独立的可显示对象**，各持自己的
`vtkPoints / vtkPolyData / vtkActor / 颜色数组 / cell↔拓扑 id 反查表`：

| 显示层 | 数据来源 | VTK 单元 | 外观 |
| --- | --- | --- | --- |
| 面 | `BRepMesh_IncrementalMesh` 三角化 | `vtkTriangle` | 浅灰实体面 |
| 边 | `BRepAdaptor_Curve` + `GCPnts` 按弦差自适应采样 | `vtkPolyLine` | 深色粗线 |
| 顶点 | `BRep_Tool::Pnt` 取顶点坐标 | `vtkVertex` | 蓝色点 |

三层之间、以及渲染图元与 OCCT 拓扑实体（Solid / Face / Edge / Vertex）之间，全部通过
`TopologyIndex` 里的整数 id 表关联。体本身没有像素，不单独建 actor：拾取永远先命中某层
的一个 cell，再沿拓扑表**上卷**到目标层级（cell→面→体），高亮时再**下钻**收集该层级
覆盖的全部 cell。交互风格通过切换各 actor 的 `Pickable` 实现"当前只拾取某一层"的选择过滤。

## 功能进度

- ✅ STEP → OCCT（`STEPControl_Reader`）→ 面三角化 → `vtkPolyData` 渲染
- ✅ 边的自适应离散（`BRepAdaptor_Curve` + `GCPnts_QuasiUniformDeflection`）与线框层显示
- ✅ 顶点提取（`BRep_Tool::Pnt`，含 `Location` 坐标变换）与点层显示
- ✅ `TopologyIndex`：Solid / Face / Edge / Vertex 四张索引表、邻接表，以及面 / 边 / 点三层
  的 cell ↔ 拓扑 id 双向反查，支持从渲染图元上卷到拓扑实体
- ✅ 自定义交互风格：左键选择、右键旋转、中键平移、滚轮缩放
- ✅ 点选（`vtkCellPicker` 射线拾取）与矩形框选（`vtkHardwareSelector` 硬件 ID 拾取 + 橡皮筋）
- ✅ 面 / 体 / 边 / 点四种选择模式（`f / s / e / v`），逐图元 RGB 高亮与通用恢复

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
112 顶点；三角化后 540 个三角形，边按 0.01 弦差离散为 180 条折线、408 个采样点）。
不传参数时默认加载它：

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
| `Source/StepViewer/stepview.cpp` | step-view 入口：组装读取、三层转换、渲染与交互管线 |
| `Source/StepViewer/ShapeToPolyData.*` | OCCT `TopoDS_Shape` → 面 / 边 / 点三层 `vtkPolyData` 的转换 |
| `Source/StepViewer/TopologyIndex.h` | 拓扑索引 / 邻接 / cell↔拓扑 id 反查表（header-only） |
| `Source/StepViewer/StepInteractorStyle.*` | 自定义交互风格：相机操作 + 点选 / 框选 + 高亮 + 模式切换 |
| `models/` | 自带示例 STEP 模型 |

## 技术要点

1. **OCCT → VTK 面桥接**：`STEPControl_Reader` 先 `ReadFile` 把 STEP 解析为交换模型，
   `TransferRoots` 得到 `TopoDS_Shape`，`OneShape` 合并为整体；`BRepMesh_IncrementalMesh`
   离散为三角网格后，逐面读取 `BRep_Tool::Triangulation` 的点表与三角形表，处理每个面的
   `Location` 坐标变换，以及 OCCT 1 基局部点编号到 VTK 0 基全局点编号的偏移，拼成全局
   `vtkPoints / vtkPolyData`（相邻面共享顶点、不重复存点）。
2. **边与顶点的独立离散**：边用 `BRepAdaptor_Curve` 包裹 `TopoDS_Edge`，再用
   `GCPnts_QuasiUniformDeflection` 按弦差自适应采样，`Value(k)` 直接给出世界坐标，逐边连成
   `vtkPolyLine`；顶点用 `BRep_Tool::Pnt` 取局部点后再做 `Location` 变换，每个顶点建一个
   `vtkVertex`。边遍历去重后的边表（同一条边被两个面引用，只离散一次）。
3. **拓扑层与渲染层解耦**：用 `TopTools_IndexedMapOfShape`（`TopExp::MapShapes`）建立
   Solid / Face / Edge / Vertex 四张索引表，用 `TopExp::MapShapesAndAncestors` 建立
   面→体、边→面、点→边邻接；再为三层各建一份 cell ↔ 拓扑 id 反查。选择时从命中的渲染
   cell 进入拓扑网上卷到目标层级，高亮时下钻收集该层级覆盖的全部 cell。
4. **两种拾取方式**：点选用 `vtkCellPicker` 做 CPU 射线求交，按当前层设置拾取容差；框选用
   `vtkHardwareSelector` 读取 GPU 的 ID 缓冲（`FIELD_ASSOCIATION_CELLS`、`SetArea` 以左下角为
   原点），按像素精确返回矩形内的 cell，再统一交给同一套上卷 / 高亮逻辑（点选是框选退化为
   单个 cell 的特例）。
5. **逐图元高亮**：每层挂一个三通道 `vtkUnsignedCharArray` 到 `CellData`，mapper 设
   `UseCellData + DirectScalars`；对收集到的 cell 集合 `SetTuple3` 改为高亮色，并记录上一次
   高亮的 cell 集合，选择新对象时统一恢复原色。
6. **交互风格与选择过滤**：继承 `vtkInteractorStyleTrackballCamera` 直接复用相机操作，只重写
   左键（点选 / 拖框状态机）与字符事件（模式切换）；框选橡皮筋用 `vtkActor2D +
   vtkPolyDataMapper2D + vtkCoordinate`（显示坐标）绘制；切换模式时同步各 actor 的
   `Pickable`，使射线 / 硬件拾取只作用于当前对象层。
7. **CMake 多目标组织**：以 target 为中心显式列举源文件、包含目录与链接库，依赖根用 `-D`
   注入，MSVC 下一次性 GLOB 导入第三方静态库并统一 `/utf-8`。

## 已知简化与可改进方向

- 当前以单个模型为中心，三层数据与 `TopologyIndex` 尚未封装成"零件对象"；支持多模型 / 装配
  时需要引入零件 id 与对象管理，避免不同模型的 cell id 冲突。
- 选择状态目前直接用颜色数组表达；工业级实现通常把"选择状态"与"显示颜色"解耦（独立的高亮
  表示 / 显示模式），便于同时支持预选、多选与多种显示样式。
- 边用粗线、顶点用方点（线宽 / 点尺寸在部分驱动上会被钳制），可升级为 `vtkTubeFilter`
  圆管与 `vtkGlyph3D` 球形点；顶点也可改为仅在点选择模式下显示。
- 点选为 CPU 射线求交，在边角密集处容差过小不易选中、过大可能误选，可统一改用小矩形的
  硬件选择；后续还可扩展框选的"全包含 / 相交"语义、悬停预选、Ctrl 多选，以及嵌入 Qt 界面。

## License

MIT

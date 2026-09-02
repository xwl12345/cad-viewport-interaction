# vtk-line-editor — VTK 交互式线段编辑器（学习项目）

从零实现的 VTK 鼠标交互练习：不依赖 `vtkInteractorStyle` 的现成拖拽行为，而是派生 `vtkCommand`
自定义观察者，配合 `vtkCellPicker` 完成线段的**拾取、高亮、拖拽平移**完整交互链路，并用 CMake 管理构建。

> 学习背景：为工业软件（石材加工机器人 CAD 模块）的鼠标交互开发做的自学练习，思路同样适用于
> 编辑器类工具的 3D 交互。在 Windows + Visual Studio + VTK 9.3.0 下开发验证。

## 交互方式

- **左键点击线段**：拾取并高亮（变黄、线宽加粗）；再点其他线段切换选中，点空白取消高亮。
- **按住线段拖动后松开**：线段按鼠标位移平移到新位置（透视投影下 1:1 跟手）。
- **按住空白处拖动**：保留默认交互样式的相机旋转，互不干扰。

## 功能状态

- ✅ 场景搭建：renderer / renderWindow / interactor + 线段 actor（editor / edge / model 极简分层）
- ✅ 鼠标拾取：`LeftButtonPressEvent` + `vtkCellPicker`（容差 0.005）命中检测与高亮切换
- ✅ 事件仲裁：观察者优先级（1.0 > 默认样式 0）配合 `AbortFlagOn()`，命中线段才截断默认相机旋转，
  空白处事件放行
- ✅ 拖拽平移：三事件状态机（按下锁定 / 移动超 5px 判拖拽 / 松开提交）
- ✅ 屏幕像素 → 世界坐标：`SetDisplayPoint` / `DisplayToWorld` 双反投影 delta 法，并按拾取点深度
  修正（避免在近裁剪面计算导致的跟手不足）
- ✅ 文档/视图同步：model 更新后写回 `vtkPoints`，`Modified()` 标脏并 `Render()` 重绘

## 技术要点

1. **观察者模式自定义交互**：`MyMouseCallback : vtkCommand`，通过 `AddObserver` 订阅按下/移动/
   松开三个事件，以事件 id 分流，而非继承改写交互样式。
2. **事件竞争与截断**：默认 `vtkInteractorStyleTrackballCamera` 同样订阅左键事件；用更高优先级
   抢先处理，命中自有对象时 `AbortFlagOn()` 截断事件链（该标志 per-event 自动清零）。
3. **透视下的拖拽换算**：像素位移与世界位移的比例随深度变化（相似三角形），在被拖对象的实际深度
   平面上做两次反投影再相减，得到世界平移量。
4. **文档/视图分离**：`edge`（model）与 `vtkPoints`（polydata）各存一份，先改业务数据再同步
   渲染数据；VTK 拉模式管线靠 MTime 判断脏数据，`SetPoint` 后必须手动 `Modified()`。
5. **CMake 构建**：以 target 为中心组织源文件、包含目录与链接库；依赖路径通过缓存变量
   `-DVTK_ROOT` 注入，不写死绝对路径。

## 构建

要求：CMake ≥ 3.16、支持 C++17 的 MSVC（当前链接脚本按 Windows/MSVC 编写）、一份 VTK 9.x
安装树（目录下含 `include/`、`lib/Release/*.lib`、`bin/*.dll`）。

```bash
cmake -S . -B build -DVTK_ROOT="D:/path/to/VTK-9.3.0"
cmake --build build --config Release
```

> 说明：本机 VTK 为手动整理的安装树，不含官方的 CMake 配置文件（`VTKConfig.cmake`），因此没有
> 使用 `find_package(VTK)`，而是通过 `VTK_ROOT` 指定 include 目录并链接 `lib/Release` 下的导入库。
> 若改用官方安装版 VTK，可替换为标准写法 `find_package(VTK REQUIRED)` +
> `target_link_libraries(... ${VTK_LIBRARIES})`。

运行时需要能找到 VTK 的动态库：将 VTK 的 `bin` 目录加入 `PATH`，或把所需 dll 拷到 exe 同目录。

## 目录说明

| 文件 | 职责 |
|---|---|
| `main.cpp` | 场景组装、三条线段创建、事件观察者注册与程序入口 |
| `editor.h/.cpp` | 由两端点创建线段 actor，维护 model 与 actor 的映射 |
| `edge.h/.cpp` | 数据模型：一条边（两个端点）与平移接口 moveEdge |
| `model.h/.cpp` | 数据模型：边集合 + actor 到边的查找表 |
| `MyMouseCallback.h/.cpp` | 自定义交互回调：拾取 / 高亮 / 拖拽状态机与坐标转换 |

## License

MIT

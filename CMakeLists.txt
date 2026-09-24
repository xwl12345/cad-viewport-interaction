# ====================================================================
#  VTKLesson1 / CMakeLists.txt
#  目录结构（对齐 Qt_TcpClient 习惯）：
#    Source/Include/<模块>/*.h   头文件集中管理
#    Source/<模块>/*.cpp         每个可执行目标一个模块文件夹
#    Output/<Debug|Release>/     最终 exe 统一输出到这里
#    build/                      CMake 生成目录（不入库）
#  三个目标：vtk-line-editor（纯VTK）/ step-info（纯OCCT）/ step-view（VTK+OCCT）
# ====================================================================

# ---------- 第 0 步：工程声明（必须） ----------
cmake_minimum_required(VERSION 3.16)
project(VTKLesson1 LANGUAGES CXX)

# ---------- 第 1 步：语言标准 ----------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- 统一源码/头文件根目录 ----------
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)
set(INC_DIR ${SRC_DIR}/Include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Output")

find_package(VTK 9.3 REQUIRED COMPONENTS
  CommonCore
  CommonDataModel
  InteractionStyle
  RenderingCore
  vtksys
  RenderingOpenGL2)

  find_package(OpenCASCADE 7.8.0 REQUIRED)

  set(VTK_DBG_PATH "PATH=${VTK_PREFIX_PATH}/bin;%PATH%")
  set(OCCT_DBG_PATH "PATH=${VTK_PREFIX_PATH}/bin;${OpenCASCADE_BINARY_DIR};%PATH%")

  set(OCCT_TOOLKITS TKernel TKMath TKG3d TKGeomBase TKBRep TKMesh TKXSBase TKDESTEP)

# 目标 A：线拖拽学习程序（纯 VTK）
add_executable(vtk-line-editor
  ${SRC_DIR}/LineEditor/main.cpp
  ${SRC_DIR}/LineEditor/editor.cpp
  ${SRC_DIR}/LineEditor/edge.cpp
  ${SRC_DIR}/LineEditor/model.cpp
  ${SRC_DIR}/LineEditor/MyMouseCallback.cpp
  ${INC_DIR}/LineEditor/editor.h
  ${INC_DIR}/LineEditor/edge.h
  ${INC_DIR}/LineEditor/model.h
  ${INC_DIR}/LineEditor/MyMouseCallback.h)
# 自有头文件根(Source/Include) + VTK 头文件；cpp 里写 #include "LineEditor/xxx.h"
target_include_directories(vtk-line-editor PRIVATE ${INC_DIR})
target_link_libraries(vtk-line-editor PRIVATE ${VTK_LIBRARIES})
target_compile_options(vtk-line-editor PRIVATE /utf-8)
set_target_properties(vtk-line-editor PROPERTIES VS_DEBUGGER_ENVIRONMENT "${VTK_DBG_PATH}")

# 目标 B：STEP 读取演示（纯 OCCT）
add_executable(step-info ${SRC_DIR}/StepInfo/stepinfo.cpp)
target_include_directories(step-info PRIVATE ${INC_DIR} ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(step-info PRIVATE ${OCCT_TOOLKITS})
target_compile_options(step-info PRIVATE /utf-8)
set_target_properties(step-info PROPERTIES 
VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
VS_DEBUGGER_ENVIRONMENT "${OCCT_DBG_PATH}")

# 目标 C：STEP -> VTK 显示（VTK + OCCT）
add_executable(step-view
  ${SRC_DIR}/StepViewer/stepview.cpp
  ${SRC_DIR}/StepViewer/ShapeToPolyData.cpp
  ${SRC_DIR}/StepViewer/StepInteractorStyle.cpp
  ${INC_DIR}/StepViewer/ShapeToPolyData.h
  ${INC_DIR}/StepViewer/TopologyIndex.h
  ${INC_DIR}/StepViewer/StepInteractorStyle.h)
target_include_directories(step-view PRIVATE ${INC_DIR} ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(step-view PRIVATE ${VTK_LIBRARIES} ${OCCT_TOOLKITS})
target_compile_options(step-view PRIVATE /utf-8)
set_target_properties(step-view PROPERTIES
VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
VS_DEBUGGER_ENVIRONMENT "${OCCT_DBG_PATH}")
 
vtk_module_autoinit(TARGETS vtk-line-editor step-view MODULES ${VTK_LIBRARIES})



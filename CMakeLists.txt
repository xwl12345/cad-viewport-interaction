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

# ---------- 统一输出目录：Output/，VS 多配置会自动再分 Release/Debug ----------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Output")

# ---------- 第 2 步：第三方库根目录（手动整理的安装树，无 find_package 配置） ----------
set(VTK_ROOT "" CACHE PATH "VTK 根目录（其下含 include/ 与 lib/）")
if(NOT VTK_ROOT)
  message(FATAL_ERROR
    "请指定 VTK 根目录，例如：cmake -S . -B build -DVTK_ROOT=D:/path/to/VTK-9.3.0")
endif()

set(OCCT_ROOT "" CACHE PATH "OCCT 根目录（其下含 include/ 与 lib/）")
if(NOT OCCT_ROOT)
  message(FATAL_ERROR
    "请指定 OCCT 根目录，例如：cmake -S . -B build -DOCCT_ROOT=D:/path/to/OCCT")
endif()

# ---------- 第 3 步：三个可执行目标 ----------

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
target_include_directories(vtk-line-editor PRIVATE
  ${INC_DIR}
  ${VTK_ROOT}/include)

# 目标 B：STEP 读取演示（纯 OCCT）
add_executable(step-info
  ${SRC_DIR}/StepInfo/stepinfo.cpp)
target_include_directories(step-info PRIVATE
  ${INC_DIR}
  ${OCCT_ROOT}/include)

# 目标 C：STEP -> VTK 显示（VTK + OCCT）
add_executable(step-view
  ${SRC_DIR}/StepViewer/stepview.cpp
  ${SRC_DIR}/StepViewer/ShapeToPolyData.cpp
  ${INC_DIR}/StepViewer/ShapeToPolyData.h)
target_include_directories(step-view PRIVATE
  ${INC_DIR}
  ${VTK_ROOT}/include
  ${OCCT_ROOT}/include)

# ---------- 第 4 步：链接库（Windows + MSVC，学习期一次性 GLOB 全部 .lib） ----------
if(MSVC)
  file(GLOB VTK_LIBS  "${VTK_ROOT}/lib/Release/*.lib")
  file(GLOB OCCT_LIBS "${OCCT_ROOT}/lib/release/*.lib")

  target_link_libraries(vtk-line-editor PRIVATE ${VTK_LIBS})
  target_link_libraries(step-info        PRIVATE ${OCCT_LIBS})
  target_link_libraries(step-view        PRIVATE ${VTK_LIBS} ${OCCT_LIBS})

  # 源文件为 UTF-8（含中文注释），让 MSVC 按 UTF-8 解析，避免 C4819/C2065
  target_compile_options(vtk-line-editor PRIVATE /utf-8)
  target_compile_options(step-info        PRIVATE /utf-8)
  target_compile_options(step-view        PRIVATE /utf-8)
endif()

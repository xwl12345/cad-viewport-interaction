cmake_minimum_required(VERSION 3.16)
project(vtk-line-editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- VTK 根目录（其下应包含 include/ 与 lib/）----
# 学习用的这份 VTK 是手动整理的安装树，没有可用的 find_package 配置，
# 因此直接指定根目录并链接导入库；暂只支持 Release 构建。
# 若换用官方安装版 VTK，可改回标准写法：
#   find_package(VTK REQUIRED)
#   target_link_libraries(vtk-line-editor PRIVATE ${VTK_LIBRARIES})
set(VTK_ROOT "" CACHE PATH "VTK 根目录（其下含 include/ 与 lib/）")
if(NOT VTK_ROOT)
  message(FATAL_ERROR
    "请指定 VTK 根目录，例如：cmake -S . -B build -DVTK_ROOT=D:/path/to/VTK-9.3.0")
endif()

add_executable(vtk-line-editor
  main.cpp
  editor.cpp
  edge.cpp
  model.cpp
  MyMouseCallback.cpp)

target_include_directories(vtk-line-editor PRIVATE "${VTK_ROOT}/include")

if(MSVC)
  file(GLOB VTK_LIBS "${VTK_ROOT}/lib/Release/*.lib")
  target_link_libraries(vtk-line-editor PRIVATE ${VTK_LIBS})
endif()

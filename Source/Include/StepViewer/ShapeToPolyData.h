// ShapeToPolyData.h —— 把 OCCT 的 TopoDS_Shape 三角化并填充为 vtkPolyData
#pragma once
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <TopoDS.hxx>
class ShapeToPolyData
{
public:
	void transferToVtk(const TopoDS_Shape& oneShape, vtkPoints* points, vtkPolyData* polyData);
};

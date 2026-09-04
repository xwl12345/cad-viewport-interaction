// ShapeToPolyData.h —— 把 OCCT 的 TopoDS_Shape 三角化并填充为 vtkPolyData
#pragma once
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <TopoDS.hxx>

#include <vector>

#include <vtkIdTypeArray.h>

#include "StepViewer/TopologyIndex.h"
class ShapeToPolyData
{
public:
	void transferToVtk(const TopoDS_Shape& oneShape, vtkPoints* points, vtkPolyData* polyData);
private:
	std::vector<int> cellToFace;
	std::vector<std::vector<vtkIdType>> faceToCell;
};

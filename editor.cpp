#include "editor.h"
vtkSmartPointer<vtkActor> editor::createEdgeActor(const double p1[3],const double p2[3])
{
	m_model.addEdge(p1, p2);
	vtkNew<vtkPoints> points;
	points->InsertNextPoint(p1[0], p1[1], p1[2]);
	points->InsertNextPoint(p2[0], p2[1], p2[2]);
	//创建新的CellArray对象来存储线段
    vtkNew<vtkCellArray> lines;
	vtkIdType pointIds[2] = { 0,1 };
	lines->InsertNextCell(2, pointIds);

	//创建新的polyData对象打包存储每个点和线和拓扑关系
	vtkNew<vtkPolyData> lineData;
	lineData->SetPoints(points);
	lineData->SetLines(lines);

	//创建映射器，把数据转化为图形系统可以识别的图元，为渲染做准备
	vtkNew<vtkPolyDataMapper> polyDataMapper;
	polyDataMapper->SetInputData(lineData);
	
	//创建actor，挂载映射器
	vtkNew<vtkActor> actor;
	actor->SetMapper(polyDataMapper);
	actor->GetProperty()->SetLineWidth(3.0);
	actor->GetProperty()->SetColor(0.9, 0.9, 0.9);

	m_model.map.emplace(actor, m_model.edges.back());
    return actor;
}

model& editor::getModel()
{
	return m_model;
}

#include <iostream>
#include <string>
#include <Windows.h>
#include "StepViewer/ShapeToPolyData.h"
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>

#include <vtkAutoInit.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellData.h>
#include <vtkProperty.h>
#include "StepViewer/StepInteractorStyle.h"

int main(int argc,char* argv[])
{
	SetConsoleOutputCP(CP_UTF8);
	std::string file = (argc > 1) ? argv[1] : "models/TCMT1107_4.stp";
	STEPControl_Reader reader;
	IFSelect_ReturnStatus status = reader.ReadFile(file.c_str());
	if (status != IFSelect_RetDone)
	{
		std::cout << "STP读取文件失败,状态码为:" <<(int)status<< std::endl;
		return 1;
	}

	Standard_Integer transfered = reader.TransferRoots();

	TopoDS_Shape oneShape = reader.OneShape();
	ShapeToPolyData transferer;
	vtkNew<vtkPoints> points;
	vtkNew<vtkPolyData> polyData;
	vtkNew<vtkPoints> edgePoints;
	vtkNew<vtkPolyData> edgePolyData;
	vtkNew<vtkPoints> vertexPoints;
	vtkNew<vtkPolyData> vertexPolyData;
	TopologyIndex relationIndex;
	transferer.transferToVtk(oneShape, points, polyData,relationIndex);
	transferer.transferEdgeToVtk(edgePoints, edgePolyData, relationIndex);
	transferer.transferVertexToVtk(vertexPoints, vertexPolyData, relationIndex);
	vtkNew<vtkUnsignedCharArray> cellColors;
	vtkNew<vtkUnsignedCharArray> edgeCellColors;
	vtkNew<vtkUnsignedCharArray> vertexCellColors;
	cellColors->SetNumberOfComponents(3);
	edgeCellColors->SetNumberOfComponents(3);
	vertexCellColors->SetNumberOfComponents(3);
	for (vtkIdType i = 0;i < polyData->GetNumberOfCells();++i)
	{
		cellColors->InsertNextTuple3(200, 200, 210);
	}
	for (vtkIdType i = 0;i<edgePolyData->GetNumberOfCells();++i)
	{
		edgeCellColors->InsertNextTuple3(40, 40, 40);
	}
	for (vtkIdType i = 0;i < vertexPolyData->GetNumberOfCells();++i)
	{
		vertexCellColors->InsertNextTuple3(0, 110, 220);
	}

	polyData->GetCellData()->SetScalars(cellColors);
	edgePolyData->GetCellData()->SetScalars(edgeCellColors);
	vertexPolyData->GetCellData()->SetScalars(vertexCellColors);

	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputData(polyData);
	mapper->SetScalarModeToUseCellData();
	mapper->SetColorModeToDirectScalars();

	vtkNew<vtkPolyDataMapper> edgeMapper;
	edgeMapper->SetInputData(edgePolyData);
	edgeMapper->SetScalarModeToUseCellData();
	edgeMapper->SetColorModeToDirectScalars();

	vtkNew<vtkPolyDataMapper> vertexMapper;
	vertexMapper->SetInputData(vertexPolyData);
	vertexMapper->SetScalarModeToUseCellData();
	vertexMapper->SetColorModeToDirectScalars();
 
	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);

	vtkNew<vtkActor> edgeActor;
	edgeActor->SetMapper(edgeMapper);
	edgeActor->GetProperty()->SetLineWidth(4.5f);
	edgeActor->PickableOff();

	vtkNew<vtkActor> vertexActor;
	vertexActor->SetMapper(vertexMapper);
	vertexActor->GetProperty()->SetPointSize(8.0f);

	vtkNew<vtkRenderer> renderer;
	renderer->AddActor(actor);
	renderer->AddActor(edgeActor);
	renderer->AddActor(vertexActor);

	vtkNew<vtkRenderWindow> window;
	window->AddRenderer(renderer);

	vtkNew<vtkRenderWindowInteractor> interactor;
	interactor->SetRenderWindow(window);

	vtkNew<StepInteractorStyle> style;

	style->SetTopologyIndex(relationIndex);
	style->SetPolyData(polyData);
	style->SetEdgePolyData(edgePolyData);
	style->SetVertexPolyData(vertexPolyData);
	style->SetEdgeActor(edgeActor);
	style->SetFaceActor(actor);
	style->SetVertexActor(vertexActor);
	
	interactor->SetInteractorStyle(style);
	style->InitRubberBand(renderer);
	style->SetSelectMode(SelectMode::Face);
	renderer->ResetCamera();
	window->Render();
	interactor->Start();

}

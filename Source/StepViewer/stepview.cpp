// stepview.cpp —— 你来写：程序入口（读 STEP -> 调转换 -> 弹出 VTK 窗口显示）
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
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
int main(int argc,char* argv[])
{
	SetConsoleOutputCP(CP_UTF8);
	std::string file = (argc > 1) ? argv[1] : "D:/VS_Project/Muyan/TCMT1107_4.stp";
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
	TopologyIndex relationIndex;
	transferer.transferToVtk(oneShape, points, polyData,relationIndex);

	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputData(polyData);

	vtkNew<vtkActor> actor;
	actor->SetMapper(mapper);

	vtkNew<vtkRenderer> renderer;
	renderer->AddActor(actor);

	vtkNew<vtkRenderWindow> window;
	window->AddRenderer(renderer);

	vtkNew<vtkRenderWindowInteractor> interactor;
	interactor->SetRenderWindow(window);

	std::cout << relationIndex.vertexToEdges.size();
	renderer->ResetCamera();
	window->Render();
	interactor->Start();

}

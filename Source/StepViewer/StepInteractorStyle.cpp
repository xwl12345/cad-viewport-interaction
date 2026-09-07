#include "StepViewer/StepInteractorStyle.h"
#include <vtkNew.h>
#include <vtkCellPicker.h>
#include <vtkRenderer.h>
#include <vtkIdTypeArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <iostream>
void StepInteractorStyle::OnLeftButtonDown()
{

	vtkNew<vtkCellPicker> picker;

	int* clickpos = this->GetInteractor()->GetEventPosition();
	int position_x=clickpos[0];
	int position_y=clickpos[1];
	std::cout << "x坐标为：" << position_x << "，y坐标为： " << position_y << std::endl;
	vtkRenderer* renderer = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	picker->SetTolerance(0.05);
	picker->Pick(position_x,position_y,0,renderer);

	vtkActor* actor=picker->GetActor();
	vtkIdType cellId=picker->GetCellId();
	
	std::cout << "cellId:" << cellId << std::endl;
}

// 右键缩放：Down/Up 成对留空、都不调基类，右键拖动就不再缩放
void StepInteractorStyle::OnRightButtonDown()
{
	this->StartRotate();
}

void StepInteractorStyle::OnRightButtonUp()
{
	this->EndRotate();
}


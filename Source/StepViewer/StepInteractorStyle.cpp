#include "StepViewer/StepInteractorStyle.h"
#include "StepViewer/TopologyIndex.h"
#include <vtkNew.h>
#include <vtkCellPicker.h>
#include <vtkRenderer.h>
#include <vtkIdTypeArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkCellData.h>
#include <vtkUnsignedCharArray.h>
#include <iostream>
#include <vector>

void StepInteractorStyle::OnLeftButtonDown()
{

	vtkNew<vtkCellPicker> picker;

	int* clickpos = this->GetInteractor()->GetEventPosition();
	int position_x = clickpos[0];
	int position_y = clickpos[1];
	std::cout << "x坐标为：" << position_x << "，y坐标为： " << position_y << std::endl;
	vtkRenderer* renderer = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	picker->SetTolerance(0.005);
	picker->Pick(position_x, position_y, 0, renderer);

	vtkActor* actor = picker->GetActor();
	vtkIdType cellId = picker->GetCellId();

	std::cout << "cellId:" << cellId << std::endl;
	auto* colors = vtkUnsignedCharArray::SafeDownCast(m_polyData->GetCellData()->GetScalars());
	if (!colors || m_relationIndex == nullptr)
	{
		return;
	}
	if (cellId < 0)
	{
		std::cout << "未选中体" << std::endl; 

		if (m_highLightFaceId != -2)
		{
			for (vtkIdType c : m_relationIndex->faceToCell[m_highLightFaceId])
			{
				colors->SetTuple3(c, 200, 200, 210);
			}
			m_highLightFaceId = -2;
		}
	}
	else
	{
		vtkIdType onPickedFaceId = m_relationIndex->cellToFace[cellId];
		vtkIdType onPickedSolidId = m_relationIndex->faceToSolid[onPickedFaceId];
		std::cout << "当前选中的是第" << onPickedFaceId << "个面" << std::endl;
		std::cout << "当前选中的是第" << onPickedSolidId << "个体" << std::endl;
		if (m_highLightFaceId == onPickedFaceId)
		{
			return;
		}

		if (m_highLightFaceId != -2)
		{
			for (vtkIdType c : m_relationIndex->faceToCell[m_highLightFaceId])
			{
				colors->SetTuple3(c, 200, 200, 210);
			}
			m_highLightFaceId = -2;
		}
		m_highLightFaceId = onPickedFaceId;

		for (vtkIdType c : m_relationIndex->faceToCell[m_highLightFaceId])
		{
			colors->SetTuple3(c, 255, 255, 0);
		}

	}
	colors->Modified();
	this->GetInteractor()->GetRenderWindow()->Render();
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

void StepInteractorStyle::SetTopologyIndex(TopologyIndex& relationIndex)
{
	this->m_relationIndex = &relationIndex;
}

void StepInteractorStyle::SetPolyData(vtkPolyData* polydata)
{
	this->m_polyData = polydata;
}


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

	if (!colors)
	{
		return;
	}

	std::vector<vtkIdType> onPicked;
	CollectPickedArray(cellId, onPicked);

	
	if (onPicked.empty())
	{
		std::cout << "未选中体" << std::endl; 

		if (!m_lastCells.empty())
		{
			for (vtkIdType c : m_lastCells)
			{
				colors->SetTuple3(c, 200, 200, 210);
			}
			m_lastCells.clear();
		}
	}
	else
	{
		if (m_lastCells == onPicked)
		{
			return;
		}

		if (!m_lastCells.empty())
		{
			for (vtkIdType c : m_lastCells)
			{
				colors->SetTuple3(c, 200, 200, 210);
			}
			m_lastCells.clear();
		}
		m_lastCells = onPicked;

		for (vtkIdType c : m_lastCells)
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

void StepInteractorStyle::SetSelectMode(SelectMode m)
{
	this->ClearThisHighLightCell();
	m_selectMode = m;
}

void StepInteractorStyle::OnChar()
{
	switch(this->GetInteractor()->GetKeyCode())
	{
	case 'f': SetSelectMode(SelectMode::Face);std::cout << "面模式" << std::endl;return;
	case 's': SetSelectMode(SelectMode::Solid);std::cout << "体模式" << std::endl;return;
	default:vtkInteractorStyleTrackballCamera::OnChar();break;
	}
}



void StepInteractorStyle::CollectPickedArray(vtkIdType cellId, std::vector<vtkIdType>& out)
{
	if (cellId == -1 || m_relationIndex == nullptr)
		return;
	int faceId = m_relationIndex->cellToFace[cellId];
	//面模式
	if (m_selectMode == SelectMode::Face)
	{
		out = m_relationIndex->faceToCell[faceId];
	}
	//体模式
	if (m_selectMode == SelectMode::Solid)
	{
		int solidId= m_relationIndex->faceToSolid[faceId];
		out.clear();
		for (int f = 0;f < (int)m_relationIndex->faceToSolid.size();++f)
		{
			if (m_relationIndex->faceToSolid[f] == solidId)
			{
				for (vtkIdType c :m_relationIndex->faceToCell[f])
				{
					out.emplace_back(c);
				}
			}
		}
	}
}

void StepInteractorStyle::ClearThisHighLightCell()
{
	auto* colors = vtkUnsignedCharArray::SafeDownCast(m_polyData->GetCellData()->GetScalars());
	if (!colors||m_relationIndex==nullptr||m_lastCells.empty())
	{
		return;
	}
	for (vtkIdType c : m_lastCells)
	{
		colors->SetTuple3(c, 200, 200, 210);
	}
	m_lastCells.clear();
	colors->Modified();
	this->GetInteractor()->GetRenderWindow()->Render();
}


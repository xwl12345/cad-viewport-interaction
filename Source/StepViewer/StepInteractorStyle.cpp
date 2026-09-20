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
#include <vtkHardwareSelector.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <iostream>
#include <vector>
#include <set>
void StepInteractorStyle::OnLeftButtonDown()
{
	m_leftButtonDown = true;
	m_select = false;

	int* clickpos = this->GetInteractor()->GetEventPosition();
	m_position_x = clickpos[0];
	m_position_y = clickpos[1];
	std::cout << "x坐标为：" << m_position_x << "，y坐标为： " << m_position_y << std::endl;
	
}

void StepInteractorStyle::OnRightButtonDown()
{
	this->StartRotate();
}

void StepInteractorStyle::OnRightButtonUp()
{
	this->EndRotate();
}

void StepInteractorStyle::OnMouseMove()
{
	
	if (m_leftButtonDown==true)
	{
		int* clickpos = this->GetInteractor()->GetEventPosition();
		int dx = clickpos[0]-m_position_x;
		int dy = clickpos[1]-m_position_y;
		if (dx * dx + dy * dy > 25)
		{
			m_select = true;
		}
		
	}
	vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void StepInteractorStyle::OnLeftButtonUp()
{
	vtkNew<vtkCellPicker> picker;

	int* clickpos = this->GetInteractor()->GetEventPosition();
	int position_x = clickpos[0];
	int position_y = clickpos[1];
	vtkRenderer* renderer = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	picker->SetTolerance(0.005);

	if (m_select == false)
	{
		picker->Pick(position_x, position_y, 0, renderer);
		vtkIdType cellId = picker->GetCellId();
		std::cout << "点选" << std::endl << "x坐标为：" << position_x << "，y坐标为： " << position_y << std::endl;
		std::vector<vtkIdType> seed;
		if(cellId!=-1)
		{
			seed.emplace_back(cellId);
		}
		std::vector<vtkIdType> onPicked;
		CollectPickedArray(seed, onPicked);
		HighLight(onPicked);
	}
	else
	{
		std::cout << "框选" << std::endl;
		
		int xmin = std::min(position_x, m_position_x);
		int xmax = std::max(position_x, m_position_x);
		int ymin = std::min(position_y, m_position_y);
		int ymax = std::max(position_y, m_position_y);

		std::vector<vtkIdType> seeds;
	
		std::vector<vtkIdType> onPicked;
		vtkNew<vtkHardwareSelector> hs;
		hs->SetRenderer(renderer);
		hs->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_CELLS);
		hs->SetArea(xmin,ymin, xmax,  ymax);
		vtkSelection* sel = hs->Select();
		if(sel)
		{
			for (int i = 0;i < sel->GetNumberOfNodes();++i)
			{
				vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(sel->GetNode(i)->GetSelectionList());
				if (ids)
				{
					for (int j = 0;j < ids->GetNumberOfTuples();++j)
					{
						vtkIdType cellId = ids->GetValue(j);
						seeds.emplace_back(cellId);
					}
				}
			}
			sel->Delete();
		}
		CollectPickedArray(seeds, onPicked);
		HighLight(onPicked);
 	}
	m_leftButtonDown = false;
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



void StepInteractorStyle::CollectPickedArray(const std::vector<vtkIdType>& seeds, std::vector<vtkIdType>& out)
{
	out.clear();
	if (m_relationIndex == nullptr || seeds.empty())
	{
		return;
	}
	std::set<vtkIdType> faceSet;
	for (int i = 0;i < seeds.size();++i)
	{
		vtkIdType now = m_relationIndex->cellToFace[seeds[i]];
		faceSet.emplace(now);
	}
	//面模式
	if (m_selectMode == SelectMode::Face)
	{
		for (auto faceId:faceSet)
		{
			for (auto c : m_relationIndex->faceToCell[faceId])
			{
				out.emplace_back(c);
			}
		}
	}
	//体模式
	if (m_selectMode == SelectMode::Solid)
	{
		std::set<vtkIdType> solidIds;
		for (auto faceId : faceSet)
		{
			solidIds.insert(m_relationIndex->faceToSolid[faceId]);
		}
		out.clear();
		for(auto solidId:solidIds)
		{
			for (int f = 0;f < (int)m_relationIndex->faceToSolid.size();++f)
			{
				if (m_relationIndex->faceToSolid[f] == solidId)
				{
					for (vtkIdType c : m_relationIndex->faceToCell[f])
					{
						out.emplace_back(c);
					}
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

void StepInteractorStyle::HighLight(std::vector<vtkIdType> onPicked)
{
	auto* colors = vtkUnsignedCharArray::SafeDownCast(m_polyData->GetCellData()->GetScalars());

	if (!colors)
	{
		return;
	}

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


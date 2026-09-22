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
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCoordinate.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <iostream>
#include <vector>
#include <set>
void StepInteractorStyle::OnLeftButtonDown()
{
	m_rubberActor->VisibilityOff();
	m_leftButtonDown = true;
	m_select = false;

	int* clickpos = this->GetInteractor()->GetEventPosition();
	m_position_x = clickpos[0];
	m_position_y = clickpos[1];
	/*std::cout << "x坐标为：" << m_position_x << "，y坐标为： " << m_position_y << std::endl;
	*/
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
		int x1 = clickpos[0];
		int y1 = clickpos[1];

		int dx = clickpos[0]-m_position_x;
		int dy = clickpos[1]-m_position_y;
		if (dx * dx + dy * dy > 25)
		{
			m_select = true;
		}
		if (m_leftButtonDown && m_select)
		{
			m_rubberActor->VisibilityOn();
			UpdateRubberBand(m_position_x, m_position_y, x1, y1);
			this->GetInteractor()->GetRenderWindow()->Render();
		}
	}
	vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void StepInteractorStyle::OnLeftButtonUp()
{
	m_rubberActor->VisibilityOff();
	this->GetInteractor()->GetRenderWindow() -> Render();
	vtkNew<vtkCellPicker> picker;

	int* clickpos = this->GetInteractor()->GetEventPosition();
	int position_x = clickpos[0];
	int position_y = clickpos[1];
	vtkRenderer* renderer = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	double tol;
	tol = 0.005;
	picker->SetTolerance(tol);

	if (m_select == false)
	{
		picker->Pick(position_x, position_y, 0, renderer);
		vtkIdType cellId = picker->GetCellId();
		/*std::cout << "点选" << std::endl << "x坐标为：" << position_x << "，y坐标为： " << position_y << std::endl;*/
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
		/*std::cout << "框选" << std::endl;*/
		
		int xmin = std::min(position_x, m_position_x);
		int xmax = std::max(position_x, m_position_x);
		int ymin = std::min(position_y, m_position_y);
		int ymax = std::max(position_y, m_position_y);

		int* winSz = this->GetInteractor()->GetRenderWindow()->GetSize(); 
		int w = winSz[0], h = winSz[1]; //得到窗口宽高，防止框选越界
		xmin = std::max(0, std::min(xmin, w - 1));
		xmax = std::max(0, std::min(xmax, w - 1));
		ymin = std::max(0, std::min(ymin, h - 1));
		ymax = std::max(0, std::min(ymax, h - 1));

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
	m_faceActor->SetPickable((m == SelectMode::Face) || (m == SelectMode::Solid));
	m_edgeActor->SetPickable(m == SelectMode::Edge);
	m_vertexActor->SetPickable(m == SelectMode::Vertex);
}

void StepInteractorStyle::OnChar()
{
	switch(this->GetInteractor()->GetKeyCode())
	{
	case 'f': 
		SetSelectMode(SelectMode::Face);
		std::cout << "面模式" << std::endl;
		return;
	case 's': 
		SetSelectMode(SelectMode::Solid);
		std::cout << "体模式" << std::endl;
		return;
	case 'e': 
		SetSelectMode(SelectMode::Edge);
		std:: cout << "线模式" << std::endl;
		return;
	case 'v':
		SetSelectMode(SelectMode::Vertex);
		std::cout << "点模式" << std::endl;
		return;
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

	vtkIdType layerCellCount = 0;
	if (m_selectMode == SelectMode::Vertex)
	{
		layerCellCount = (vtkIdType)m_relationIndex->vertexCellToVertex.size();
	}
	else if (m_selectMode == SelectMode::Edge)
	{
		layerCellCount = (vtkIdType)m_relationIndex->edgeCellToEdge.size();
	}
	else
	{
		layerCellCount = (vtkIdType)m_relationIndex->cellToFace.size();
	}

	std::vector<vtkIdType> safe;

	for (vtkIdType s : seeds)
	{
		if (s >= 0 && s < layerCellCount) 
		{
			safe.push_back(s);
		}
	}
	if (safe.empty())
	{
		return;
	}
	//点模式
	if (m_selectMode == SelectMode::Vertex)
	{
		std::set<vtkIdType> vertexSet;
		for (int i = 0;i < safe.size();++i)
		{
			vtkIdType now = m_relationIndex->vertexCellToVertex[safe[i]];
			vertexSet.emplace(now);
		}
		for (auto vertexId : vertexSet)
		{
			for (auto v : m_relationIndex->vertexToVertexCell[vertexId])
			{
				out.emplace_back(v);
			}
		}
		return;
	}
	//线模式
	if (m_selectMode == SelectMode::Edge)
	{
		std::set<vtkIdType>  edgeSet;
		for (int i = 0;i < safe.size();++i)
		{
			vtkIdType now = m_relationIndex->edgeCellToEdge[safe[i]];
			edgeSet.emplace(now);
		}
		for (auto edgeId : edgeSet)
		{
			for (auto e : m_relationIndex->edgeToEdgeCell[edgeId])
			{
				out.emplace_back(e);
			}
		}
		return;
	}


	//面体选中
	{
		std::set<vtkIdType> faceSet;
		for (int i = 0;i < safe.size();++i)
		{
			vtkIdType now = m_relationIndex->cellToFace[safe[i]];
			faceSet.emplace(now);
		}
		//面模式
		if (m_selectMode == SelectMode::Face)
		{
			for (auto faceId : faceSet)
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
			for (auto solidId : solidIds)
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
}

void StepInteractorStyle::ClearThisHighLightCell()
{
	
	if (m_selectMode == SelectMode::Edge)
	{
		auto* colors = vtkUnsignedCharArray::SafeDownCast(m_edgePolyData->GetCellData()->GetScalars());
		if (!colors || m_relationIndex == nullptr || m_lastCells.empty())
		{
			return;
		}
		for (vtkIdType c : m_lastCells)
		{
			colors->SetTuple3(c, 40, 40, 40);
		}
		colors->Modified();
	}
	else if (m_selectMode == SelectMode::Face || m_selectMode == SelectMode::Solid)
	{
		auto* colors = vtkUnsignedCharArray::SafeDownCast(m_polyData->GetCellData()->GetScalars());
		if (!colors || m_relationIndex == nullptr || m_lastCells.empty())
		{
			return;
		}
		for (vtkIdType c : m_lastCells)
		{
			colors->SetTuple3(c, 200, 200, 210);
		}
		colors->Modified();
	}
	else
	{
		auto* colors = vtkUnsignedCharArray::SafeDownCast(m_vertexPolyData->GetCellData()->GetScalars());
		if (!colors || m_relationIndex == nullptr || m_lastCells.empty())
		{
			return;
		}
		for (vtkIdType c : m_lastCells)
		{
			colors->SetTuple3(c, 0,110,220);
		}
		colors->Modified();
	}
	m_lastCells.clear();
	
	this->GetInteractor()->GetRenderWindow()->Render();
}

void StepInteractorStyle::HighLight(std::vector<vtkIdType> onPicked)
{
	vtkUnsignedCharArray* colors;

	if (m_selectMode == SelectMode::Edge)
	{
		colors= vtkUnsignedCharArray::SafeDownCast(m_edgePolyData->GetCellData()->GetScalars());
	}
	else if(m_selectMode==SelectMode::Face||m_selectMode==SelectMode::Solid)
	{
		colors = vtkUnsignedCharArray::SafeDownCast(m_polyData->GetCellData()->GetScalars());
	}
	else
	{
		colors = vtkUnsignedCharArray::SafeDownCast(m_vertexPolyData->GetCellData()->GetScalars());
	}
	if (!colors)
	{
		return;
	}

	if (onPicked.empty())
	{

		if (!m_lastCells.empty())
		{
			ClearThisHighLightCell();
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
			ClearThisHighLightCell();
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

void StepInteractorStyle::UpdateRubberBand(int x0, int y0, int x1, int y1)
{
	m_rubberPoints->SetPoint(0,x0, y0,0);
	m_rubberPoints->SetPoint(1, x1, y0, 0);
	m_rubberPoints->SetPoint(2, x1, y1, 0);
	m_rubberPoints->SetPoint(3, x0, y1, 0);
	m_rubberPoints->SetPoint(4, x0, y0, 0);
	m_rubberPoints->Modified();

}

void StepInteractorStyle::InitRubberBand(vtkRenderer* renderer)
{
	m_rubberPoints = vtkPoints::New();
	for (int i = 0;i < 5;++i)
	{
		m_rubberPoints->InsertNextPoint(0, 0, 0);
	}
	vtkNew<vtkPolyLine> outLine;
	outLine->GetPointIds()->SetNumberOfIds(5);
	for (int i = 0;i < 5;++i)
	{
		outLine->GetPointIds()->SetId(i, i);
	}

	vtkNew<vtkCellArray> line;
	line->InsertNextCell(outLine);

	m_rubberPolyData = vtkSmartPointer<vtkPolyData>::New();
	m_rubberPolyData->SetPoints(m_rubberPoints);
	m_rubberPolyData->SetLines(line);

	vtkNew<vtkCoordinate> coord;
	coord->SetCoordinateSystemToDisplay();

	vtkNew<vtkPolyDataMapper2D> rubberMapper2D;
	rubberMapper2D->SetInputData(m_rubberPolyData);
	rubberMapper2D->SetTransformCoordinate(coord);

	m_rubberActor = vtkSmartPointer<vtkActor2D>::New();
	m_rubberActor->SetMapper(rubberMapper2D);
	m_rubberActor->GetProperty()->SetColor(1.0, 1.0, 0.0);
	m_rubberActor->GetProperty()->SetDisplayLocationToForeground();
	m_rubberActor->VisibilityOff();

	renderer->AddActor2D(m_rubberActor);
}

void StepInteractorStyle::SetEdgeActor(vtkActor* edgeActor)
{
	m_edgeActor = edgeActor;
}

void StepInteractorStyle::SetFaceActor(vtkActor* faceActor)
{
	m_faceActor = faceActor;
}

void StepInteractorStyle::SetEdgePolyData(vtkPolyData* edgePolyData)
{
	m_edgePolyData = edgePolyData;
}

void StepInteractorStyle::SetVertexPolyData(vtkPolyData* vertexPolyData)
{
	m_vertexPolyData = vertexPolyData;
}

void StepInteractorStyle::SetVertexActor(vtkActor* vertexActor)
{
	m_vertexActor = vertexActor;
}


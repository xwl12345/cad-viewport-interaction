#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
class TopologyIndex;
class vtkActor;
enum class SelectMode
{
	Solid,
	Face,
	Edge,
	Vertex
};
class vtkPoints;
class vtkActor2D;
class vtkRenderer;
class StepInteractorStyle :public vtkInteractorStyleTrackballCamera
{
public:
	static StepInteractorStyle* New()
	{
		return new StepInteractorStyle;
	}
	void OnLeftButtonDown() override;
	void OnRightButtonDown() override;
	void OnRightButtonUp() override;
	void OnMouseMove() override;
	void OnLeftButtonUp() override;
	void SetTopologyIndex(TopologyIndex& relationIndex);
	void SetPolyData(vtkPolyData* polydata); 
	void SetSelectMode(SelectMode m);
	void OnChar()override;
	void InitRubberBand(vtkRenderer* renderer);
	void SetEdgeActor(vtkActor* edgeActor);
	void SetFaceActor(vtkActor* faceActor);
	void SetEdgePolyData(vtkPolyData* edgePolyData);
	void SetVertexPolyData(vtkPolyData* vertexPolyData);
	void SetVertexActor(vtkActor* vertexActor);

private:
	TopologyIndex* m_relationIndex=nullptr;
	vtkPolyData* m_polyData=nullptr;
	vtkPolyData* m_edgePolyData = nullptr;
	vtkPolyData* m_vertexPolyData = nullptr;

	vtkSmartPointer<vtkPoints> m_rubberPoints;
	vtkSmartPointer<vtkPolyData> m_rubberPolyData;
	vtkSmartPointer<vtkActor2D> m_rubberActor;

	vtkActor* m_vertexActor = nullptr;
	vtkActor* m_edgeActor=nullptr;
	vtkActor* m_faceActor = nullptr;

	bool m_leftButtonDown=false;
	bool m_select = false;
	int m_position_x=0;
	int m_position_y=0;
	std::vector<vtkIdType> m_lastCells;
	SelectMode m_selectMode = SelectMode::Face;
	void CollectPickedArray(const std::vector<vtkIdType>& seeds, std::vector<vtkIdType>& out);
	void ClearThisHighLightCell();
	void HighLight(std::vector<vtkIdType> cells);
	void UpdateRubberBand(int x0, int y0, int x1, int y1);
};
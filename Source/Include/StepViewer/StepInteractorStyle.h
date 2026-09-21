#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
class TopologyIndex;
enum class SelectMode
{
	Solid,
	Face
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

private:
	TopologyIndex* m_relationIndex=nullptr;
	vtkPolyData* m_polyData=nullptr;

	vtkSmartPointer<vtkPoints> m_rubberPoints;
	vtkSmartPointer<vtkPolyData> m_rubberPolyData;
	vtkSmartPointer<vtkActor2D> m_rubberActor;

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
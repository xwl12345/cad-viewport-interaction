#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyData.h>
class TopologyIndex;
enum class SelectMode
{
	Solid,
	Face
};
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
private:
	TopologyIndex* m_relationIndex=nullptr;
	vtkPolyData* m_polyData=nullptr;
	bool m_leftButtonDown=false;
	bool m_select = false;
	int m_position_x=0;
	int m_position_y=0;
	std::vector<vtkIdType> m_lastCells;
	SelectMode m_selectMode = SelectMode::Face;
	void CollectPickedArray(const std::vector<vtkIdType>& seeds, std::vector<vtkIdType>& out);
	void ClearThisHighLightCell();
	void HighLight(std::vector<vtkIdType> cells);
};
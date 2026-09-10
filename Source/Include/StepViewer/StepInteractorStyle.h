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
	void SetTopologyIndex(TopologyIndex& relationIndex);
	void SetPolyData(vtkPolyData* polydata); 
	void SetSelectMode(SelectMode m);
	void OnChar()override;
private:
	TopologyIndex* m_relationIndex=nullptr;
	vtkPolyData* m_polyData=nullptr;
	std::vector<vtkIdType> m_lastCells;
	SelectMode m_selectMode = SelectMode::Face;
	void CollectPickedArray(vtkIdType cellId, std::vector<vtkIdType>& out);
	void ClearThisHighLightCell();
};
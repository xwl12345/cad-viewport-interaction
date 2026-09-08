#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyData.h>
class TopologyIndex;
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
private:
	TopologyIndex* m_relationIndex=nullptr;
	vtkPolyData* m_polyData=nullptr;
	vtkIdType m_highLightFaceId = -2;
};
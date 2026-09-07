#pragma once
#include <vtkInteractorStyleTrackballCamera.h>
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

private:
	TopologyIndex* m_relationIndex=nullptr;
};
#pragma once
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <unordered_map>
#include "model.h"
class editor
{
public:
	vtkSmartPointer<vtkActor> createEdgeActor(const double p1[3], const double p2[3]);
	model& getModel();
	

private:
	model m_model;
};
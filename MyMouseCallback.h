#pragma once
#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkCellPicker.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkNew.h>
#include <iostream>
#include <vtkProperty.h>
#include <unordered_map>
#include <memory>
#include "model.h"
class MyMouseCallback:public vtkCommand
{
public:
	static MyMouseCallback* New()
	{ 
		return new MyMouseCallback; 
	}
	virtual void  Execute(vtkObject* caller, unsigned long eventId, void* callData) override
	{
		//获取交互器
		vtkRenderWindowInteractor* interactor = dynamic_cast<vtkRenderWindowInteractor*> (caller);
		if (!interactor)
		{
			return;
		}

		//获取事件触发鼠标位置
		int* clickPos = interactor->GetEventPosition();
		int position_x = clickPos[0];
		int position_y = clickPos[1];

		//获取渲染器
		vtkRenderer* renderer = interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
		if (!renderer)
		{
			return;
		}

		//创建拾取器
		vtkNew<vtkCellPicker> picker;
		picker->SetTolerance(0.005);
		picker->Pick(position_x, position_y, 0, renderer);

		//获取拾取结果
		vtkActor* actor= picker->GetActor();
		vtkIdType cellId = picker->GetCellId();

		if (eventId == vtkCommand::LeftButtonPressEvent)
		{
			m_isdraging.store(false);
			m_position_x = position_x;
			m_position_y = position_y;
			//如果高亮actor已经存在就把他变回普通线
			if (highLightActor != nullptr)
			{
				highLightActor->GetProperty()->SetColor(0.9, 0.9, 0.9);
				highLightActor->GetProperty()->SetLineWidth(3.0);
				interactor->GetRenderWindow()->Render();
			}
			highLightActor = actor;
			//处理拾取结果
			if (highLightActor && cellId != -1)
			{
				std::cout << "The actor is picked" << highLightActor;
				highLightActor->GetProperty()->SetColor(1.0, 1.0, 0);
				highLightActor->GetProperty()->SetLineWidth(6.0);
			}
			else
			{
				std::cout << "No actor is picked" << std::endl;
			}
		}
		if (eventId == vtkCommand::MouseMoveEvent)
		{
			int dx = clickPos[0] - m_position_x;
			int dy = clickPos[1] - m_position_y;//检查移动位置大小
			if (dx * dx + dy * dy > 25)//超过5个像素就算拖拽
			{
				m_isdraging.store(true);
				m_delta_x = dx;
				m_delta_y = dy;
			}
		}
		if (eventId == vtkCommand::FifthButtonReleaseEvent)
		{
			if (m_isdraging.load())
			{
				edge onPicked = m_model.map[actor];
				double displayPoint[3] = { (double)(position_x+m_delta_x), (double)(position_y+m_delta_y), 0.0 };
				double worldPoint[4];  // 齐次坐标，返回 (x, y, z, w)

				renderer->SetDisplayPoint(displayPoint);
				renderer->DisplayToWorld();
				renderer->GetWorldPoint(worldPoint);

				// 归一化得到真正的三维坐标
				double worldX = worldPoint[0] / worldPoint[3];
				double worldY = worldPoint[1] / worldPoint[3];
				double worldZ = worldPoint[2] / worldPoint[3];
				onPicked.moveEdge(worldX,worldY,worldZ);
				
			}
		}
		
		
	}
protected:
	MyMouseCallback();
	~MyMouseCallback();
private:
	model m_model;
	vtkSmartPointer<vtkActor> highLightActor;
	std::atomic<bool> m_isdraging = false;//拾取标志位
	int m_position_x;
	int m_position_y;
	int m_delta_x;
	int m_delta_y;
};
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
#include "LineEditor/model.h"
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
class MyMouseCallback:public vtkCommand
{
public:
	static MyMouseCallback* New()
	{ 
		return new MyMouseCallback; 
	}
	void setModel(model* modelptr);
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

			m_isdragging.store(false);
			m_position_x = position_x;
			m_position_y = position_y;
			double pickedWorld[3];
			picker->GetPickPosition(pickedWorld);
			renderer->SetWorldPoint(pickedWorld[0], pickedWorld[1], pickedWorld[2], 1.0);
			renderer->WorldToDisplay();
			m_pickdepth = renderer->GetDisplayPoint()[2];
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
				interactor->GetRenderWindow()->Render();
				this->AbortFlagOn();
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
				this->AbortFlagOn();
				m_isdragging.store(true);
			}
		}
		if (eventId == vtkCommand::LeftButtonReleaseEvent)
		{
			if (m_isdragging.load())
			{
				this->AbortFlagOn();
				std::cout << "dragging";
				auto it = m_model->map.find(highLightActor);
				if (it == m_model->map.end())
				{
					return;
				}
				edge& onPicked = it->second;
				//起点像素转世界坐标
				renderer->SetDisplayPoint((double)m_position_x, (double)m_position_y, (double)m_pickdepth);
				renderer->DisplayToWorld();
				double W1[4];
				renderer->GetWorldPoint(W1);

				//终点像素转世界坐标
				renderer->SetDisplayPoint((double)position_x, (double)position_y, (double)m_pickdepth);
				renderer->DisplayToWorld();
				double W2[4];
				renderer->GetWorldPoint(W2);

				double Wx = W2[0] / W2[3] - W1[0] / W1[3];
				double Wy= W2[1] / W2[3] - W1[1] / W1[3];
				//通过moveEdge改变数据结构中的数据
				onPicked.moveEdge(Wx,Wy, 0);

				//同步视图
				vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(highLightActor->GetMapper());
				vtkPoints* pts = mapper->GetInput()->GetPoints();
				auto& p1 = onPicked.getPoint1();
				auto& p2 = onPicked.getPoint2();
				pts->SetPoint(0,p1[0], p1[1], p1[2]);
				pts->SetPoint(1, p2[0], p2[1], p2[2]);
				pts->Modified();
				interactor->GetRenderWindow()->Render();
				m_isdragging.store(false);

			}
		}
		
		
	}
protected:
	MyMouseCallback();
	~MyMouseCallback();
private:
	model* m_model=nullptr;
	vtkSmartPointer<vtkActor> highLightActor;
	std::atomic<bool> m_isdragging = false;//拾取标志位
	int m_position_x;
	int m_position_y;
	double m_pickdepth;
};
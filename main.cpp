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
#include <vtkCellPicker.h>
#include <vtkAutoInit.h>
#include "editor.h"
#include "MyMouseCallback.h"
#include <unordered_map>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

int main()
{
    double point1[3] = {-1.0, 0.0, 0.0};
    double point2[3]{1.0, 0.0, 0.0};
    double point3[3]{0.0, 1.0, 0.0};
    double point4[3]{0.0, -1.0, 0.0};
    double point5[3]{2.0, 3.0, 4.0};
    double point6[3]{3.0, 5.0, 2.0};


    
    editor m_editor;
    vtkSmartPointer<vtkActor> lineActor1;
    vtkSmartPointer<vtkActor> lineActor2;
    vtkSmartPointer<vtkActor> lineActor3;

    lineActor1 = m_editor.createEdgeActor(point1,point2);
    lineActor2 = m_editor.createEdgeActor(point3,point4);
    lineActor3 = m_editor.createEdgeActor(point5,point6);


    //  场景、窗口、鼠标键盘交互
    vtkNew<vtkRenderer> renderer;
    renderer->SetBackground(0.3, 0.1, 0.2);
    renderer->AddActor(lineActor1);
    renderer->AddActor(lineActor2);
    renderer->AddActor(lineActor3);

    vtkNew<vtkRenderWindow> window;
    window->SetWindowName("VTK Lesson 01 - Line");
    window->AddRenderer(renderer);
    window->SetSize(900, 600);

    vtkNew<vtkRenderWindowInteractor> interactor;
    vtkNew<MyMouseCallback> callback;
    interactor->SetRenderWindow(window);
    interactor->AddObserver(vtkCommand::LeftButtonPressEvent, callback);


    renderer->ResetCamera();
    window->Render();
    interactor->Start();

    return 0;
}
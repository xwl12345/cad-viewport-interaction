#include "StepViewer/ShapeToPolyData.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <vtkNew.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <BRep_Tool.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
void ShapeToPolyData::transferToVtk(const TopoDS_Shape& oneShape, vtkPoints* points, vtkPolyData* polyData)
{
	cellToFace.clear();
	faceToCell.clear();
	//三角化需要处理的OCCT实体
	BRepMesh_IncrementalMesh mesh(oneShape,0.1);

	//建立该OCCT实体的face到shape的索引表
	TopTools_IndexedMapOfShape faceMap;
	TopExp::MapShapes(oneShape, TopAbs_FACE, faceMap);
	faceToCell.resize(faceMap.Extent());
	//第一层循环是遍历oneShape里被三角化的每一个面，并且把他定义成一个确定的TopoDS_Face
	TopExp_Explorer explorer(oneShape, TopAbs_FACE);
	TopLoc_Location location;
	vtkNew<vtkCellArray> cells;
	int base;
	for (;explorer.More();explorer.Next())
	{
		//进入循环更新点集当前点数
		base = points->GetNumberOfPoints();
		TopoDS_Face face = TopoDS::Face(explorer.Current());
		//取当前面的三角网格
		const Handle(Poly_Triangulation)& current_triangulation = BRep_Tool::Triangulation(face, location);
		if (!current_triangulation.IsNull())
		{
			int faceId = faceMap.FindIndex(explorer.Current())-1;
			//第二层循环遍历点
			for (int i = 1;i <=current_triangulation->NbNodes();++i)
			{
				gp_Pnt p = current_triangulation->Node(i);
				//把当前点的位置从局部坐标转换为全局坐标
				p.Transform(location.Transformation());
				points->InsertNextPoint(p.X(), p.Y(), p.Z());
			}
			//第二层循环遍历三角形
			for (int j = 1;j <= current_triangulation->NbTriangles();++j)
			{
				Standard_Integer n1, n2, n3;
				//得到当前三角形的三个顶点索引
				current_triangulation->Triangle(j).Get(n1, n2, n3);
				vtkNew<vtkTriangle> Triangle;
				//创建vtk使用的三角形数据，并使用点索引创建三角形
				//n1指的是在这个面的点集上的索引，只有加上base才可以找到指定的点，-1是由于在occt中，从1开始计数，而vtk是从0开始，这里设置点的索引是在vtkPoints里的
				Triangle->GetPointIds()->SetId(0, base + n1 - 1);
				Triangle->GetPointIds()->SetId(1, base + n2 - 1);
				Triangle->GetPointIds()->SetId(2, base + n3 - 1);
				vtkIdType TriangleId=cells->InsertNextCell(Triangle);
				cellToFace.emplace_back(faceId);
				faceToCell[faceId].emplace_back(TriangleId);
			}
			
		}

	}
	int sum = 0;
	for (int i = 0;i < 70;++i)
	{
		sum += faceToCell[i].size();
	}
	//完成从occt实体到vtkPolyData的转化
	polyData->SetPoints(points);
	polyData->SetPolys(cells);
	std::cout << "当前共有"<<cellToFace.size()<<"个三角形" << std::endl;
	std::cout << "当前共有" << faceToCell.size() << "个面" << std::endl;
	std::cout << polyData->GetNumberOfCells()<<std::endl;
	std::cout << faceMap.Extent()<<std::endl;
	std::cout << sum;
}

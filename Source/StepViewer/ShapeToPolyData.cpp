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
#include <vtkPolyLine.h>
#include <vtkIdList.h>
#include <BRep_Tool.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include  <BRepAdaptor_Curve.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <vtkVertex.h>
void ShapeToPolyData::transferToVtk(const TopoDS_Shape& oneShape, vtkPoints* points, vtkPolyData* polyData, TopologyIndex& relationIndex)
{
	relationIndex.Clear();
	//三角化需要处理的OCCT实体
	BRepMesh_IncrementalMesh mesh(oneShape,0.1);


	//建立该OCCT实体的shape的索引表
	TopExp::MapShapes(oneShape, TopAbs_SOLID, relationIndex.solidMap);
	TopExp::MapShapes(oneShape, TopAbs_FACE, relationIndex.faceMap);
	relationIndex.faceToCell.resize(relationIndex.faceMap.Extent());
	TopExp::MapShapes(oneShape, TopAbs_EDGE, relationIndex.edgeMap);
	TopExp::MapShapes(oneShape, TopAbs_VERTEX, relationIndex.vertexMap);
	{
		//先创建一个保存face到solid的临时邻接表，后面用来保存face到solid父子关系
		TopTools_IndexedDataMapOfShapeListOfShape faceToSolidMap;
		TopExp::MapShapesAndAncestors(oneShape, TopAbs_FACE, TopAbs_SOLID, faceToSolidMap);

		relationIndex.faceToSolid.assign(relationIndex.faceMap.Extent(), -1);
		//遍历面
		for (int f = 1;f <= relationIndex.faceMap.Extent();++f)
		{
			//通过查找faceMap找到 编号与face 的对应关系
			const TopoDS_Shape& thisFace = relationIndex.faceMap.FindKey(f);

			//通过查找faceToSolidMap(保存了face到solid的父子关系找到 thisFace的父Solid列表 
			const TopTools_ListOfShape& solids = faceToSolidMap.FindFromKey(thisFace);

			//通过迭代器去遍历当前面对应的体集合
			for (TopTools_ListIteratorOfListOfShape it(solids);it.More();it.Next())
			{
				//通过查找solidMap得到当前体的索引
				int solidId = relationIndex.solidMap.FindIndex(it.Value()) - 1;//-1为了从1基到0基

				//得到由face id到solid id的映射，存储在relationIndex的vector<int>中，方便后面业务数据查找
				relationIndex.faceToSolid[f - 1] = solidId;
			}
		}
	}

	{
		TopTools_IndexedDataMapOfShapeListOfShape edgeToFaceMap;
		TopExp::MapShapesAndAncestors(oneShape, TopAbs_EDGE, TopAbs_FACE, edgeToFaceMap);
		
		relationIndex.edgeToFaces.assign(relationIndex.edgeMap.Extent(), { });

		for (int e = 1;e <= relationIndex.edgeMap.Extent();++e)
		{
			const TopoDS_Shape& thisEdge = relationIndex.edgeMap.FindKey(e);

			const TopTools_ListOfShape& face = edgeToFaceMap.FindFromKey(thisEdge);

			for (TopTools_ListIteratorOfListOfShape it(face);it.More();it.Next())
			{
				int faceId = relationIndex.faceMap.FindIndex(it.Value()) - 1;
				relationIndex.edgeToFaces[e - 1].emplace_back(faceId);
			}
		}
	}

	{
		TopTools_IndexedDataMapOfShapeListOfShape vertexToEdgeMap;
		TopExp::MapShapesAndAncestors(oneShape, TopAbs_VERTEX, TopAbs_EDGE, vertexToEdgeMap);

		relationIndex.vertexToEdges.assign(relationIndex.vertexMap.Extent(), { });

		for (int v = 1;v <= relationIndex.vertexMap.Extent();++v)
		{
			const TopoDS_Shape& thisVertex = relationIndex.vertexMap.FindKey(v);

			const TopTools_ListOfShape& edge = vertexToEdgeMap.FindFromKey(thisVertex);

			for (TopTools_ListIteratorOfListOfShape it(edge);it.More();it.Next())
			{
				int edgeId = relationIndex.edgeMap.FindIndex(it.Value()) - 1;
				relationIndex.vertexToEdges[v-1].emplace_back(edgeId);
			}
		}

	}
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
			int faceId = relationIndex.faceMap.FindIndex(explorer.Current())-1;
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
				relationIndex.cellToFace.emplace_back(faceId);
				relationIndex.faceToCell[faceId].emplace_back(TriangleId);
			}
			
		}

	}
	//完成从occt实体到vtkPolyData的转化
	polyData->SetPoints(points);
	polyData->SetPolys(cells);
	
}

void ShapeToPolyData::transferEdgeToVtk(vtkPoints* edgePoints, vtkPolyData* edgePolyData, TopologyIndex& relationIndex)
{
	vtkNew<vtkCellArray> edgeCells;
	int base;
	relationIndex.edgeToEdgeCell.resize(relationIndex.edgeMap.Extent());
	for (int e = 1;e <= relationIndex.edgeMap.Extent();++e)
	{
		base = edgePoints->GetNumberOfPoints();
		TopoDS_Shape now = relationIndex.edgeMap.FindKey(e);
		TopoDS_Edge edge=TopoDS::Edge(now);
		BRepAdaptor_Curve adaptor(edge);
		GCPnts_QuasiUniformDeflection uDeflection(adaptor, 0.01);
		if (uDeflection.IsDone())
		{
			int n= uDeflection.NbPoints();
			for (int k=1;k <= n;++k)
			{
				gp_Pnt p = uDeflection.Value(k);
				double x = p.X();
				double y = p.Y();
				double z = p.Z();
				edgePoints->InsertNextPoint(x, y, z);
			}
			vtkNew<vtkPolyLine> line;
			line->GetPointIds()->SetNumberOfIds(n);
			for (int l = 0;l < n;++l)
			{
				line->GetPointIds()->SetId(l, base + l);
			}
			vtkIdType cellId= edgeCells->InsertNextCell(line);
			relationIndex.edgeCellToEdge.push_back(e - 1);
			relationIndex.edgeToEdgeCell[e - 1].push_back(cellId);
		}
		else
		{
			continue;
		}

	}
	edgePolyData->SetPoints(edgePoints);
	edgePolyData->SetLines(edgeCells);
	
}

void ShapeToPolyData::transferVertexToVtk(vtkPoints* vertexPoints, vtkPolyData* vertexPolyData, TopologyIndex& relationIndex)
{
	vtkNew<vtkCellArray> vertexCells;
	relationIndex.vertexToVertexCell.resize(relationIndex.vertexMap.Extent());
	for (int v = 1;v <= relationIndex.vertexMap.Extent();++v)
	{
		TopoDS_Vertex vt = TopoDS::Vertex(relationIndex.vertexMap.FindKey(v));
		gp_Pnt p = BRep_Tool::Pnt(vt);
		p.Transform(vt.Location().Transformation());

		vtkIdType pid=vertexPoints->InsertNextPoint(p.X(), p.Y(), p.Z());

		vtkNew<vtkVertex> vertex;
		vertex->GetPointIds()->SetId(0, pid);
		vtkIdType vertexId=vertexCells->InsertNextCell(vertex);
		relationIndex.vertexCellToVertex.push_back(v - 1);
		relationIndex.vertexToVertexCell[v - 1].push_back(vertexId);
 	}
	vertexPolyData->SetPoints(vertexPoints);
	vertexPolyData->SetVerts(vertexCells);
}

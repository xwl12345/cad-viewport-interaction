#pragma once
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <vtkIdTypeArray.h>
#include <vector>
struct TopologyIndex
{
	TopTools_IndexedMapOfShape solidMap, faceMap, edgeMap, vertexMap;
	std::vector<int> faceToSolid;
	std::vector<std::vector<int>> edgeToFaces;
	std::vector<std::vector<int>> vertexToEdges;
	std::vector<int> cellToFace;
	std::vector<std::vector<vtkIdType>> faceToCell;
	std::vector<int> edgeCellToEdge;

	void Clear()   
	{
		solidMap.Clear(); faceMap.Clear(); edgeMap.Clear(); vertexMap.Clear();
		faceToSolid.clear(); edgeToFaces.clear(); vertexToEdges.clear();
		cellToFace.clear(); faceToCell.clear();edgeCellToEdge.clear();
	}
};
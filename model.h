#pragma once
#include <memory>
#include <vector>
#include "edge.h"
#include <unordered_map>
#include <vtkActor.h>
class model
{  
public:
	std::vector<edge> edges;
	void addEdge(const double p1[3], const double p2[3]);
	std::unordered_map<vtkActor*, edge> map;
};
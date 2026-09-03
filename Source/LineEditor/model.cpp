#include "LineEditor/model.h"

void model::addEdge(const double p1[3], const double p2[3])
{
	edges.emplace_back(p1, p2);
}

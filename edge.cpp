#include "edge.h"

edge::edge(const double p1[3], const double p2[3])
{
	for (int i = 0;i < 3;++i)
	{
		point1[i] = p1[i];
		point2[i] = p2[i];
	}
}

void edge::moveEdge()
{
	for (int i = 0;i < 3;++i)
	{
		point1[i] += delta;
		point2[i] += delta;
	}
}

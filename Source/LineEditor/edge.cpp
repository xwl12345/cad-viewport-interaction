#include "LineEditor/edge.h"

edge::edge(const double p1[3], const double p2[3])
{
	for (int i = 0;i < 3;++i)
	{
		point1[i] = p1[i];
		point2[i] = p2[i];
	}
}
void edge::moveEdge(double dx, double dy, double dz)
{
	double d[3] = {dx,dy,dz};
	for (int i = 0;i < 3;++i)
	{
		point1[i] += d[i];
		point2[i] += d[i];
	}
}

const std::array<double,3>& edge::getPoint1() const
{
	return point1;
}

const std::array<double, 3>&  edge::getPoint2() const
{
	return point2;
}


#pragma once
#include <vector>
#include <array>
class edge
{
public:
	edge(const double p1[3], const double p2[3]);
	void moveEdge(const double delta);
private:
	std::array<double,3> point1;
	std::array<double,3> point2;
};
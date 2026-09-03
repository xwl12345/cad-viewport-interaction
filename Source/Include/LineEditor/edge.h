#pragma once
#include <vector>
#include <array>
class edge
{
	
public:
	edge() = default;
	edge(const double p1[3], const double p2[3]);
	void moveEdge(double dx,double dy,double dz);

	const std::array<double, 3>&  getPoint1() const;
	const std::array<double, 3>&  getPoint2() const;
private:
	std::array<double,3> point1;
	std::array<double,3> point2;
};
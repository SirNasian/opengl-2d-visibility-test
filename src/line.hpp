#pragma once

#include <glm/glm.hpp>

class Line
{
	public:
		glm::vec2 origin, normal;
		Line(glm::vec2 origin, glm::vec2 normal);
};

class LineSegment
{
	public:
		glm::vec2 p1, p2;
		LineSegment(glm::vec2 p1, glm::vec2 p2);
		glm::vec2 direction();
		glm::vec2 normal();
		float length();
		bool intersect(Line line, glm::vec2 &intersect);
		bool intersect(LineSegment segment, glm::vec2 &intersect);
};


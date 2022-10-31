#include "line.hpp"

#include <glm/glm.hpp>

Line::Line(glm::vec2 origin, glm::vec2 normal)
{
	this->origin = origin; this->normal = normal;
}

LineSegment::LineSegment(glm::vec2 p1, glm::vec2 p2)
{
	this->p1 = p1; this->p2 = p2;
}

glm::vec2 LineSegment::direction()
{
	return glm::normalize(this->p2 - this->p1);
};

glm::vec2 LineSegment::normal()
{
	glm::vec3 normal = glm::cross(glm::vec3(this->p2 - this->p1, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	return glm::vec2(normal.x, normal.y);
};

float LineSegment::length()
{
	return glm::length(this->p1 - this->p2);
};

bool LineSegment::intersect(Line line, glm::vec2 &intersect)
{
	if (glm::dot(line.normal, this->direction()) == 0.0f) return false;
	float d = glm::dot(line.normal, line.origin);
	float x = (d - glm::dot(line.normal, this->p1)) / glm::dot(line.normal, this->direction());
	intersect = this->p1 + (this->direction() * x);
	return (x >= 0.0f) && (x <= this->length());
}

bool LineSegment::intersect(LineSegment segment, glm::vec2 &intersect)
{
	return (
		this->intersect(Line(segment.p1, segment.normal()), intersect) &&
		segment.intersect(Line(this->p1, this->normal()), intersect)
	);
}

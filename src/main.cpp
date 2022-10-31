#include <iostream>

#include <glm/glm.hpp>

class Line
{
	public:
		glm::vec2 origin, normal;
		Line(glm::vec2 origin, glm::vec2 normal) { this->origin = origin; this->normal = normal; };
};

class LineSegment
{
	public:
		glm::vec2 p1, p2;
		LineSegment(glm::vec2 p1, glm::vec2 p2) { this->p1 = p1; this->p2 = p2; };
		glm::vec2 direction() { return glm::normalize(this->p2 - this->p1); };
		glm::vec2 normal()
		{
			glm::vec3 normal = glm::cross(glm::vec3(this->p2 - this->p1, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			return glm::vec2(normal.x, normal.y);
		};
		float length() { return glm::length(this->p1 - this->p2); };
		bool intersect(Line line, glm::vec2 &intersect)
		{
			if (glm::dot(line.normal, this->direction()) == 0.0f) return false;
			float d = glm::dot(line.normal, line.origin);
			float x = (d - glm::dot(line.normal, this->p1)) / glm::dot(line.normal, this->direction());
			intersect = this->p1 + (this->direction() * x);
			return (this->length() > glm::abs(x));
		}
		bool intersect(LineSegment segment, glm::vec2 &intersect)
		{
			return (
				this->intersect(Line(segment.p1, segment.normal()), intersect) &&
				segment.intersect(Line(this->p1, this->normal()), intersect)
			);
		}
};

int main()
{
	glm::vec2 i(0.0f, 0.0f);
	LineSegment segments[] = {
		LineSegment(glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 2.0f)),
		LineSegment(glm::vec2(0.0f, 2.0f), glm::vec2(2.0f, 0.0f)),
		LineSegment(glm::vec2(0.5f, 2.0f), glm::vec2(0.5f, 0.0f)),
	};

	for (int ii = 0; ii < sizeof(segments)/sizeof(segments[0]); ii++)
		for (int jj = ii+1; jj < sizeof(segments)/sizeof(segments[0]); jj++)
			if (segments[ii].intersect(segments[jj], i))
				printf("intersect at (%f, %f); ii %d; jj %d;\n", i.x, i.y, ii, jj);

	return 0;
}

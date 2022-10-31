#include <iostream>

#include <glm/glm.hpp>

#include "line.hpp"

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

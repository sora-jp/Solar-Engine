#pragma once

#include "glm/glm.hpp"

struct Bounds
{
	glm::vec3 min, max;

	glm::vec3 Center() const { return (min + max) / 2.0f;}
	glm::vec3 Size() const { return max - min; }
	glm::vec3 Extents() const { return Size() / 2.0f; }
};
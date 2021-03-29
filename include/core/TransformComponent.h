#pragma once
#include "Common.h"
//#include "glm/vec3.hpp"
//#include "glm/ext/quaternion_float.hpp"

struct SOLAR_API TransformComponent
{
	float nempty;
	//glm::vec3 position;
	//glm::quat rotation;
	//glm::vec3 scale;

	//glm::mat4x4 GetTransformMatrix() const { return glm::translate(static_cast<glm::mat4x4>(rotation) * glm::scale(glm::identity<glm::mat4x4>(), scale), position); }
};
#pragma once
#include "Common.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

struct SOLAR_API TransformComponent
{
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::quat rotation = glm::quat(glm::vec3(0, 0, 0));
	glm::vec3 scale = glm::vec3(1, 1, 1);

	[[nodiscard]] glm::mat4x4 GetTransformMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), position)
		     * static_cast<glm::mat4>(rotation)
		     * glm::scale(glm::mat4(1.0f), scale);
	}
};
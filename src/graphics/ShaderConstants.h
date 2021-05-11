#pragma once
#include "glm/glm.hpp"

struct ShaderConstants
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 viewProj;
	glm::vec3 worldSpaceCamPos;
};
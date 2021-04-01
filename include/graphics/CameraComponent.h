#pragma once
#include "glm/glm.hpp"

struct CameraComponent
{
	float fov;
	float nearClip, farClip;
	float aspect;

	glm::mat4 GetCameraMatrix() const
	{
		return glm::perspectiveLH_ZO(fov, aspect, nearClip, farClip);
	}
};
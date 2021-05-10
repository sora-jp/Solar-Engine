#pragma once
#include "glm/glm.hpp"

struct Plane
{
	float distance;
	glm::vec3 normal;

	Plane() : distance(0), normal(0) {}
	Plane(const glm::vec3 nrm, const float d) : distance(d), normal(nrm) {}
	Plane(const glm::vec4 data) : distance(-data.w), normal(data) {}
	Plane(const float a, const float b, const float c, const float d) noexcept : distance(-d), normal(a, b, c) {}

	[[nodiscard]] Plane Transform(const glm::mat4& mat) const
	{
		auto np = glm::vec4(normal * distance, 1) * mat;
		np /= np.w;

		const auto nn = glm::vec4(normal, 0) * mat;

		return { nn, glm::dot(np, nn) };
	}
};
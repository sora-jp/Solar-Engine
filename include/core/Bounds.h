#pragma once

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <vector>

struct Bounds
{
	glm::vec3 min, max;

	[[nodiscard]] glm::vec3 Center() const { return (min + max) / 2.0f;}
	[[nodiscard]] glm::vec3 Size() const { return max - min; }
	[[nodiscard]] glm::vec3 Extents() const { return Size() / 2.0f; }

	void GetCorners(std::vector<glm::vec3>& outCorners) const
	{
		outCorners.emplace_back(min.x, min.y, min.z);
		outCorners.emplace_back(max.x, min.y, min.z);
		outCorners.emplace_back(min.x, max.y, min.z);
		outCorners.emplace_back(max.x, max.y, min.z);
		outCorners.emplace_back(min.x, min.y, max.z);
		outCorners.emplace_back(max.x, min.y, max.z);
		outCorners.emplace_back(min.x, max.y, max.z);
		outCorners.emplace_back(max.x, max.y, max.z);
	}

	[[nodiscard]] glm::vec3 GetCorner(const glm::bvec3& minMask) const
	{
		return glm::mix(max, min, minMask);
	}

	[[nodiscard]] Bounds Transform(const glm::mat4& matrix) const
	{
		auto corners = std::vector<glm::vec3>(8);
		GetCorners(corners);

		auto newMin = glm::vec3(INFINITY);
		auto newMax = glm::vec3(-INFINITY);
		
		for (auto c : corners)
		{
			auto tc = glm::vec3(matrix * glm::vec4(c, 1));
			newMin = glm::min(newMin, tc);
			newMax = glm::max(newMax, tc);
		}

		return { newMin, newMax };
	}

	[[nodiscard]] bool InsideFrustum(glm::mat4 projMat) const
	{
		auto corners = std::vector<glm::vec3>(8);
		GetCorners(corners);

		return std::any_of(corners.cbegin(), corners.cend(), [&](const glm::vec3 c)
		{
			auto res = glm::vec4(c, 1) * projMat;
			res /= res.w;
			return (res.x > -1 && res.x < 1) && (res.y > -1 && res.y < 1) && (res.z > -1 && res.z < 1);
		});
	}
};
#pragma once
//#include "Cubemap.h"
#include "Texture.h"
#include "editor/EditorGUI.h"
#include "core/Bounds.h"
#include "core/Plane.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <algorithm>

struct Frustum
{
	Plane planes[6]; // front, back, left, right, up, down

	static Frustum FromVPMatrix(const glm::mat4& vp)
	{
		Frustum f;
		f.planes[0] = glm::row(vp, 3) + glm::row(vp, 2);
		f.planes[1] = glm::row(vp, 3) - glm::row(vp, 2);

		f.planes[2] = glm::row(vp, 3) + glm::row(vp, 0);
		f.planes[3] = glm::row(vp, 3) - glm::row(vp, 0);

		f.planes[4] = glm::row(vp, 3) + glm::row(vp, 1);
		f.planes[5] = glm::row(vp, 3) - glm::row(vp, 1);
		return f;
	}

	[[nodiscard]] bool TestAABB(const Bounds& bounds) const
	{
		return std::all_of(std::cbegin(planes), std::cend(planes), [&](const Plane& p) { return InsideOrIntersecting(bounds, p); });
	}

private:
	[[nodiscard]] bool InsideOrIntersecting(const Bounds& bounds, const Plane& plane) const
	{
		return glm::dot(plane.normal, GetVertex(bounds, plane, false)) > plane.distance;
	}
	
	[[nodiscard]] glm::vec3 GetVertex(const Bounds& bounds, const Plane& plane, const bool nVert) const
	{
		return bounds.GetCorner(glm::greaterThan(plane.normal, glm::vec3(0)) ^ nVert);
	}
};

struct CameraComponent
{
	Range fov { 0, 90 };
	float nearClip, farClip;
	float aspect;
	Shared<TextureCube> skybox;
	Shared<TextureCube> indirectIBL;
	Shared<RenderTexture> target;

	[[nodiscard]] glm::mat4 GetCameraMatrix() const
	{
		const auto actualAspect = target != nullptr ? (static_cast<float>(target->Width()) / target->Height()) : aspect;
		return glm::perspectiveLH_ZO(glm::radians<float>(fov), actualAspect, nearClip, farClip);
	}

	void OnInspectorGUI()
	{
		EditorGUI::Field(fov, "Field of View");
		EditorGUI::Field(nearClip, "Near clip plane");
		EditorGUI::Field(farClip, "Far clip plane");
	}
};
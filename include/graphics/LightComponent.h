#pragma once

enum class LightType
{
	Directional, Point
};

struct LightComponent
{
	LightType type;
	float strength;
};
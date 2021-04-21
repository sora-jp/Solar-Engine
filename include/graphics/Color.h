#pragma once

#include "glm/vec4.hpp"
#include <limits>

struct Color : glm::vec4
{
	Color(const glm::vec4& value) : vec(value) {}
	Color(const float& r, const float& g, const float& b, const float& a = 1) : vec(r, g, b, a) {}
	Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = std::numeric_limits<uint8_t>::max()) : vec(glm::vec4(r, g, b, a) / 255.f) {}

	operator float* () { return &x; }
	operator const float* () const { return &x; }

	float& operator[](int idx) { return (&x)[idx]; }
	const float& operator[](int idx) const { return (&x)[idx]; }
};
#pragma once
#include "core/Common.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <string>

#include "graphics/Material.h"

class EditorGUI
{
	static bool Field(float* value, int components, const std::string& label = "");

public:
	static void BeginField(const std::string& label);
	static void EndField();
	static void Label(const std::string& label);
	static void Separator();
	
	template <glm::length_t L, glm::qualifier Q>
	static bool Field(glm::vec<L, float, Q>& value, const std::string& label = "");
	static bool Field(glm::quat& value, const std::string& label = "");
	static bool Field(float& value, const std::string& label = "");
	static bool Field(Range& value, const std::string& label = "");
	static bool Field(std::string& value, const std::string& label = "");

	static void InlineEditor(Shared<Material>& material, const std::string& label = "");
};

template <glm::length_t L, glm::qualifier Q>
bool EditorGUI::Field(glm::vec<L, float, Q>& value, const std::string& label)
{
	static_assert(L > 0 && L <= 4);
	static_assert(!std::is_const_v<decltype(value)>);

	return Field(glm::value_ptr(value), L, label);
}

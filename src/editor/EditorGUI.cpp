#include "pch.h"
#include "EditorGUI.h"
#include "graphics/Color.h"

#include "imgui.h"
#include "imgui_internal.h"

static constexpr const char* const FLOAT_FORMAT = "%.1f";

template<typename T, class = void> struct is_glm_vec_t : std::false_type {};
template<typename T> struct is_glm_vec_t<T, std::void_t<typename T::value_type, typename std::is_same<T, typename T::type>::type>> : std::true_type {};

template<typename T>
static constexpr bool is_glm_vec_v = is_glm_vec_t<T>::value;


template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
struct ptrvec
{
	ptrvec(T* data, const int sz) noexcept : data(data), sz(sz) {}
	ptrvec(ptrvec&& other) noexcept : data(std::exchange(other.data, nullptr)), sz(std::exchange(other.sz, 0)) {}
	
	T* data;
	int sz;

	typedef T pt_valty;
};

template<typename T, class = void> struct is_ptrvec_t : std::false_type {};
template<typename T> struct is_ptrvec_t<T, std::void_t<std::is_same<ptrvec<typename T::pt_valty>, T>>> : std::true_type {};

template<typename T>
static constexpr bool is_ptrvec_v = is_ptrvec_t<T>::value;

template<typename T>
constexpr ImGuiDataType GetImGuiDataType()
{
	if (std::is_same_v<T, bool>) return ImGuiDataType_COUNT;
	
	if constexpr (std::is_floating_point_v<T>)
		return std::is_same_v<T, float> ? ImGuiDataType_Float : ImGuiDataType_Double;
	else if constexpr(std::is_integral_v<T>)
	{
		constexpr ImGuiDataType sintLut[] = { ImGuiDataType_S8, ImGuiDataType_S16, ImGuiDataType_S32, ImGuiDataType_S64 };
		constexpr ImGuiDataType uintLut[] = { ImGuiDataType_U8, ImGuiDataType_U16, ImGuiDataType_U32, ImGuiDataType_U64 };

		constexpr auto sz = sizeof(T);
		constexpr auto idx = 
			sz == 1 ? 0 :
			sz == 2 ? 1 :
			sz == 4 ? 2 :
			sz == 8 ? 3 : -1;

		if constexpr (std::is_signed_v<T>) return sintLut[idx];
		else if constexpr (std::is_unsigned_v<T>) return uintLut[idx];
	}

	return ImGuiDataType_COUNT;
}

bool ColorEditor(const std::string& id, float* ptr)
{
	const auto open = ImGui::ColorButton(id.c_str(), *reinterpret_cast<ImVec4*>(ptr), ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoBorder, ImVec2(ImGui::CalcItemWidth(), 0));

	const auto pid = id + "_PopupEditor";
	if (open) ImGui::OpenPopup(pid.c_str());

	if (ImGui::BeginPopup(pid.c_str()))
	{
		ImGui::SetWindowSize(ImVec2(200, 200), ImGuiCond_Appearing);
		const auto cid = id + "_ColorEdit4";
		const auto changed = ImGui::ColorPicker4(cid.c_str(), ptr, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar);

		ImGui::EndPopup();

		return changed;
	}

	return false;
}

template<typename T>
bool Input(T& value, const std::string& id)
{
	auto changed = false;

	if constexpr (std::is_same_v<T, std::string>)	changed = ImGui::InputText(id.c_str(), &value);
	else if constexpr (std::is_same_v<T, bool>)		changed = ImGui::Checkbox(id.c_str(), &value);
	else if constexpr (std::is_arithmetic_v<T>)		changed = ImGui::InputScalar(id.c_str(), GetImGuiDataType<T>(), &value);
	else if constexpr (std::is_same_v<T, Range>)	changed = ImGui::SliderFloat(id.c_str(), &value, value.min, value.max);
	else if constexpr (std::is_same_v<T, Color>)	changed = ColorEditor(id, &value.x);
	else if constexpr (is_glm_vec_v<T>)				changed = ImGui::InputScalarN(id.c_str(), GetImGuiDataType<typename T::value_type>(), glm::value_ptr(value), T::length());
	else if constexpr (is_ptrvec_v<T>)				changed = ImGui::InputScalarN(id.c_str(), GetImGuiDataType<typename T::pt_valty>(), value.data, value.sz);
	
	return changed;
}

template<typename T>
bool RawField(T& value, const std::string& label)
{
	EditorGUI::Label(label);
	ImGui::SameLine();

	return Input<T>(value, "##" + label);
}

template<typename T>
bool Field(T& value, const std::string& label)
{
	EditorGUI::BeginField(label);
	auto changed = Input<T>(value, "##" + label);
	EditorGUI::EndField();

	return changed;
}

bool EditorGUI::Field(glm::quat& value, const std::string& label)
{
	auto euler = glm::degrees(glm::eulerAngles(value));
	if (::Field(euler, label)) 
	{
		value = glm::quat(glm::radians(euler));
		return true;
	}

	return false;
}

bool EditorGUI::Field(float& value, const std::string& label) { return ::Field(value, label); }
bool EditorGUI::Field(Range& value, const std::string& label) { return ::Field(value, label); }
bool EditorGUI::Field(std::string& value, const std::string& label) { return ::Field(value, label); }

bool EditorGUI::Field(float* value, const int components, const std::string& label)
{
	auto tmp = ptrvec<float>{ value, components };
	return ::Field(tmp, label);
}

template<typename T>
bool MpbVecField(MaterialPropertyBlock& mpb, const std::string& prop, int count, const std::string& label)
{
	auto tmp = ptrvec<T> { mpb.Property<T>(prop), count };
	return ::Field(tmp, label);
}

bool MpbColField(MaterialPropertyBlock& mpb, const std::string& prop, const std::string& label)
{
	const auto tmp = mpb.Property<Color>(prop);
	return ::Field(*tmp, label);
}

void EditorGUI::InlineEditor(Shared<Material>& material, const std::string& label)
{
	if (!material) return;
	BeginField(label);
	ImGui::NewLine();
	ImGui::Indent(20);
	
	auto& mpb = material->GetProperties();
	auto modified = false;
	
	mpb.Iterate([&](CBufferVariable& prop)
	{
		const auto cleanName = prop.name.substr(prop.name.find_first_of('_') + 1);
		
		switch (prop.baseType)
		{
		case CBufferBasicType::Int:
		{
			modified |= MpbVecField<int>(mpb, prop.name, prop.componentCount, cleanName);
			break;
		}
		case CBufferBasicType::UInt:
		{
			modified |= MpbVecField<unsigned int>(mpb, prop.name, prop.componentCount, cleanName);
			break;
		}
		case CBufferBasicType::Float: 
		{
			if (prop.componentCount == 4) modified |= MpbColField(mpb, prop.name, cleanName);
			else modified |= MpbVecField<float>(mpb, prop.name, prop.componentCount, cleanName);
			break;
		}
		case CBufferBasicType::Bool: 
		{
			modified |= MpbVecField<bool>(mpb, prop.name, prop.componentCount, cleanName);
			break;
		}
		default: break;
		}
	});

	if (modified) mpb.Flush();
	EndField();
}

void EditorGUI::BeginField(const std::string& label)
{
	ImGui::PushID(label.c_str());
	ImGui::BeginGroup();
	
	Label(label);
	
	ImGui::SameLine(160);
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
}

void EditorGUI::EndField()
{
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::PopID();
}

void EditorGUI::Label(const std::string& label)
{
	if (!label.empty())
	{
		ImGui::Text("%s", label.c_str());
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - ImGui::CalcTextSize(label.c_str()).x - ImGui::GetStyle().ItemSpacing.x);
	}
}

void EditorGUI::Separator()
{
	ImGui::Separator();
}

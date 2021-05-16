#include "CameraComponent.h"
#include "Log.h"
#include "EntryPoint.h"
#include "Scene.h"
#include "Entity.h"
#include "System.h"
#include "dear-imgui/imgui.h"
#include "TransformComponent.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "RendererComponent.h"
#include "Cubemap.h"
#include <filesystem>

#include "GraphicsSubsystem.h"
#include "Shader.h"
#include "Input.h"
#include "InspectorWindow.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "SceneGraphWindow.h"
#include "SceneViewWindow.h"

class TestApp final : public SolarApp
{
	SOLAR_APPNAME("Test App")
	
public:
	void Init() override;
	void Run() override;
	void Shutdown() override;
	void UseSubsystems() override;
};

struct TestComponent
{
	glm::vec2 angle;
};

static float _time;

float KBGetAxis(const Key pos, const Key neg)
{
	return Keyboard::Current().KeyHeld(pos) * 1.f + Keyboard::Current().KeyHeld(neg) * -1.f;
}

class TestSystem final : public System<CameraComponent, TransformComponent, TestComponent>
{
public:
	void Execute(const Entity e, CameraComponent& c, TransformComponent& t, TestComponent& tc) override
	{
		const auto& mouse = Input::First<Mouse>();
		
		if (mouse.ButtonDown(MouseButton::Right)) mouse.SetCursorEnabled(false);
		else if (mouse.ButtonUp(MouseButton::Right)) mouse.SetCursorEnabled(true);
		else if (mouse.ButtonHeld(MouseButton::Right))
		{
			const auto delta = Input::First<Mouse>().GetDelta();
			tc.angle += delta / 35.f;

			tc.angle.y = glm::clamp(tc.angle.y, -90.f, 90.f);
			t.rotation = glm::quat(glm::radians(glm::vec3(tc.angle.y, tc.angle.x, 0)));

			const auto fwd = t.rotation * glm::vec3(0, 0, 1);
			const auto up = t.rotation * glm::vec3(0, 1, 0);
			const auto right = t.rotation * glm::vec3(1, 0, 0);

			auto dpos = glm::vec3(0);
			dpos += fwd * KBGetAxis(Key::W, Key::S);
			dpos += up * KBGetAxis(Key::E, Key::Q);
			dpos += right * KBGetAxis(Key::D, Key::A);
			t.position += dpos * 4.f / 75.f;
		}
	}
};

static Shared<Shader> m_shader;
static Shared<Mesh> m_mesh;
static Shared<Cubemap> m_envMap, m_diffuseIbl;

void TestApp::Init()
{
	SOLAR_INFO("TestApp::Init()");
	Engine::UseSystem<TestSystem>();

	m_shader = ShaderCompiler::Compile("DefaultDeferred.hlsl", "vert", "frag", [](GraphicsPipelineDesc& desc)
	{
		//desc.RasterizerDesc.CullMode = CULL_MODE_NONE;
	});
	// C:\Users\oskar.tornevall\Documents\Projects\Github\Solar-Engine\src\test
	const std::filesystem::path path = R"(E:\Solar Engine\src\test)";
	//const std::filesystem::path path = R"(C:\Users\oskar.tornevall\Documents\Projects\Github\Solar-Engine\src\test)";
	
	m_envMap = Cubemap::Load((path / "HdrOutdoorCityPathDayClear001_JPG_4K.jpg").string());
	m_diffuseIbl = Cubemap::ConvolveDiffuse(m_envMap);
	
	m_mesh = Mesh::Load((path / "sponza" / "Sponza.gltf").string());
	
	auto scene = Scene::Create();

	auto e = scene->CreateEntity("Render test", Entity::null);
	e.GetComponent<TransformComponent>().scale = glm::vec3(4.f);
	
	auto& r = e.AddComponent<RendererComponent>();
	r.mesh = m_mesh;

	for (auto& m : m_mesh->GetMaterials())
	{
		auto mat = Material::Create(m_shader);

		auto diff = glm::vec4(m.diffuse, 1);
		auto emiss = glm::vec4(m.emissive, 1);
		//auto smxx = glm::vec2(m.roughness, m.metallicity);
		auto smxx = glm::vec2(0.85f, 0);

		glm::vec3 tp(0);
		if (m.diffuseTex != nullptr) {
			tp.x = 1;
			mat->GetProperties().SetTexture("_MainTex", *m.diffuseTex);
		}
		
		if (m.normalTex != nullptr) {
			tp.y = 1;
			mat->GetProperties().SetTexture("_NormalTex", *m.normalTex);
		}

		if (m.metalRoughTex != nullptr) {
			tp.z = 1;
			mat->GetProperties().SetTexture("_MRTex", *m.metalRoughTex);
		}
		
		mat->GetProperties().Set("_Tint", &diff);
		mat->GetProperties().Set("_Emission", &emiss);
		mat->GetProperties().Set("_SmoothnessMetal", &smxx);
		mat->GetProperties().Set("_TexturesPresent", &tp);

		r.materials.push_back(mat);
	}

	auto ec = scene->CreateEntity("Camera", Entity::null);
	auto& c = ec.AddComponent<CameraComponent>();
	c.fov = 60;
	c.nearClip = 0.3f;
	c.farClip = 500.0f;
	c.aspect = 1280.0f / 720;
	c.skybox = m_envMap;
	c.indirectIBL = m_diffuseIbl;

	auto& t = ec.GetComponent<TransformComponent>();
	t.position = glm::vec3(0, 3.5f, -10);
	t.rotation = glm::quat(glm::vec3(glm::radians(15.f), 0, 0));

	ec.AddComponent<TestComponent>();
	
	EditorWindow::Open<SceneGraphWindow>();
	EditorWindow::Open<InspectorWindow>();
	EditorWindow::Open<SceneViewWindow>(c, t);
}

void TestApp::Run()
{
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	_time += 1.0f / 60;

	EditorWindow::UpdateAll();
}

void TestApp::Shutdown()
{
	SOLAR_INFO("TestApp::Shutdown()");
}

void TestApp::UseSubsystems()
{
	Engine::UseSubsystem<GraphicsSubsystem>();
}

REGISTER_APPLICATION(TestApp)

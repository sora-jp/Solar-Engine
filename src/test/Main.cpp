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
#include "InspectorWindow.h"
#include "SceneGraphWindow.h"

class TestApp final : public SolarApp
{
	SOLAR_APPNAME("Test App")
	
public:
	void Init() override;
	void Run() override;
	void Shutdown() override;
	void UseSubsystems() override;
};

static float _time;
class TestSystem final : public System<CameraComponent, TransformComponent>
{
public:
	void Execute(const Entity e, CameraComponent& c, TransformComponent& t) override
	{
		const auto y = _time * 3.5f / (2 * glm::pi<float>());
		t.rotation = glm::quat(glm::vec3(glm::radians(15.f), y, 0));
		t.position = glm::vec3(0, 2.f, -7) * glm::inverse(glm::quat(glm::vec3(0, y, 0)));
	}
};

static Shared<Shader> m_shader;
static Shared<Mesh> m_mesh;
static Shared<Cubemap> m_diffuseIBL;

void TestApp::Init()
{
	SOLAR_INFO("TestApp::Init()");
	Engine::UseSystem<TestSystem>();

	m_shader = ShaderCompiler::Compile("DefaultDeferred.hlsl", "vert", "frag", [](GraphicsPipelineDesc& desc)
	{
		desc.RasterizerDesc.CullMode = CULL_MODE_NONE;
	});
	// C:\Users\oskar.tornevall\Documents\Projects\Github\Solar-Engine\src\test
	//const std::filesystem::path path = R"(D:\Projects\Solar Engine\src\test)";
	const std::filesystem::path path = R"(C:\Users\oskar.tornevall\Documents\Projects\Github\Solar-Engine\src\test)";
	
	m_diffuseIBL = Cubemap::Load((path / "HdrOutdoorCityPathDayClear001_JPG_4K_DIFFUSE.png").string());
	m_mesh = Mesh::Load((path / "ferarri" / "millenio.glb").string());
	
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
		auto smxx = glm::vec2(m.roughness, m.metallicity);
		
		mat->GetProperties().Set("_Tint", &diff);
		mat->GetProperties().Set("_Emission", &emiss);
		mat->GetProperties().Set("_SmoothnessMetal", &smxx);

		r.materials.push_back(mat);
	}

	auto ec = scene->CreateEntity("Camera", Entity::null);
	auto& c = ec.AddComponent<CameraComponent>();
	c.fov = 60;
	c.nearClip = 0.3f;
	c.farClip = 1000.0f;
	c.aspect = 1280.0f / 720;
	c.skybox = m_diffuseIBL;

	auto& t = ec.GetComponent<TransformComponent>();
	t.position = glm::vec3(0, 3.5f, -10);
	t.rotation = glm::quat(glm::vec3(glm::radians(15.f), 0, 0));
	
	EditorWindow::Open<SceneGraphWindow>();
	EditorWindow::Open<InspectorWindow>();
}

void TestApp::Run()
{
	static auto demoOpen = true;
	ImGui::ShowDemoWindow(&demoOpen);
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

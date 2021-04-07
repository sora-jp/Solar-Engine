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

class TestApp final : public SolarApp
{
	SOLAR_APPNAME("Test App")
	
public:
	void Init() override;
	void Run() override;
	void Shutdown() override;
};

static float _time;
class TestSystem final : public System<TransformComponent>
{
public:
	void Execute(const Entity e, TransformComponent& c) override
	{
		//SOLAR_INFO("{:E} -> {:E}", 
		//	e, 
		//	c.GetParent()
		//);
		if (e.HasComponent<CameraComponent>()) return;
		//SOLAR_INFO("{} {} {} {}", c.rotation.x, c.rotation.y, c.rotation.z, c.rotation.w);

	}
};

static Shared<Shader> m_shader;
static Shared<Material> m_mat;
static Shared<Mesh> m_mesh, m_mesh2;
static Entity trans(entt::null, nullptr);

void TestApp::Init()
{
	SOLAR_INFO("TestApp::Init()");

	m_shader = ShaderCompiler::Compile("DefaultDeferred.hlsl", "vert", "frag");
	m_mat = Material::Create(m_shader);

	auto cc = glm::vec4(114, 206, 224, 255) / 255.f;
	m_mat->GetProperties().Set("_Tint", &cc);
	m_mat->GetProperties().Set("_SMXX", &cc);

	m_mesh = Mesh::Load("D:\\Projects\\Solar Engine\\src\\test\\nissan\\nissan.obj");
	m_mesh2 = Mesh::Load("D:\\Projects\\Solar Engine\\src\\test\\plane2.fbx");
	
	auto scene = Scene::Create();

	auto e = trans = scene->CreateEntity("Render test", Entity::null);
	auto& r = e.AddComponent<RendererComponent>();
	r.material = m_mat;
	r.mesh = m_mesh;
	trans.GetComponent<TransformComponent>().scale = glm::vec3(4.f);
	
	e = scene->CreateEntity("Render test 2", Entity::null);
	auto& r2 = e.AddComponent<RendererComponent>();
	r2.material = m_mat;
	r2.mesh = m_mesh2;
	e.GetComponent<TransformComponent>().position = glm::vec3(0.f, 0.f, 0.f);

	auto ec = scene->CreateEntity("Camera", Entity::null);
	auto& c = ec.AddComponent<CameraComponent>();
	c.fov = 60;
	c.nearClip = 0.3f;
	c.farClip = 1000.0f;
	c.aspect = 1280.0f / 720;

	auto& t = ec.GetComponent<TransformComponent>();
	t.position = glm::vec3(0, 3.5f, -10);
	t.rotation = glm::quat(glm::vec3(glm::radians(15.f), 0, 0));
	//t.scale = glm::vec3(1, 1, 1);
	
	//for (auto i = 0; i < 10; i++) {
	//	auto e1 = scene->CreateEntity(fmt::format("Parent ({:03})", i), Entity::null);
	//	auto e2 = scene->CreateEntity(fmt::format("Child  ({:03})", i), e1);
	//	//e2.AddComponent<TransformComponent>();
	//}
}

void TestApp::Run()
{
	//SOLAR_INFO("TestApp::Run()");
	trans.GetComponent<TransformComponent>().rotation = glm::quat(glm::vec3(0, _time * 2, 0));
	trans.GetComponent<TransformComponent>().position = (glm::vec3(-1.481f, -0.5058f, -0.734f) * 4.f) * glm::inverse(trans.GetComponent<TransformComponent>().rotation);
	
	static auto demoOpen = true;
	ImGui::ShowDemoWindow(&demoOpen);
	_time += 1.0f / 60;
}

void TestApp::Shutdown()
{
	SOLAR_INFO("TestApp::Shutdown()");
}

REGISTER_SYSTEM(TestSystem)
REGISTER_APPLICATION(TestApp)
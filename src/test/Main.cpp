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
		c.rotation = glm::quat(glm::vec3(0, _time * 2, 0));
	}
};

const char* shaderSrc = 
R"(cbuffer Constants
{
    float4x4 g_Model;
    float4x4 g_ViewProj;
};

// Vertex shader takes two inputs: vertex position and uv coordinates.
// By convention, Diligent Engine expects vertex shader inputs to be 
// labeled 'ATTRIBn', where n is the attribute number.
struct VSInput
{
    float3 Pos : ATTRIB0;
    float3 Nrm : ATTRIB1;
    float2 UV  : ATTRIB2;
};

struct PSInput 
{ 
    float4 Pos : SV_POSITION; 
    float3 Nrm : TEX_COORD0;
    float2 UV  : TEX_COORD1; 
    float3 WorldPos : TEX_COORD2; 
};

struct PSOutput
{
	float4 Color : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 Position : SV_TARGET2;
	float4 SpecMetal : SV_TARGET3;
};

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be indentical.
void vert(in  VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos = mul( float4(VSIn.Pos,1.0), mul(g_Model, g_ViewProj));
	PSIn.WorldPos = mul(float4(VSIn.Pos, 1.0), g_Model).xyz;
	PSIn.Nrm = normalize(mul(float4(VSIn.Nrm, 0), g_Model).xyz);
    PSIn.UV  = VSIn.UV;
}

void frag(in  PSInput  PSIn,
	out PSOutput PSOut)
{
	PSOut.Color = 1;//lerp(0.015, 1, saturate(dot(PSIn.Nrm, normalize(float3(1, 1, -1)))));//_MainTex.Sample(_MainTex_sampler, PSIn.UV);
	PSOut.Normal = float4(normalize(PSIn.Nrm), 1);
	PSOut.Position = float4(PSIn.WorldPos, 1);
	PSOut.SpecMetal = float4(0, 0, 0, 0);
})";

static Shared<Shader> m_shader;
static Shared<Material> m_mat;
static Shared<Mesh> m_mesh, m_mesh2;

void TestApp::Init()
{
	SOLAR_INFO("TestApp::Init()");

	m_shader = ShaderCompiler::Compile("Test Shader", shaderSrc, "vert", "frag");
	m_mat = Material::Create(m_shader);

	m_mesh = Mesh::Load("D:\\Projects\\Solar Engine\\src\\test\\test.fbx");
	m_mesh2 = Mesh::Load("D:\\Projects\\Solar Engine\\src\\test\\plane2.fbx");
	
	auto scene = Scene::Create();

	auto e = scene->CreateEntity("Render test", Entity::null);
	auto& r = e.AddComponent<RendererComponent>();
	r.material = m_mat;
	r.mesh = m_mesh;
	e.GetComponent<TransformComponent>().scale = glm::vec3(0.01f);
	
	e = scene->CreateEntity("Render test 2", Entity::null);
	auto& r2 = e.AddComponent<RendererComponent>();
	r2.material = m_mat;
	r2.mesh = m_mesh2;
	e.GetComponent<TransformComponent>().position = glm::vec3(0.f, -2.f, 0.f);

	auto ec = scene->CreateEntity("Camera", Entity::null);
	auto& c = ec.AddComponent<CameraComponent>();
	c.fov = 60;
	c.nearClip = 0.3f;
	c.farClip = 1000.0f;
	c.aspect = 1280.0f / 720;

	auto& t = ec.GetComponent<TransformComponent>();
	t.position = glm::vec3(0, 1, -10);
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
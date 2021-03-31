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

const char* frag = R"(Texture2D    _MainTex;
SamplerState _MainTex_sampler; // By convention, texture samplers must use the '_sampler' suffix

struct PSInput
{
	float4 Pos : SV_POSITION;
    float3 Nrm : TEX_COORD0;
    float2 UV  : TEX_COORD1; 
};

struct PSOutput
{
	float4 Color : SV_TARGET;
};

void main(in  PSInput  PSIn,
	out PSOutput PSOut)
{
	PSOut.Color = lerp(0.015, 1, saturate(dot(PSIn.Nrm, normalize(float3(1, 1, -1)))));//_MainTex.Sample(_MainTex_sampler, PSIn.UV);
})";

const char* vert = R"(cbuffer Constants
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
};

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be indentical.
void main(in  VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos = mul( float4(VSIn.Pos,1.0), (mul(g_Model, g_ViewProj)));
	PSIn.Nrm = mul(float4(VSIn.Nrm, 0), (g_Model)).xyz;
    PSIn.UV  = VSIn.UV;
})";

static Shared<Shader> m_shader;
static Shared<Material> m_mat;
static Shared<Mesh> m_mesh;

void TestApp::Init()
{
	SOLAR_INFO("TestApp::Init()");

	m_shader = ShaderCompiler::Compile("Test Shader", vert, frag);
	m_mat = Material::Create(m_shader);

	m_mesh = Mesh::Load("D:\\Projects\\Solar Engine\\src\\test\\test.fbx");
	
	auto scene = Scene::Create();

	auto e = scene->CreateEntity("Render test", Entity::null);
	auto& r = e.AddComponent<RendererComponent>();
	r.material = m_mat;
	r.mesh = m_mesh;

	auto ec = scene->CreateEntity("Camera", Entity::null);
	auto& c = ec.AddComponent<CameraComponent>();
	c.fov = 60;
	c.nearClip = 0.3f;
	c.farClip = 1000.0f;

	auto& t = ec.GetComponent<TransformComponent>();
	t.position = glm::vec3(0, 0, -500);
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
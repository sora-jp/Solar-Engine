#include "Log.h"
#include "EntryPoint.h"
#include "Scene.h"
#include "Entity.h"
#include "System.h"
#include "dear-imgui/imgui.h"
#include "TransformComponent.h"

class TestApp final : public SolarApp
{
	SOLAR_APPNAME("Test App")
	
public:
	void Init() override;
	void Run() override;
	void Shutdown() override;
};

class TestSystem final : public System<CommonEntityData>
{
public:
	void Execute(const Entity e, CommonEntityData& c) override
	{
		//SOLAR_INFO("{:E} -> {:E}", 
		//	e, 
		//	c.GetParent()
		//);
	}
};

void TestApp::Init()
{
	SOLAR_INFO("TestApp::Init()");
	
	auto scene = Scene::Create();

	for (auto i = 0; i < 10; i++) {
		auto e1 = scene->CreateEntity(fmt::format("Parent ({:03})", i), Entity::null);
		auto e2 = scene->CreateEntity(fmt::format("Child  ({:03})", i), e1);
		//e2.AddComponent<TransformComponent>();
	}
}

void TestApp::Run()
{
	//SOLAR_INFO("TestApp::Run()");
	
	static auto demoOpen = true;
	ImGui::ShowDemoWindow(&demoOpen);
}

void TestApp::Shutdown()
{
	SOLAR_INFO("TestApp::Shutdown()");
}

REGISTER_SYSTEM(TestSystem)
REGISTER_APPLICATION(TestApp)
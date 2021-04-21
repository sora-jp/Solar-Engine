sharedBuildLoc = "build/Combined"
testBuildLog = path.join("build/test/bin/%{cfg.buildcfg}")

function copytoshared()
	postbuildcommands { 
		"{COPY} %{cfg.targetdir} build/Combined",
		"{COPY} %{cfg.targetdir} build/test/bin/%{cfg.buildcfg}"
	}
end

function copyshaders()
	prebuildcommands {
		"{COPY} src/%{prj.name:lower()}/shaders %{cfg.targetdir}/shaders"
	}
end

function enginepch()
	pchheader "pch.h"
	pchsource "src/%{prj.name:lower()}/pch.cpp"
end

function vendor(vendors)
	for _,v in pairs(vendors) do
		includedirs {"vendor/" .. v .. "/"}
		links {"vendor/" .. v .. "/*%{cfg.buildcfg}.lib", "vendor/" .. v .. "/dll/*%{cfg.buildcfg}.dll"}
		if os.isdir("vendor/" .. v .. "/dll") then
			prebuildcommands { 
				"{COPY} vendor/" .. v .. "/dll %{cfg.targetdir}",
			}
		end
	end
end

workspace "Solar"
	language "C++"
	cppdialect "C++17"
	
	architecture "x64"
	staticruntime "Off"
	
	flags
	{
		"MultiProcessorCompile"
	}
	
	platforms { "Win64" }
	configurations { "Debug", "Release" }
	targetdir "build/%{prj.name:lower()}/bin/%{cfg.buildcfg}"
	debugdir "%{cfg.targetdir}"
	objdir "build/%{prj.name:lower()}/obj/%{cfg.buildcfg}"
	includedirs { "include", "include/%{prj.name:lower()}/", "src/%{prj.name:lower()}/"}
	files { "include/%{prj.name:lower()}/**.h", "src/%{prj.name:lower()}/**.h", "src/%{prj.name:lower()}/**.cpp", "src/%{prj.name:lower()}/**.hlsl" }
	defines { "ENTT_NO_ETO=1" }
	--vpaths {
	--	["Source/*"] = {"src/%{prj.name:lower()}/**.h", "src/%{prj.name:lower()}/**.cpp"},
	--	["Include/*"] = {"include/%{prj.name:lower()}/**.h"}
	--}
	
filter { "files:**.hlsl" }
	buildmessage "copy /B /Y \"%{file.relpath:gsub('/', '\\')}\" \"%{cfg.targetdir:gsub('/', '\\')}\\%{file.name}\""
	buildcommands {"copy /B /Y \"%{file.relpath:gsub('/', '\\')}\" \"%{cfg.targetdir:gsub('/', '\\')}\\%{file.name}\""}
	buildoutputs { "%{cfg.targetdir}/%{file.name}" }
	
filter { "platforms:Win64" }
    system "Windows"
    architecture "x64"
	linkoptions { "/LTCG:INCREMENTAL" }
	defines { "PLATFORM_WIN32=1", "D3D_SUPPORTED=1" ,"D3D2_SUPPORTED=1" ,"GL_SUPPORTED=1" ,"VULKAN_SUPPORTED=1" }

filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"

filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"
	
include "vendor/dear-imgui"

project "Core"
	kind "StaticLib"
	defines { "SOLAR_ENGINE_BUILD" }
	copytoshared()
	enginepch()
	vendor {"entt", "spdlog", "glm"}
	
project "Graphics"
	kind "StaticLib"
	defines { "SOLAR_SUBSYSTEM_BUILD", "SOLAR_GRAPHICS_BUILD", "ENGINE_DLL=1", "ENABLE_HLSL" }
	links { "Core", "ImGui" }
	files { "vendor/implot/**.cpp" }
	includedirs {"vendor/spdlog", "vendor/dear-imgui"}
	--copyshaders()
	copytoshared()
	enginepch()
	vendor {"entt", "spdlog", "assimp", "compat", "diligent", "GLFW", "glm", "iconfontheaders", "implot", "stb", "tinystl", "glslang", "freeimage"}
	--enginepch()
	
project "Editor"
	kind "StaticLib"
	links { "Core", "ImGui" }
	includedirs { "vendor/dear-imgui", "vendor/diligent" }
	defines { "SOLAR_SUBSYSTEM_BUILD" }
	copytoshared()
	enginepch()
	vendor { "entt", "spdlog", "glm" }
	
project "Test"
	kind "ConsoleApp"
	links { "Core", "Graphics", "Editor" }
	--copyshaders()
	includedirs { "include/*", "vendor/", "vendor/*" }
	-- prebuildcommands { "{COPY} \"%{sharedBuildLoc}\" \"%{cfg.targetdir}\"" }
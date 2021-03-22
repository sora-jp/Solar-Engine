sharedBuildLoc = "build/Combined"
testBuildLog = path.join("build/test/bin/%{cfg.buildcfg}")

function copytoshared()
	postbuildcommands { 
		"{COPY} %{cfg.targetdir} build/Combined",
		"{COPY} %{cfg.targetdir} build/test/bin/%{cfg.buildcfg}"
	}
end

function enginepch()
	pchheader "pch.h"
	pchsource "src/%{prj.name:lower()}/pch.cpp"
end

workspace "Solar"
	language "C++"
	cppdialect "C++17"
	
	architecture "x64"
	staticruntime "Off"
	
	platforms { "Win64" }
	configurations { "Debug", "Release" }
	targetdir "build/%{prj.name:lower()}/bin/%{cfg.buildcfg}"
	objdir "build/%{prj.name:lower()}/obj/%{cfg.buildcfg}"
	includedirs { "include", "include/%{prj.name:lower()}/", "src/%{prj.name:lower()}/", "vendor/%{prj.name:lower()}/include/" }
	links { "vendor/%{prj.name:lower()}/*%{cfg.buildcfg}.lib" }
	files { "include/%{prj.name:lower()}/**.h", "src/%{prj.name:lower()}/**.h", "src/%{prj.name:lower()}/**.cpp" }
	vpaths {
		["Source/*"] = {"src/%{prj.name:lower()}/**.h", "src/%{prj.name:lower()}/**.cpp"},
		["Include/*"] = {"include/%{prj.name:lower()}/**.h"}
	}

filter { "platforms:Win64" }
    system "Windows"
    architecture "x64"

filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"

filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"

-- project "ImGui"
	-- kind "StaticLib"
	-- defines { "IMGUI_BUILD" }

	-- removelinks()
	-- removefiles()
	-- removevpaths()
	-- removeincludedirs()
	
	-- copytoshared()
	-- files { "vendor/graphics/include/dear-imgui/**.cpp", "vendor/graphics/include/dear-imgui/**.inl", "vendor/graphics/include/dear-imgui/**.h" }
	-- includedirs { "vendor/graphics/include/dear-imgui", "vendor/graphics/include" }

project "Core"
	kind "SharedLib"
	defines { "SOLAR_ENGINE_BUILD" }
	copytoshared()
	enginepch()
	
project "Graphics"
	kind "SharedLib"
	defines { "SOLAR_SUBSYSTEM_BUILD", "SOLAR_GRAPHICS_BUILD" }
	links { "Core" }
	files { "vendor/graphics/include/dear-imgui/**.cpp", "vendor/graphics/include/dear-imgui/**.inl", "vendor/graphics/include/dear-imgui/**.h" }
	includedirs {"vendor/core/include", "vendor/graphics/include/dear-imgui"}
	copytoshared()
	--enginepch()
	
project "Test"
	kind "ConsoleApp"
	links { "Core", "Graphics" }
	
	includedirs { "include/*", "vendor/*/include" }
	-- prebuildcommands { "{COPY} \"%{sharedBuildLoc}\" \"%{cfg.targetdir}\"" }
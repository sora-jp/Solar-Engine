project "ImGuizmo"
	kind "StaticLib"
	links { "ImGui" }
	includedirs { "../dear-imgui" }
	language "C++"

	files
	{
		"*.h",
		"*.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "Off"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "Off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
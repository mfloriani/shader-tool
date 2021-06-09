workspace "Shadertool"
	architecture "x64"
	startproject "Shadertool"
	
	configurations
	{
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"

project "Shadertool"
	location "Shadertool"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "%{prj.name}/src/pch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"Common/spdlog/include",
	}

	libdirs
	{
		
	}

	links
	{
		"d3d12.lib",
		"dxgi.lib"
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			
		}

	filter "configurations:Debug"
		defines 
		{
			"_DEBUG"
		}
		symbols "on"
		buildoptions "/MDd"

	filter "configurations:Release"
		defines 
		{
			
		}
		optimize "on"
		buildoptions "/MD"

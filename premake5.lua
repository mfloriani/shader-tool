workspace "ShaderTool"
	architecture "x64"
	startproject "ShaderTool"
	
	configurations
	{
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"

project "ShaderTool"
	location "ShaderTool"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("bin/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "%{prj.name}/src/pch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hlsl",
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
		"dxgi.lib",
		"dxguid.lib",
		"D3DCompiler.lib"
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
	
	filter 'files:**/ImGui/**.cpp'
		flags  { 'NoPCH' }
	
	filter { "files:**.hlsl" }
		flags "ExcludeFromBuild"
		shadermodel "5.0"

	filter { "files:**_vs.hlsl" }
		removeflags "ExcludeFromBuild"
		shadertype "Vertex"
		-- shaderentry "ForVertex"

	 filter { "files:**_ps.hlsl" }
		removeflags "ExcludeFromBuild"
		shadertype "Pixel"
		-- shaderentry "ForPixel"
		
workspace "ShaderTool"
	architecture "x64"
	startproject "ShaderTool"
	
	configurations
	{
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}/%{prj.name}"

project "ShaderTool"
	location "ShaderTool"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
		
	targetdir ("bin/" .. outputdir)
	objdir ("bin-int/" .. outputdir)
	debugdir ("bin/" .. outputdir)

	pchheader "pch.h"
	pchsource "%{prj.name}/src/pch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hlsl",
		"%{prj.name}/src/**.fx",
		"%{prj.name}/src/**.c",
	}

	includedirs
	{
		"%{prj.name}/src",
		"Common/spdlog/include",
		"Common/assimp/include",
	}

	libdirs
	{
		"Common/assimp/lib"
	}

	links
	{
		"d3d12.lib",
		"dxgi.lib",
		"dxguid.lib",
		"D3DCompiler.lib",
		"assimp-vc142-mt.lib"
	}
	
	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			
		}

		--prebuildcommands
		postbuildcommands
		{
			("{COPY} ../Common/assimp/dll/assimp-vc142-mt.dll %{cfg.targetdir}")
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
	
	filter 'files:**/NFD/**.c'
		flags  { 'NoPCH' }

	filter 'files:**/NFD/**.cpp'
		flags  { 'NoPCH' }
	
	filter { "files:**.hlsl" }
		flags "ExcludeFromBuild"
		shadermodel "5.0"
	
	filter { "files:**_vs.hlsl" }
		removeflags "ExcludeFromBuild"
		shadertype "Vertex"

	 filter { "files:**_ps.hlsl" }
		removeflags "ExcludeFromBuild"
		shadertype "Pixel"
	
	-- To make sure the fx files are up to date in the targetdir
	filter 'files:**.fx'
	   buildmessage 'Copying %{file.relpath}'
	   buildcommands { '{COPY} %{file.relpath} %{cfg.targetdir}' }
	   buildoutputs '%{cfg.targetdir}'
	   
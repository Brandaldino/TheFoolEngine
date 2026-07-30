-- ȫ�ֹ���������
workspace "TheFoolEngine"	-- �����������
	architecture "x64"	-- ָ��64λ�ܹ�
	startproject "Sandbox"

	-- ���幹����������
	configurations{
		"Debug",	-- ��ʽ�汾�������Է��ţ�
		"Release",	-- �Ż��汾
		"Dist"	-- ���а汾
	}

-- ���Ŀ¼����
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"	-- �������� Debug-Windows-x64 ��·��

-- Include directories relative to root folder (solution diretory)
IncludeDir = {}
IncludeDir["GLFW"] = "TheFoolEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "TheFoolEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "TheFoolEngine/vendor/imgui"
IncludeDir["glm"] = "TheFoolEngine/vendor/glm"
IncludeDir["stb_image"] = "TheFoolEngine/vendor/stb_image"
IncludeDir["entt"] = "TheFoolEngine/vendor/entity/include"
IncludeDir["assimp"] = "TheFoolEngine/vendor/assimp/include"
IncludeDir["moodycamel"] = "TheFoolEngine/vendor/moodycamel/include"
IncludeDir["imguizmo"] = "TheFoolEngine/vendor/imguizmo"

include "TheFoolEngine/vendor/GLFW"
include "TheFoolEngine/vendor/Glad"
include "TheFoolEngine/vendor/imgui"
-- include "TheFoolEngine/vendor/assimp"

-- ������Ŀ���ã�DLL��
project "TheFoolEngine"	-- ��Ŀ����
	location "TheFoolEngine"	-- ��Ŀ�ļ����Ŀ¼
	kind "StaticLib"	-- ���ɶ�̬���ӿ�
	language "C++"	-- ����
	cppdialect "C++17"
	staticruntime "On"

	-- ���Ŀ¼����
	targetdir("bin/"..outputdir.."/%{prj.name}")	-- ��������DLL��·��
	objdir("bin-int/"..outputdir.."/%{prj.name}")	-- �м��ļ�·��

	-- ����Ԥ����ͷ�ļ�
	pchheader "tfpch.h"
	pchsource "TheFoolEngine/src/tfpch.cpp"	-- ��MSVC������������

	-- ������Դ���ļ�
	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		-- "%{prj.name}/vendor/assimp/**.cpp",
		-- "%{prj.name}/vendor/assimp/**.h",
		-- "%{prj.name}/vendor/assimp/**.hpp"
		"%{prj.name}/vendor/imguizmo/**.cpp",
		"%{prj.name}/vendor/imguizmo/**.h",
	}

	defines{
		"_CRT_SECURE_NO_WARNINGS"
	}

	-- ͷ�ļ��������������⣩
	includedirs{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.moodycamel}",
		"%{IncludeDir.imguizmo}",
	}

	links{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		-- "Assimp"
	}

	-- �����������windowsƽ̨�ض�����
	filter "system:windows"	-- ����windows��Ч
		-- cppdialect "C++17"	-- ʹ��C++17��׼
		-- staticruntime "On"	 -- ��̬��������ʱ��
		systemversion "latest"	-- ʹ������Windows SDK
		buildoptions{"/utf-8"}

	 -- Ԥ����������
	defines{
		"TF_PLATFORM_WINDOWS",	-- ƽ̨��ʶ��
		"TF_BUILD_DLL",	-- ��ʾ���ڹ���DLL�ĺ�
		"GLFW_INCLUDE_NONE"
	}

	-- �������ù���
	filter "configurations:Debug"
		defines "TF_DEBUG"	-- ����ģʽ�궨��
		runtime "Debug"	
		symbols "on"	-- ���ɵ��Է���
		links {"TheFoolEngine/vendor/assimp/lib/assimp-vc145-mtd.lib"}
		links {"TheFoolEngine/vendor/assimp/lib/zlibstaticd.lib"}

	filter "configurations:Release"
		defines "TF_RELEASE"	-- ����ģʽ�궨��
		runtime "Release"	
		optimize "on"	-- �����Ż�
		links {"TheFoolEngine/vendor/assimp/lib/assimp-vc145-mt.lib"}
		links {"TheFoolEngine/vendor/assimp/lib/zlibstaticd.lib"}

	filter "configurations:Dist"
		defines "TF_DIST"	-- ����ģʽ�궨��
		runtime "Release"	
		optimize "on"	 -- ��߼����Ż�
		links {"TheFoolEngine/vendor/assimp/lib/assimp-vc145-mt.lib"}
		links {"TheFoolEngine/vendor/assimp/lib/zlibstaticd.lib"}

	filter {}

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"	-- ����̨Ӧ�ó���
	language "C++"
	cppdialect "C++17"

	targetdir("bin/"..outputdir.."/%{prj.name}")
	objdir("bin-int/"..outputdir.."/%{prj.name}")
	debugdir "../"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"TheFoolEngine/vendor/spdlog/include",
		"TheFoolEngine/src",
		"TheFoolEngine/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}"
	}

	-- ���������Ŀ�
	links{
		"TheFoolEngine"	-- ���������DLL
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"
		buildoptions{"/utf-8"}

	defines{
		"TF_PLATFORM_WINDOWS"
	}

	filter "configurations:Debug"
		defines "TF_DEBUG"
		runtime "Debug"	-- ���̵߳���
		symbols "on"

	filter "configurations:Release"
		defines "TF_RELEASE"
		runtime "Release"	
		optimize "on"
		symbols "Off"

	filter "configurations:Dist"
		defines "TF_DIST"
		runtime "Release"	
		optimize "on"
		symbols "Off"

	filter {}

project "TheFool-Editor"
	location "TheFool-Editor"
	kind "ConsoleApp"	-- ����̨Ӧ�ó���
	language "C++"
	cppdialect "C++17"

	targetdir("bin/"..outputdir.."/%{prj.name}")
	objdir("bin-int/"..outputdir.."/%{prj.name}")
	debugdir "../"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"TheFoolEngine/vendor/spdlog/include",
		"TheFoolEngine/src",
		"TheFoolEngine/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.imguizmo",
	}

	-- ���������Ŀ�
	links{
		"TheFoolEngine"	-- ���������DLL
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"
		buildoptions{"/utf-8"}

	defines{
		"TF_PLATFORM_WINDOWS"
	}

	filter "configurations:Debug"
		defines "TF_DEBUG"
		runtime "Debug"	-- ���̵߳���
		symbols "on"

	filter "configurations:Release"
		defines "TF_RELEASE"
		runtime "Release"	
		optimize "on"
		symbols "Off"

	filter "configurations:Dist"
		defines "TF_DIST"
		runtime "Release"	
		optimize "on"
		symbols "Off"

	filter {}

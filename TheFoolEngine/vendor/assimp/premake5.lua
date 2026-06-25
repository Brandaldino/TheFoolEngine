project "assimp"
  kind "StaticLib"
  language "C++"
  cppdialect "C++17"
  staticruntime "on"

  targetdir ("bin/" .. outputdir .. "/%{prj.name}")
  objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

  defines {
    "ASSIMP_BUILD_NO_OWN_ZLIB",
  }

  files {
      "include/**",

      "code/Common/**.cpp",
      "code/Common/**.h",
      
      "code/AssetLib/**.cpp",
      "code/AssetLib/**.h",
      
      "code/PostProcessing/**.cpp",
      "code/PostProcessing/**.h",
      
      "code/Material/**.cpp",
      "code/Material/**.h",

      "contrib/irrXML/*",
  }

  includedirs {
      "include",

      "code",
      "code/Common",
      "code/AssetLib",
      "code/PostProcessing",
      "code/Material",
      
      "contrib",
      "contrib/irrXML",
      "contrib/zlib",
      "contrib/rapidjson/include",
  }


   filter "system:windows"
      systemversion "latest"

   filter  "configurations:Debug"
       runtime "Debug"
       symbols "on"

   filter  "configurations:Release"
       runtime "Release"
       optimize "on"

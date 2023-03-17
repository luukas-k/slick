workspace "Slick"
    configurations { "Debug", "Release", "Distribution" }

    project "Core"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        architecture "x86_64"
        
        links {
            "Ws2_32"
        }

        includedirs {
            "core/"
        }

        files { 
            "core/**.h", 
            "core/**.cpp" 
        }

        filter { "configurations:Debug" }
            defines { "DEBUG", "_ITERATOR_DEBUG_LEVEL=0" }
            optimize "Debug"
            symbols "On"

        filter { "configurations:Release" }
            defines { "NDEBUG" }
            optimize "On"

            
        filter { "configurations:Distribution" }
            defines { "NDEBUG" }
            symbols "off"
            optimize "Full"

    project "Editor"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        architecture "x86_64"
        
        links {
            "Core"
        }

        includedirs {
            "core/"
        }

        files { 
            "editor/**.h", 
            "editor/**.cpp" 
        }

        filter { "configurations:Debug" }
            defines { "DEBUG", "_ITERATOR_DEBUG_LEVEL=0" }
            symbols "On"

        filter { "configurations:Release" }
            defines { "NDEBUG" }
            optimize "On"

            
        filter { "configurations:Distribution" }
            defines { "NDEBUG" }
            symbols "off"
            optimize "Full"
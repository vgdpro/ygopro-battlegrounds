solution "ygo"
    location "build"
    language "C++"
    objdir "obj"
    if os.ishost("windows") or os.getenv("USE_IRRKLANG") then
        USE_IRRKLANG = true
        if os.getenv("irrklang_pro") then
            IRRKLANG_PRO = true
        end
    end
    if not os.ishost("windows") then
        if os.getenv("YGOPRO_BUILD_LUA") then
            BUILD_LUA=true
        end
        if os.getenv("YGOPRO_BUILD_SQLITE") then
            BUILD_SQLITE=true
        end
        if os.getenv("YGOPRO_BUILD_FREETYPE") then
            BUILD_FREETYPE=true
        end
        if os.getenv("YGOPRO_BUILD_ALL") or os.ishost("macosx") then
            BUILD_ALL=true
        end
        if os.ishost("linux") and os.getenv("YGOPRO_LINUX_ALL_STATIC") then
            BUILD_ALL=true
            LINUX_ALL_STATIC=true
            LIB_ROOT=os.getenv("YGOPRO_LINUX_ALL_STATIC_LIB_PATH") or "/usr/lib/x86_64-linux-gnu/"
            LIBEVENT_ROOT=os.getenv("YGOPRO_LINUX_ALL_STATIC_LIBEVENT_PATH")
        end
        if BUILD_ALL then
            BUILD_LUA=true
            BUILD_SQLITE=true
            BUILD_FREETYPE=true
        end
    end

    configurations { "Release", "Debug" }
    configuration "windows"
        defines { "WIN32", "_WIN32", "WINVER=0x0501" }
        libdirs { "$(DXSDK_DIR)Lib/x86" }
        entrypoint "mainCRTStartup"
        systemversion "latest"
        startproject "ygopro"
        
    configuration { "windows", "vs2015" }
        toolset "v140_xp"

    configuration { "windows", "vs2017" }
        toolset "v141_xp"

    configuration { "windows", "vs2019" }
        toolset "v141_xp"

    configuration "bsd"
        defines { "LUA_USE_POSIX" }
        includedirs { "/usr/local/include" }
        libdirs { "/usr/local/lib" }

    configuration "macosx"
        defines { "LUA_USE_MACOSX", "DBL_MAX_10_EXP=+308", "DBL_MANT_DIG=53", "GL_SILENCE_DEPRECATION" }
        includedirs { "/usr/local/include/event2", }
        libdirs { "/usr/local/lib" }
        buildoptions { "-stdlib=libc++" }
        links { "OpenGL.framework", "Cocoa.framework", "IOKit.framework" }

    configuration "linux"
        defines { "LUA_USE_LINUX" }

    configuration "Release"
        optimize "Speed"
        targetdir "bin/release"

    configuration "Debug"
        symbols "On"
        defines "_DEBUG"
        targetdir "bin/debug"

    configuration { "Release", "vs*" }
        flags { "StaticRuntime", "LinkTimeOptimization" }
        --staticruntime "On"
        disablewarnings { "4244", "4267", "4838", "4577", "4819", "4018", "4996", "4477", "4091", "4828", "4800" }

    configuration { "Release", "not vs*" }
        symbols "On"
        defines "NDEBUG"
        buildoptions "-march=native"

    configuration { "Debug", "vs*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0" }
        disablewarnings { "4819", "4828" }

    configuration "vs*"
        vectorextensions "SSE2"
        defines { "_CRT_SECURE_NO_WARNINGS" }
    
    configuration "not vs*"
        buildoptions { "-fno-strict-aliasing", "-Wno-multichar" }

    configuration {"not vs*", "windows"}
        buildoptions { "-static-libgcc" }

    include "ocgcore"
    include "gframe"
    if os.ishost("windows") then
        include "lua"
        include "event"
        include "freetype"
        include "irrlicht"
        include "sqlite3"
    else
        if BUILD_LUA then
            include "lua"
        end
        if BUILD_SQLITE then
            include "sqlite3/premake4.lua"
        end
        if BUILD_FREETYPE then
            include "freetype"
        end
    end
    if os.ishost("linux") then
        include "irrlicht_linux"
    end
    if USE_IRRKLANG then
        include "ikpmp3"
    end

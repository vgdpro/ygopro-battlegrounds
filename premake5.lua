solution "ygo"
    location "build"
    language "C++"
    objdir "obj"
    if os.getenv("YGOPRO_BUILD_LUA") then
        BUILD_LUA=true
    end
    if os.getenv("YGOPRO_BUILD_SQLITE") then
        BUILD_SQLITE=true
    end
    if os.getenv("YGOPRO_BUILD_FREETYPE") then
        BUILD_FREETYPE=true
    end
    if os.getenv("YGOPRO_BUILD_IRRLICHT") then
        BUILD_IRRLICHT=true
    end
    if os.getenv("YGOPRO_LIBEVENT_STATIC_PATH") then
        LIBEVENT_ROOT=os.getenv("YGOPRO_LIBEVENT_STATIC_PATH")
    end
    if os.ishost("macosx") then
        if os.getenv("YGOPRO_TARGET_ARM") then
            MAC_ARM=true
        end
    end
    if os.ishost("windows") then
        BUILD_EVENT=true -- only on windows for now
    end
    if os.getenv("USE_IRRKLANG") and not os.getenv("NO_IRRKLANG") then
        USE_IRRKLANG = true
        if os.getenv("IRRKLANG_PRO") then
            IRRKLANG_PRO = true
        end
    end

    configurations { "Release", "Debug" }
if os.getenv("YGOPRO_LUA_SAFE") then
    defines { "LUA_COMPAT_5_2", "YGOPRO_LUA_SAFE" }
else
    defines { "LUA_COMPAT_5_2" }
end
    configuration "windows"
        defines { "WIN32", "_WIN32", "WINVER=0x0501" }
        libdirs { "$(DXSDK_DIR)Lib/x86" }
        entrypoint "mainCRTStartup"
        --toolset "v141_xp"
        systemversion "latest"
        startproject "ygopro"

if not os.getenv("YGOPRO_NO_XP_TOOLSET") then
    configuration { "windows", "vs2015" }
        toolset "v140_xp"

    configuration { "windows", "vs2017" }
        toolset "v141_xp"

    configuration { "windows", "vs2019" }
        toolset "v141_xp"
end

    configuration "bsd"
        includedirs { "/usr/local/include" }
        libdirs { "/usr/local/lib" }

    configuration "macosx"
        defines { "GL_SILENCE_DEPRECATION" }
        if not LIBEVENT_ROOT then
            includedirs { "/usr/local/include/event2" }
            libdirs { "/usr/local/lib" }
        end
        buildoptions { "-stdlib=libc++" }
        if MAC_ARM then
            buildoptions { "--target=arm64-apple-macos12" }
        end
        links { "OpenGL.framework", "Cocoa.framework", "IOKit.framework" }

    configuration "linux"
        buildoptions { "-U_FORTIFY_SOURCE" }

    configuration "Release"
        optimize "Speed"
        targetdir "bin/release"

    configuration "Debug"
        symbols "On"
        defines "_DEBUG"
        targetdir "bin/debug"

    configuration { "Release", "vs*" }
        flags { "LinkTimeOptimization" }
        staticruntime "On"
        disablewarnings { "4244", "4267", "4838", "4577", "4819", "4018", "4996", "4477", "4091", "4828", "4800" }

    configuration { "Release", "not vs*" }
        symbols "On"
        defines "NDEBUG"
        if not MAC_ARM then
            buildoptions "-march=native"
        end

    configuration { "Debug", "vs*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0" }
        disablewarnings { "4819", "4828" }

    configuration "vs*"
        vectorextensions "SSE2"
        buildoptions { "/utf-8" }
        defines { "_CRT_SECURE_NO_WARNINGS" }
    
    configuration "not vs*"
        buildoptions { "-fno-strict-aliasing", "-Wno-format-security" }

    configuration {"not vs*", "windows"}
        buildoptions { "-static-libgcc" }

    include "ocgcore"
    include "gframe"
    if BUILD_LUA then
        include "lua"
    end
    if BUILD_EVENT then
        include "event"
    end
    if BUILD_FREETYPE then
        include "freetype"
    end
    if BUILD_IRRLICHT then
        include "irrlicht"
    end
    if BUILD_SQLITE then
        include "sqlite3"
    end
    if USE_IRRKLANG then
        include "ikpmp3"
    end

local Config = {
  Debug = true,
  GarrysMod = "Q:/Games/Steam Games/steamapps/common/GarrysMod"
}
local Boost = require "boost"

-- Fix for VS2017
-- https://github.com/premake/premake-core/issues/935#issuecomment-343491487
function os.winSdkVersion()
  local reg_arch = iif(os.is64bit(), "\\Wow6432Node\\", "\\")
  local sdk_version = os.getWindowsRegistry("HKLM:SOFTWARE"..reg_arch.."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion")
  if sdk_version ~= nil then return sdk_version end
end

solution "gm_socket.io"
  flags {  }
  language "C++"
  location "project"
  targetdir "bin"
  cppdialect "C++11"
  architecture "x86"

  configurations { "Release" }
  configuration "Release"
    optimize "On"

  project "gm_socket.io"
    characterset "MBCS"
    include "lib/LuaInterface"

    files {
      "src/*.hpp",
      "src/*.cpp",
      "src/**/*.hpp",
      "src/**/*.cpp",

      "lib/socket.io/src/*.h",
      "lib/socket.io/src/*.cpp",
      "lib/socket.io/src/internal/*.h",
      "lib/socket.io/src/internal/*.cpp",
    }
    defines {
      "GMMODULE",
      "_WEBSOCKETPP_CPP11_STL_"
    }

    filter {"system:windows", "action:vs*"}
      defines {
        "UNICODE",
        "_UNICODE",
        "_CRT_SECURE_NO_WARNINGS",
        "_SCL_SECURE_NO_WARNINGS",
        "NOMINMAX",
        "_WEBSOCKETPP_CPP11_FUNCTIONAL_",
        "_WEBSOCKETPP_CPP11_SYSTEM_ERROR_",
        "_WEBSOCKETPP_CPP11_RANDOM_DEVICE_",
        "_WEBSOCKETPP_CPP11_MEMORY_"
      }
      runtime "Release"
      systemversion(os.winSdkVersion() .. ".0")

    Boost.link "system"
    Boost.link "date_time"
    Boost.link "random"
    Boost.apply()

    includedirs {
      "lib/socket.io/src",
      "lib/socket.io/lib/rapidjson/include",
      "lib/socket.io/lib/websocketpp"
    }

    if Config.Debug then
      postbuildcommands {
        '{RMDIR} "'..Config.GarrysMod..'/garrysmod/lua/bin"',
        '{MKDIR} "'..Config.GarrysMod..'/garrysmod/lua/bin"',
        '{COPY} "%{cfg.buildtarget.abspath}" ' ..
          '"'..Config.GarrysMod..'/garrysmod/lua/bin"',
        '{MOVE} "'..Config.GarrysMod..'/garrysmod/lua/bin/%{cfg.buildtarget.name}" ' ..
          '"'..Config.GarrysMod..'/garrysmod/lua/bin/gmcl_socket.io%{cfg.buildtarget.suffix}.dll"',
      }
    end

    kind "SharedLib"
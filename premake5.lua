local Boost = require "boost"
local Testing = require "testing"

-- https://github.com/premake/premake-core/issues/935#issuecomment-343491487
function os.winSdkVersion()
  local reg_arch = iif(os.is64bit(), "\\Wow6432Node\\", "\\")
  local sdk_version = os.getWindowsRegistry("HKLM:SOFTWARE"..reg_arch.."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion")
  if sdk_version ~= nil then return sdk_version end

  return ""
end

solution "gm_socket.io"
  flags { }
  language "C++"
  location "project"
  targetdir "bin"
  cppdialect "C++14"
  architecture "x86"

  configurations { "Release" }
  configuration "Release"
    optimize "On"
  
  -- https://github.com/zaphoyd/websocketpp/blob/ac5d7ea5af9734de965688b54a7860259887b537/CMakeLists.txt#L101
  filter { "system:windows", "action:vs*" }
    defines {
      "UNICODE",
      "_UNICODE",
      "_WINSOCK_DEPRECATED_NO_WARNINGS",
      "_CRT_SECURE_NO_WARNINGS",
      "_SCL_SECURE_NO_WARNINGS",
      "NOMINMAX"
    }
    runtime "Release"
    systemversion(os.winSdkVersion() .. ".0")
    
  filter { "system:linux" }
    defines {
      "NDEBUG"
    }
    
  filter { "system:macosx" }
    defines {
      "NDEBUG"
    }

  project "gm_socket.io"
    kind "SharedLib"
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
      "_WEBSOCKETPP_CPP11_STL_",
      "_WEBSOCKETPP_CPP11_FUNCTIONAL_",
      "_WEBSOCKETPP_CPP11_SYSTEM_ERROR_",
      "_WEBSOCKETPP_CPP11_RANDOM_DEVICE_",
      "_WEBSOCKETPP_CPP11_MEMORY_"
    }

    

    Boost.link "system"
    Boost.link "date_time"
    Boost.link "random"
    Boost.apply()
    Testing.apply()

    includedirs {
      "lib/socket.io/src",
      "lib/socket.io/lib/rapidjson/include",
      "lib/socket.io/lib/websocketpp"
    }
local Testing = {
  enabled = true,

  linux = {},
  macosx = {},
  windows = {
    gmod_dir = "Q:\\Games\\Steam Games\\steamapps\\common\\GarrysMod"
  },
}

function Testing.apply() 
  if not Testing.enabled then return end

  local cfg = Testing[os.target()]

  postbuildcommands {
    '{COPY} "%{cfg.buildtarget.abspath}" '..
      '"'..cfg.gmod_dir..'\\garrysmod\\lua\\bin\\gmcl_socket.io_win32.dll*"',

    '{COPY} "..\\test\\test.lua" '..
      '"'..cfg.gmod_dir..'\\garrysmod\\lua\\autorun\\socket.io-test.lua*"'
  }
end

return Testing
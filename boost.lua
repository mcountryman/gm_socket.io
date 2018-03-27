local Boost = {
  macos = {},
  linux = {
    lib_dir        = "/usr/lib/x86_64-gnu/",
    lib_prefix     = "libboost_",
    lib_suffix     = "",
    include_dir    = "/usr/include/boost",

    links = {}
  },
  windows = {
    lib_dir        = "Q:/Development/deps/boost_1_55_0/lib",
    lib_prefix     = "libboost_",
    lib_suffix     = "-vc141-mt-1_55.lib",
    include_dir    = "Q:/Development/deps/boost_1_55_0",

    links = {}
  }
}

function Boost.link(module) 
  local cfg = Boost[os.target()]
  
  table.insert(cfg.links, cfg.lib_prefix..module..cfg.lib_suffix)
end
function Boost.apply() 
  local cfg = Boost[os.target()]

  links(cfg.links)
  libdirs(cfg.lib_dir)
  defines {}
  includedirs(cfg.include_dir)
end

return Boost
#include "engine.hpp"
#include <GarrysMod/Lua/Interface.h>

GMOD_MODULE_OPEN() {
  gm_socket_io::Engine::FromState(state)->Open(state);

  return 0;
}
GMOD_MODULE_CLOSE() {
  return 0;
}
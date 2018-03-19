#include "core/engine.hpp"

GMOD_MODULE_OPEN() {
  return gm_socket_io::Engine::on_open(state);
}
GMOD_MODULE_CLOSE() {
  return gm_socket_io::Engine::on_close(state);
}
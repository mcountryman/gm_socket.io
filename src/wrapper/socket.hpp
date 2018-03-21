#ifndef _GM_SOCKET_IO_WRAPPER_SOCKET_HPP_
#define _GM_SOCKET_IO_WRAPPER_SOCKET_HPP_

#include "../config.hpp"
#include <sio_socket.h>
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  namespace socket_wrapper {
    // Push new io.Socket object
    int make(lua_State *state, sio::socket::ptr socket);

    int on(lua_State *state);

    int off(lua_State *state);
    int off_all(lua_State *state);

    int emit(lua_State *state);

    int get_namespace(lua_State *state);
  }
}

#endif//_GM_SOCKET_IO_WRAPPER_SOCKET_HPP_
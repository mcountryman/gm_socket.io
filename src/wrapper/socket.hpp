#ifndef _GM_SOCKET_IO_WRAPPER_SOCKET_HPP_
#define _GM_SOCKET_IO_WRAPPER_SOCKET_HPP_

#include <GarrysMod/Lua/Interface.h>

#include <sio_socket.h>

#define kSOCKET_TYPE 223

namespace gm_socket_io {
  namespace socket_wrapper {
    int __gc(lua_State *state);
    int __index(lua_State *state);

    int make(lua_State *state, sio::socket::ptr socket);

    int on(lua_State *state);
    int off(lua_State *state);
    int emit(lua_State *state);
  }
}

#endif//_GM_SOCKET_IO_WRAPPER_SOCKET_HPP_
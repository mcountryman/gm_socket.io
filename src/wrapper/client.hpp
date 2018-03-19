#ifndef _GM_SOCKET_IO_WRAPPER_CLIENT_HPP_
#define _GM_SOCKET_IO_WRAPPER_CLIENT_HPP_

#include <sio_socket.h>
#include <GarrysMod/Lua/Interface.h>

#define CLIENT_TYPE 222

namespace gm_socket_io {
  namespace ClientWrapper {
    int __gc(lua_State *state);
    int __index(lua_State *state);

    int make(lua_State *state);

    int on(lua_State *state);
    int socket(lua_State *state);
    int connect(lua_State *state);
    int disconnect(lua_State *state);
  }
}

#endif//_GM_SOCKET_IO_WRAPPER_CLIENT_HPP_
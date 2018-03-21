#ifndef _GM_SOCKET_IO_WRAPPER_CLIENT_HPP_
#define _GM_SOCKET_IO_WRAPPER_CLIENT_HPP_

#include "../config.hpp"
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  namespace client_wrapper {
    // Close and delete sio::client
    int __gc(lua_State *state);

    // Push new io.Client object
    int make(lua_State *state);

    int on(lua_State *state);

    int socket(lua_State *state);

    int connect(lua_State *state);

    int disconnect(lua_State *state);

    int is_connected(lua_State *state);
    int set_reconnect_delay(lua_State *state);
    int set_reconnect_delay_max(lua_State *state);
    int set_reconnect_attempts(lua_State *state);
  }
}

#endif//_GM_SOCKET_IO_WRAPPER_CLIENT_HPP_
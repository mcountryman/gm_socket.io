#ifndef _GM_SOCKET_IO_CORE_MESSAGE_HPP_
#define _GM_SOCKET_IO_CORE_MESSAGE_HPP_

#include <sio_message.h>
#include <GarrysMod/Lua/Interface.h>

#define kMAX_ARRAY_DEPTH 1000
#define kMAX_TABLE_DEPTH 1000

namespace gm_socket_io {
  namespace message {
    int push_message(lua_State *state, sio::message::ptr message);

    bool is_array(lua_State *state, int position = 1);

    sio::message::ptr get_message_from_table(lua_State *state, int position = 1, int depth = 0);
    sio::message::ptr get_message_from_array(lua_State *state, int position = 1, int depth = 0);
    sio::message::ptr get_message(lua_State *state, int position = 1, int depth = 0);
  }
}

#endif//_GM_SOCKET_IO_CORE_MESSAGE_HPP_
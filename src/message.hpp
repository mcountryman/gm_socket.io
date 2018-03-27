#pragma once

#include "lua_object.hpp"
#include <sio_message.h>

typedef struct lua_State;

namespace gm_socket_io {
  class Message : public LuaObject<Message> {
  public:
    static const int             type_ = 225;
    static constexpr const char *name_ = "gm_socket_io::Message";
  public:
    sio::message::ptr impl_;
  public:
    Message(lua_State *state, sio::message::ptr message) : LuaObject(state) {
      impl_ = message;

      AddMethod("get", get);
    }
  public:
    static int get(lua_State *state);
  public:
    static int to_null(lua_State *state);
    static int to_array(lua_State *state);
    static int to_object(lua_State *state);
    static int to_string(lua_State *state);
    static int to_binary(lua_State *state);
    static int to_integer(lua_State *state);
    static int to_double(lua_State *state);
    static int to_boolean(lua_State *state);
  public:
    static sio::message::ptr ToMessage(lua_State *state, int position = 1);
    static int PushMessage(lua_State *state, sio::message::ptr message);
  };
}
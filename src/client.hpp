#pragma once

#include "socket.hpp"
#include "lua_object.hpp"
#include "event_emitter.hpp"
#include <tuple>
#include <string>
#include <sio_client.h>

namespace gm_socket_io {
  class Client : public LuaObject<Client> {
  public:
    static const int             type_ = 223;
    static constexpr const char *name_ = "gm_socket_io::Client";
  private:
    sio::client impl_;
  private:
    EmptyEventEmitter error_emitter_;
    EmptyEventEmitter connect_emitter_;
    EmptyEventEmitter reconnecting_emitter_;
    EventEmitter<sio::client::close_reason> disconnect_emitter_;
    EventEmitter<unsigned int, unsigned int> reconnect_emitter_;
  public:
    Client(lua_State *state) : LuaObject(state) {
      AddMethod("on", on);
      AddMethod("off", off);
      AddMethod("off_all", off_all);
      AddMethod("socket", socket);
      AddMethod("connect", connect);
      AddMethod("disconnect", disconnect);

      AddGetter("is_open", get_is_open);
      AddSetter("reconnect_delay", set_reconnect_delay);
      AddSetter("reconnect_attempts", set_reconnect_attempts);
      AddSetter("reconnect_delay_max", set_reconnect_delay_max);

      Register(state);
    }
  public:
    void Think(lua_State *state);
    void Dispose(lua_State *state);
    void Register(lua_State *state);
  public:
    static int get_is_open(lua_State *state);
    static int set_reconnect_delay(lua_State *state);
    static int set_reconnect_attempts(lua_State *state);
    static int set_reconnect_delay_max(lua_State *state);
  public:
    static int on(lua_State *state);
    static int off(lua_State *state);
    static int off_all(lua_State *state);
    static int socket(lua_State *state);
    static int connect(lua_State *state);
    static int disconnect(lua_State *state);
  };
}
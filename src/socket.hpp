#pragma once

#include "lua_object.hpp"
#include "event_emitter.hpp"
#include <string>
#include <sio_socket.h>
#include <sio_message.h>
#include <functional>

namespace gm_socket_io {
  class Socket : public LuaObject<Socket> {
  public:
    static const int             type_ = 224;
    static constexpr const char *name_ = "gm_socket_io::Socket";
  public:
    sio::socket::ptr impl_;
  private:
    EventEmitter<sio::message::ptr> error_handler_;
    KeyedEventEmitter<std::string, sio::message::ptr, bool, sio::array_message::ptr> message_handler_;
  public:
    Socket(lua_State *state, sio::socket::ptr socket) : LuaObject(state) {
      impl_ = socket;

      AddMethod("on", on);
      AddMethod("off", off);
      AddMethod("off_all", off_all);
      AddMethod("emit", emit);
      AddGetter("namespace", get_namespace);

      Register(state);
    }
  public:
    void Think(lua_State *state);
    void Dispose(lua_State *state);
    void Register(lua_State *state);
  public:
    static int get_namespace(lua_State *state);
  public:
    static int on(lua_State *state);
    static int off(lua_State *state);
    static int off_all(lua_State *state);
    static int emit(lua_State *state);
  };
}
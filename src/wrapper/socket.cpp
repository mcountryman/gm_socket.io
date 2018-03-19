#include "socket.hpp"
#include "../core/engine.hpp"
#include "../core/message.hpp"

#include <iostream>
#include <sio_client.h>
#include <sio_message.h>
#include <Windows.h>

namespace gm_socket_io {
  namespace SocketWrapper {
    int make(lua_State *state, sio::socket::ptr socket) {
      LUA->CreateTable();
        LUA->PushCFunction(on);
        LUA->SetField(-2, "on");

        LUA->PushCFunction(off);
        LUA->SetField(-2, "off");

        LUA->PushCFunction(emit);
        LUA->SetField(-2, "emit");
      int socket_ref = LUA->ReferenceCreate();

      GarrysMod::Lua::UserData *ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
      ud->data = socket.get();
      ud->type = SOCKET_TYPE;
      LUA->CreateTable();

      LUA->ReferencePush(socket_ref);
      LUA->SetField(-2, "__index");

      LUA->ReferencePush(socket_ref);
      LUA->SetField(-2, "__newindex");
      LUA->SetMetaTable(-2);

      LUA->ReferenceFree(socket_ref);

      return 1;
    }

    int on(lua_State *state) {
      LUA->CheckType(1, SOCKET_TYPE);
      LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
      LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

      auto engine = Engine::from_state(state);
      auto socket = (sio::socket*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(1))->data;
      auto name = std::string(LUA->GetString(2));
      auto cb = LUA->GetCFunction(3);
      auto cb_ref = LUA->ReferenceCreate();

      if (name == "error") {
        socket->on_error([engine, cb_ref](sio::message::ptr message) {
          engine->emit(cb_ref, [message](lua_State *state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            Message::push_message(state, message);
            LUA->Call(1, 0);
          });
        });
      } else {
        socket->on(name, [engine, cb_ref](const std::string &name, sio::message::ptr const &message, bool need_ack, sio::message::list &ack_message) {
          engine->emit(cb_ref, [message, need_ack, ack_message](lua_State *state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            Message::push_message(state, message);
            LUA->PushBool(need_ack);
            Message::push_message(state, ack_message.to_array_message());
            LUA->Call(3, 0);
          });
        });
      }

      return 0;
    }
    int off(lua_State *state) {
      LUA->CheckType(1, SOCKET_TYPE);
      LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

      auto socket = (sio::socket *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto name = std::string(LUA->GetString(2));

      if (name == "error") {
        socket->off_error();
      } else {
        socket->off(name);
      }

      return 0;
    }
    int emit(lua_State *state) {
      LUA->CheckType(1, SOCKET_TYPE);
      LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

      auto socket = (sio::socket *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto name = std::string(LUA->GetString(2));
      std::function<void(const sio::message::list&)> ack = nullptr;

      sio::message::list messages;

      int arg_start = 3;
      int arg_end = arg_start + LUA->Top();
      
      for (int i = arg_start; i < arg_end; i++) {
        messages.push(Message::get_message_from_args(state, i));
      }

      if (LUA->IsType(arg_end + 1, GarrysMod::Lua::Type::FUNCTION)) {
        auto cb = LUA->GetCFunction(arg_end + 1);
        auto cb_ref = LUA->ReferenceCreate();
        auto engine = Engine::from_state(state);

        ack = [engine, cb_ref](const sio::message::list &messages) {
          sio::message::ptr array_message = messages.to_array_message();

          engine->emit(cb_ref, [array_message](lua_State *state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            Message::push_message(state, array_message);
            LUA->Call(1, 0);
          });
        };
      }

      socket->emit(name, messages, ack);

      return 0;
    }
  }
}
#include "socket.hpp"
#include "../core/engine.hpp"
#include "../core/message.hpp"

#include <iostream>
#include <sio_client.h>
#include <sio_message.h>
#include <Windows.h>

namespace gm_socket_io {
  namespace socket_wrapper {
    int make(lua_State *state, sio::socket::ptr socket) {
      LUA->CreateTable();
        LUA->PushCFunction(on);
        LUA->SetField(-2, "on");

        LUA->PushCFunction(off);
        LUA->SetField(-2, "off");

        LUA->PushCFunction(off_all);
        LUA->SetField(-2, "off_all");

        LUA->PushCFunction(emit);
        LUA->SetField(-2, "emit");

        LUA->PushCFunction(get_namespace);
        LUA->SetField(-2, "get_namespace");
      int socket_ref = LUA->ReferenceCreate();

      GarrysMod::Lua::UserData *ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
      ud->data = socket.get();
      ud->type = kGM_SOCKET_SOCKET_TYPE;
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
      LUA->CheckType(1, kGM_SOCKET_SOCKET_TYPE);
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
            message::push_message(state, message);
            LUA->Call(1, 0);
          });
        });
      } else {
        socket->on(name, [engine, cb_ref](const std::string &name, sio::message::ptr const &message, bool need_ack, sio::message::list &ack_message) {
          engine->emit(cb_ref, [message, need_ack, ack_message](lua_State *state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            message::push_message(state, message);
            LUA->PushBool(need_ack);
            message::push_message(state, ack_message.to_array_message());
            LUA->Call(3, 0);
          });
        });
      }

      return 0;
    }

    int off(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_SOCKET_TYPE);
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

    int off_all(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_SOCKET_TYPE);
      auto socket = (sio::socket *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;

      socket->off_all();

      return 0;
    }

    int emit(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_SOCKET_TYPE);
      LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

      auto socket = (sio::socket *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto name = std::string(LUA->GetString(2));
      std::function<void(const sio::message::list&)> ack = nullptr;

      sio::message::list messages;

      LUA->Push(3);
      messages.push(message::get_message(state, -1));
      LUA->Pop();

      if (LUA->IsType(4, GarrysMod::Lua::Type::FUNCTION)) {
        auto cb = LUA->GetCFunction(4);
        auto cb_ref = LUA->ReferenceCreate();
        auto engine = Engine::from_state(state);

        ack = [engine, cb_ref](const sio::message::list &messages) {
          sio::message::ptr array_message = messages.to_array_message();

          engine->emit(cb_ref, [array_message](lua_State *state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            message::push_message(state, array_message);
            LUA->Call(1, 0);
          });
        };
      }

      socket->emit(name, messages, ack);

      return 0;
    }

    int get_namespace(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_SOCKET_TYPE);
      auto socket = (sio::socket *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;

      LUA->PushString(socket->get_namespace().c_str());

      return 1;
    }
  }
}
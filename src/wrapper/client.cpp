#include "client.hpp"
#include "socket.hpp"
#include "../core/engine.hpp"

#include <map>
#include <string>
#include <sio_client.h>
#include <sio_socket.h>

namespace gm_socket_io {
  namespace client_wrapper {
    int __gc(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);
      auto client = (sio::client *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;

      if (client->opened()) {
        client->close();
      }

      delete client;

      return 0;
    }

    int make(lua_State *state) {
      LUA->CreateTable();
        LUA->PushCFunction(on);
        LUA->SetField(-2, "on");

        LUA->PushCFunction(socket);
        LUA->SetField(-2, "socket");

        LUA->PushCFunction(connect);
        LUA->SetField(-2, "connect");

        LUA->PushCFunction(disconnect);
        LUA->SetField(-2, "disconnect");

        LUA->PushCFunction(is_connected);
        LUA->SetField(-2, "is_connected");

        LUA->PushCFunction(set_reconnect_delay);
        LUA->SetField(-2, "reconnect_delay");

        LUA->PushCFunction(set_reconnect_delay_max);
        LUA->SetField(-2, "set_reconnect_delay_max");

        LUA->PushCFunction(set_reconnect_attempts);
        LUA->SetField(-2, "set_reconnect_attempts");


        LUA->CreateTable();
        LUA->SetField(-2, "__sockets");

      auto client_ref = LUA->ReferenceCreate();
      auto client = new sio::client();

      GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
        ud->data = client;
        ud->type = kGM_SOCKET_CLIENT_TYPE;
      LUA->CreateTable();
        LUA->PushCFunction(__gc);
        LUA->SetField(-2, "__gc");

        LUA->ReferencePush(client_ref);
        LUA->SetField(-2, "__index");

        LUA->ReferencePush(client_ref);
        LUA->SetField(-2, "__newindex");
      LUA->SetMetaTable(-2);

      LUA->ReferenceFree(client_ref);

      return 1;
    }

    int on(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);
      LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
      LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

      auto engine = Engine::from_state(state);
      auto client = (sio::client*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(1))->data;
      auto key = std::string(LUA->GetString(2));
      auto cb = LUA->GetCFunction(3);
      auto cb_ref = LUA->ReferenceCreate();

      if (key == "connect") {
        client->set_open_listener([engine, cb_ref]() {
          engine->emit(cb_ref, [](lua_State* state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            LUA->Call(0, 0);
          });
        });
      } else if (key == "error") {
        client->set_fail_listener([engine, cb_ref]() {
          engine->emit(cb_ref, [](lua_State* state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            LUA->Call(0, 0);
          });
        });
      } else if (key == "disconnect") {
        client->set_close_listener([engine, cb_ref](sio::client::close_reason reason) {
          engine->emit(cb_ref, [reason](lua_State* state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            LUA->PushNumber(reason);
            LUA->Call(1, 0);
          });
        });
      } else if (key == "reconnect") {
        client->set_reconnect_listener([engine, cb_ref](unsigned a, unsigned b) {
          engine->emit(cb_ref, [](lua_State* state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            LUA->Call(0, 0);
          });
        });
      } else if (key == "reconnecting") {
        client->set_reconnecting_listener([engine, cb_ref]() {
          engine->emit(cb_ref, [](lua_State* state, int cb_ref) {
            LUA->ReferencePush(cb_ref);
            LUA->Call(0, 0);
          });
        });
      }

      return 0;
    }

    int socket(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);

      auto client = (sio::client*)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto nsp = std::string("");

      if (LUA->IsType(2, GarrysMod::Lua::Type::STRING)) {
        nsp = std::string(LUA->GetString(2));
      }

      socket_wrapper::make(state, client->socket(nsp));
      return 1;
    }

    int connect(lua_State *state) { 
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);  
      LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

      std::map<std::string, std::string> query;
      std::map<std::string, std::string> headers;

      if (LUA->IsType(3, GarrysMod::Lua::Type::TABLE)) {
        // TODO: This
      }

      if (LUA->IsType(4, GarrysMod::Lua::Type::TABLE)) {
        // TODO: This
      }

      auto client = (sio::client*)((GarrysMod::Lua::UserData*)LUA->GetUserdata(1))->data;
      auto uri = LUA->GetString(2);

      client->connect(std::string(uri), query, headers);

      return 0;
    }

    int disconnect(lua_State *state) { 
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);
      auto client = (sio::client *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      
      client->close();

      return 0;
    }

    int is_connected(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);
      auto client = (sio::client *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;

      LUA->PushBool(client->opened());

      return 1;
    }
    int set_reconnect_delay(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);
   
      auto client = (sio::client *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto delay = static_cast<unsigned int>(LUA->CheckNumber(2));

      client->set_reconnect_delay(delay);

      return 0;
    }
    int set_reconnect_delay_max(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);

      auto client = (sio::client *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto delay_max = static_cast<unsigned int>(LUA->CheckNumber(2));

      client->set_reconnect_delay_max(delay_max);

      return 0;
    }
    int set_reconnect_attempts(lua_State *state) {
      LUA->CheckType(1, kGM_SOCKET_CLIENT_TYPE);

      auto client = (sio::client *)((GarrysMod::Lua::UserData *)LUA->GetUserdata(1))->data;
      auto attempts = static_cast<int>(LUA->CheckNumber(2));

      client->set_reconnect_attempts(attempts);

      return 0;
    }
  }
}
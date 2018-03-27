#include "client.hpp"
#include "socket.hpp"
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  void Client::Think(lua_State *state) {
    error_emitter_.Think(state);
    connect_emitter_.Think(state);
    reconnecting_emitter_.Think(state);
    disconnect_emitter_.Think(state);
    reconnect_emitter_.Think(state);
  }
  void Client::Dispose(lua_State *state) {
    error_emitter_.UnSet(state);
    connect_emitter_.UnSet(state);
    reconnecting_emitter_.UnSet(state);
    disconnect_emitter_.UnSet(state);
    reconnect_emitter_.UnSet(state);
  }
  void Client::Register(lua_State *state) {
    disconnect_emitter_.callback_wrapper([](lua_State *state, int callback_ref, std::tuple<sio::client::close_reason> args) {
      LUA->ReferencePush(callback_ref);

      switch (std::get<0>(args))
      {
      case sio::client::close_reason::close_reason_normal:
        LUA->PushString("normal");
        break;
      case sio::client::close_reason::close_reason_drop:
        LUA->PushString("dropped");
        break;
      default:
        LUA->PushNil();
        break;
      }

      LUA->Call(1, 0);
    });

    reconnect_emitter_.callback_wrapper([](lua_State *state, int callback_ref, std::tuple<unsigned int, unsigned int> args) {
      LUA->ReferencePush(callback_ref);
      LUA->PushNumber(std::get<0>(args));
      LUA->PushNumber(std::get<1>(args));
      LUA->Call(2, 0);
    });

    impl_.set_open_listener([this]() { connect_emitter_.Emit(); });
  }

  int Client::get_is_open(lua_State *state) {
    auto client = GetObject(state, 1);

    LUA->PushBool(client->impl_.opened());

    return 1;
  }
  int Client::set_reconnect_delay(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

    auto client = GetObject(state, 1);
    auto delay = static_cast<unsigned int>(abs(floor(LUA->GetNumber(2))));

    client->impl_.set_reconnect_delay(delay);

    return 0;
  }
  int Client::set_reconnect_attempts(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

    auto client = GetObject(state, 1);
    auto attempts = static_cast<int>(floor(LUA->GetNumber(2)));

    client->impl_.set_reconnect_attempts(attempts);

    return 0;
  }
  int Client::set_reconnect_delay_max(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::NUMBER);

    auto client = GetObject(state, 1);
    auto delay = static_cast<unsigned int>(abs(floor(LUA->GetNumber(2))));

    client->impl_.set_reconnect_delay_max(delay);

    return 0;
  }

  int Client::on(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
    LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

    auto client = LuaObject<Client>::GetObject(state, 1);
    auto event_name = std::string(LUA->GetString(2));

    if (event_name == "error") {
      client->error_emitter_.Set(state, 3);
    } else if (event_name == "connect") {
      client->connect_emitter_.Set(state, 3);
    } else if (event_name == "disconnect") {
      client->disconnect_emitter_.Set(state, 3);
    } else if (event_name == "reconnect") {
      client->reconnect_emitter_.Set(state, 3);
    } else if (event_name == "reconnecting") {
      client->reconnecting_emitter_.Set(state, 3);
    } else {
      // TODO: Throw error
    }

    return 0;
  }
  int Client::off(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

    auto client = LuaObject<Client>::GetObject(state, 1);
    auto event_name = std::string(LUA->GetString(2));

    if (event_name == "error") {
      client->error_emitter_.UnSet(state);
    } else if (event_name == "connect") {
      client->connect_emitter_.UnSet(state);
    } else if (event_name == "disconnect") {
      client->disconnect_emitter_.UnSet(state);
    } else if (event_name == "reconnect") {
      client->reconnect_emitter_.UnSet(state);
    } else if (event_name == "reconnecting") {
      client->reconnecting_emitter_.UnSet(state);
    } else {
      // TODO: Throw error
    }

    return 0;
  }
  int Client::off_all(lua_State *state) {
    auto client = LuaObject<Client>::GetObject(state, 1);

    client->error_emitter_.UnSet(state);
    client->connect_emitter_.UnSet(state);
    client->reconnecting_emitter_.UnSet(state);
    client->disconnect_emitter_.UnSet(state);
    client->reconnect_emitter_.UnSet(state);

    return 0;
  }
  int Client::socket(lua_State *state) { 
    auto client = LuaObject<Client>::GetObject(state, 1);
    auto nsp = std::string("");

    if (LUA->IsType(2, GarrysMod::Lua::Type::STRING))
      nsp = LUA->GetString(2);

    LUA->GetField(1, "_sockets");
    
    // Check if _sockets exists
    if (!LUA->IsType(-1, GarrysMod::Lua::Type::TABLE)) {
      // Create table
      LUA->CreateTable();
      LUA->SetField(1, "_sockets");
      // Push new _sockets table
      LUA->GetField(1, "_sockets");
    }

    // Push _sockets table copy for future access
    LUA->Push(-1);
    // Push socket ud
    LUA->GetField(-1, nsp.c_str());

    // Check if Socket exists
    if (LUA->IsType(-1, Socket::type_)) {
      int socket_reference = LUA->ReferenceCreate();

      // Pop table field and table copy
      LUA->Pop(2);
      LUA->ReferencePush(socket_reference);
      LUA->ReferenceFree(socket_reference);

      return 1;
    }

    // Pop table field (most likley nil)
    LUA->Pop();

    // Create and store socket in _sockets table
    auto socket = new Socket(state, client->impl_.socket(nsp));
    socket->Push(state);
    LUA->SetField(-2, nsp.c_str());

    // Push Socket
    socket->Push(state);

    return 1;
  }
  int Client::connect(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

    auto client = LuaObject<Client>::GetObject(state, 1);
    auto uri = std::string(LUA->GetString(2));
    std::map<std::string, std::string> query;
    std::map<std::string, std::string> http_extra_headers;

    client->impl_.connect(uri, query, http_extra_headers);

    return 0;
  }
  int Client::disconnect(lua_State *state) {
    auto client = LuaObject<Client>::GetObject(state, 1);

    client->impl_.close();

    return 0;
  }
}
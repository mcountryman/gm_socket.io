#include "engine.hpp"
#include "client.hpp"
#include "message.hpp"
#include "lua_object.hpp"
#include <algorithm>
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  int ThinkCallback(lua_State *state) {
    Engine::FromState(state)->Think(state);
    return 0;
  }
  int ShutdownCallback(lua_State *state) {
    Engine::FromState(state)->Shutdown(state);
    return 0;
  }

  void Engine::AddObject(LuaObjectBase *object) {
    std::unique_lock<std::mutex> lock(objects_mtx_);

    objects_.push_back(object);
  }
  bool Engine::CheckObject(LuaObjectBase *object) {
    std::unique_lock<std::mutex> lock(objects_mtx_);

    for (int i = 0; i < objects_.size(); i++)
      if (objects_[i] == object)
        return true;

    return false;
  }
  void Engine::RemoveObject(LuaObjectBase *object) {
    std::unique_lock<std::mutex> lock(objects_mtx_);

    for (int i = 0; i < objects_.size(); i++) {
      if (objects_[i] == object) {
        objects_.erase(objects_.begin() + i);
        return;
      }
    }
  }

  void Engine::Open(lua_State *state) {
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
      LUA->CreateTable();
        LUA->PushCFunction(LuaObject<Client>::PushNew);
        LUA->SetField(-2, "Client");

        LUA->CreateTable();
          LUA->PushCFunction(Message::to_null);
          LUA->SetField(-2, "to_null");
          LUA->PushCFunction(Message::to_array);
          LUA->SetField(-2, "to_array");
          LUA->PushCFunction(Message::to_object);
          LUA->SetField(-2, "to_object");
          LUA->PushCFunction(Message::to_string);
          LUA->SetField(-2, "to_string");
          LUA->PushCFunction(Message::to_binary);
          LUA->SetField(-2, "to_binary");
          LUA->PushCFunction(Message::to_integer);
          LUA->SetField(-2, "to_integer");
          LUA->PushCFunction(Message::to_double);
          LUA->SetField(-2, "to_double");
          LUA->PushCFunction(Message::to_boolean);
          LUA->SetField(-2, "to_boolean");
        LUA->SetField(-2, "Message");
      LUA->SetField(-2, "io");

      LUA->GetField(-1, "hook");
      LUA->GetField(-1, "Add");
        LUA->PushString("Tick");
        LUA->PushString("gm_socket_io::Think");
        LUA->PushCFunction(ThinkCallback);
        LUA->Call(3, 0);
      LUA->Pop();

      LUA->GetField(-1, "hook");
      LUA->GetField(-1, "Add");
        LUA->PushString("Shutdown");
        LUA->PushString("gm_socket_io::Shutdown");
        LUA->PushCFunction(ShutdownCallback);
        LUA->Call(3, 0);
      LUA->Pop();
    LUA->Pop();
  }
  void Engine::Think(lua_State *state) {
    for (auto object : objects_) {
      object->Think(state);
    }
  }

  void Engine::Shutdown(lua_State *state) {
  }

  std::map<lua_State*, std::shared_ptr<Engine>> Engine::state_map_;
  
  std::shared_ptr<Engine> Engine::FromState(lua_State *state) {
    if (state_map_.find(state) == state_map_.end()) {
      state_map_[state] = std::make_shared<Engine>();
    }

    return state_map_[state];
  }
}
#pragma once

#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include <cinttypes>

typedef struct lua_State;

namespace gm_socket_io {
  class LuaObjectBase;
  class Engine {
  private:
    std::mutex objects_mtx_;
    std::vector<LuaObjectBase*> objects_;
  public:
    void AddObject(LuaObjectBase *object);
    bool CheckObject(LuaObjectBase *object);
    void RemoveObject(LuaObjectBase *object);
  public:
    void Open(lua_State *state);
    void Think(lua_State *state);
    void Shutdown(lua_State *state);
  public:
    static std::map<lua_State*, std::shared_ptr<Engine>> state_map_;
  public:
    static std::shared_ptr<Engine> FromState(lua_State *state);
  };
}
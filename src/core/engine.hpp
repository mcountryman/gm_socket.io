#ifndef _GM_SOCKET_IO_CORE_ENGINE_HPP_
#define _GM_SOCKET_IO_CORE_ENGINE_HPP_

#include <map>
#include <queue>
#include <mutex>
#include <tuple>
#include <memory>
#include <functional>
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  class Engine {
  private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::queue<std::tuple<int, std::function<void(lua_State*, int)>>> m_callback_queue;
  public:
    Engine() {}
    ~Engine() {}
  public:
    void emit(int cb_ref, std::function<void(lua_State*, int)> cb);
    bool dequeue(std::tuple<int, std::function<void(lua_State*, int)>>* callback);
  private:
    static std::map<lua_State*, std::shared_ptr<Engine>> m_state_map;
  public:
    static std::shared_ptr<Engine> from_state(lua_State *state) {
      return m_state_map[state];
    }

  public: // Lua functions
    static int on_open(lua_State *state);
    static int on_close(lua_State *state);
  public: // Lua hook callbacks
    static int on_think(lua_State *state);
    static int on_shutdown(lua_State *state);
  };
}

#endif//_GM_SOCKET_IO_CORE_ENGINE_HPP_

/*
int on(lua_State *state) {
  auto client = Client::from_state(state);
  auto engine = Engine::from_state(state);

  CLuaFunc *cb = LUA->GetCFunction(1);

  client.on("connect", [client, engine](sio::socket *socket) {
    engine.emit([socket](lua_State *state, CLuaFunc *cb) {

    }, cb);
  });
}
*/
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
    std::mutex mutex_;
    std::condition_variable condition_;

    // Contains queue of callback wrappers and CFunc references
    std::queue<std::tuple<int, std::function<void(lua_State*, int)>>> callback_queue_;

  public:

    // Pushes CFunc reference and C++ wrapper fn to stack to be called on main thread
    void emit(int cfunc_reference, std::function<void(lua_State*, int)> callback_wrapper);

    // Dequeues CFunc reference / C++ wrapper fn in a thread safe manner
    bool dequeue(std::tuple<int, std::function<void(lua_State*, int)>>* callback_tuple);

  private:
    static std::map<lua_State*, std::shared_ptr<Engine>> state_map_;
  public:
    static std::shared_ptr<Engine> from_state(lua_State *state) {
      return state_map_[state];
    }

  public:

    // Called when library opens (internal use only)
    static int on_open(lua_State *state);

    // Called when library closed (internal use only)
    static int on_close(lua_State *state);

    // Called on Think hook (internal use only)
    static int on_think(lua_State *state);
  };
}

#endif//_GM_SOCKET_IO_CORE_ENGINE_HPP_
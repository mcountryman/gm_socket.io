#include "engine.hpp"
#include "../wrapper/client.hpp"

#include <chrono>

namespace gm_socket_io {
    std::map<lua_State*, std::shared_ptr<Engine>> Engine::state_map_;

    void Engine::emit(int cfunc_reference, std::function<void(lua_State*, int)> callback_wrapper) {
      std::lock_guard<std::mutex> lock(mutex_);
      callback_queue_.push(std::make_tuple(cfunc_reference, callback_wrapper));
      condition_.notify_one();
    }

    bool Engine::dequeue(std::tuple<int, std::function<void(lua_State*, int)>>* callback_tuple) {
      std::unique_lock<std::mutex> lock(mutex_);

      // Wait 10ms for unlock
      if (condition_.wait_for(lock, std::chrono::milliseconds(10)) == std::cv_status::timeout)
        return false;

      if (callback_queue_.empty())
        return false;

      // Store first -> pop first
      auto cb = callback_queue_.front();
      callback_queue_.pop();

      *callback_tuple = cb;

      return true;
    }

    int Engine::on_open(lua_State *state) {
      auto engine = std::make_shared<Engine>();

      LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
        client_wrapper::make(state);
          LUA->PushCFunction(client_wrapper::make);
          LUA->SetField(-2, "Client");
        LUA->SetField(-2, "io");

        LUA->GetField(-1, "hook");
        LUA->GetField(-1, "Add");
        LUA->PushString("Tick");
        LUA->PushString("CPP_SOCKET_IO_THINK");
        LUA->PushCFunction(Engine::on_think);
        LUA->Call(3, 0);
      LUA->Pop();

      // Map our engine to this state
      state_map_[state] = engine;

      return 0;
    }

    int Engine::on_close(lua_State *state) {
      auto engine = Engine::from_state(state);

      std::unique_lock<std::mutex> lock(engine->mutex_);

      // Wait indefinitely
      engine->condition_.wait(lock);

      while (engine->callback_queue_.empty()) {
        auto callback_tuple = engine->callback_queue_.front();

        // Free callback reference
        LUA->ReferenceFree(std::get<0>(callback_tuple));

        engine->callback_queue_.pop();
      }

      return 0;
    }

    int Engine::on_think(lua_State *state) {
      auto engine = Engine::from_state(state);

      // Limit to 100 callbacks per tick
      for (int i = 0; i < 100; i++) {
        std::tuple<int, std::function<void(lua_State*, int)>> callback;

        if (engine->dequeue(&callback)) {
          auto cfunc_reference = std::get<0>(callback);
          auto callback_wrapper = std::get<1>(callback);

          // Call wrapper (hopefully this will call our CFunc reference we supply)
          callback_wrapper(state, cfunc_reference);

          // Free callback reference
          LUA->ReferenceFree(cfunc_reference);
        } else {

          // Unable to dequeue element due to lock or queue being empty
          break;
        }
      }

      return 0;
    }
}
#include "engine.hpp"
#include "../wrapper/client.hpp"

#include <chrono>

namespace gm_socket_io {
    std::map<lua_State*, std::shared_ptr<Engine>> Engine::m_state_map;


    void Engine::emit(int cb_ref, std::function<void(lua_State*, int)> cb) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_callback_queue.push(std::make_tuple(cb_ref, cb));
      m_condition.notify_one();
    }
    bool Engine::dequeue(std::tuple<int, std::function<void(lua_State*, int)>>* callback) {
      std::unique_lock<std::mutex> lock(m_mutex);

      if (m_condition.wait_for(lock, std::chrono::milliseconds(10)) == std::cv_status::timeout)
        return false;
      if (m_callback_queue.empty())
        return false;

      auto cb = m_callback_queue.front();
      m_callback_queue.pop();

      *callback = cb;

      return true;
    }

    int Engine::on_open(lua_State *state) {
      auto engine = std::make_shared<Engine>();

      LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
        LUA->CreateTable();
          LUA->PushCFunction(ClientWrapper::make);
          LUA->SetField(-2, "Client");
        LUA->CreateTable();
          ClientWrapper::make(state);
          LUA->SetField(-2, "__index");
        LUA->SetMetaTable(-2);
        LUA->SetField(-2, "io");

        LUA->GetField(-1, "hook");
        LUA->GetField(-1, "Add");
        LUA->PushString("Tick");
        LUA->PushString("CPP_SOCKET_IO_THINK");
        LUA->PushCFunction(Engine::on_think);
        LUA->Call(3, 0);
      LUA->Pop();

      m_state_map[state] = engine;

      return 0;
    }
    int Engine::on_close(lua_State *state) {
      return 0;
    }
    int Engine::on_think(lua_State *state) {
      auto engine = Engine::from_state(state);

      for (int i = 0; i < 100; i++) {
        std::tuple<int, std::function<void(lua_State*, int)>> callback;

        if (engine->dequeue(&callback)) {
          auto cb_ref = std::get<0>(callback);
          auto cb = std::get<1>(callback);

          cb(state, cb_ref);
          LUA->ReferenceFree(cb_ref);
        } else {
          break;
        }
      }

      return 0;
    }
    int Engine::on_shutdown(lua_State *state) {
      auto engine = Engine::from_state(state);
      std::tuple<int, std::function<void(lua_State*, int)>> callback;
      
      while (engine->dequeue(&callback))
        LUA->ReferenceFree(std::get<0>(callback));

      return 0;
    }
}
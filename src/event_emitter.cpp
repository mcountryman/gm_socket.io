#include "event_emitter.hpp"
#include <algorithm>
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  void BaseEventEmitter::Set(lua_State *state, int position) {
    LUA->Push(position);
    callback_ref_ = LUA->ReferenceCreate();
  }
  void BaseEventEmitter::UnSet(lua_State *state) {
    if (callback_ref_ == -1)
      return;
    
    LUA->ReferenceFree(callback_ref_);
    callback_ref_ = -1;
  }

  bool BaseKeyedEventEmitter::getListeners(std::string event_name, std::vector<int> &listeners) {
    std::unique_lock<std::mutex> lock(listeners_mtx_);
    std::vector<int> listeners_copy;

    for (const auto &pair : listeners_)
      if (pair.first == event_name)
        listeners.push_back(pair.second);

    return true;
  }
  void BaseKeyedEventEmitter::AddListener(lua_State *state, std::string event_name, int position) {
    LUA->Push(position);
    int callback_ref = LUA->ReferenceCreate();

    std::unique_lock<std::mutex> lock(listeners_mtx_);
    listeners_[event_name] = callback_ref;
  }
  void BaseKeyedEventEmitter::ClearListeners(lua_State *state) {
    std::unique_lock<std::mutex> lock(listeners_mtx_);

    for (const auto &pair : listeners_) {
      LUA->ReferenceFree(pair.second);
    }

    listeners_.clear();
  }
  void BaseKeyedEventEmitter::RemoveListener(std::string event_name) {
    std::unique_lock<std::mutex> lock(listeners_mtx_);

    auto iter = listeners_.begin();
    while (iter != listeners_.end()) {
      if (iter->first == event_name) {
        listeners_.erase(iter++);
      } else {
        iter++;
      }
    }
  }

  void EmptyEventEmitter::Emit() { 
    ++event_count_; 
  }
  void EmptyEventEmitter::Think(lua_State *state) {
    if (callback_ref_ == -1)
      return;

    int max = std::min(event_count_.load(), 1000);

    for (int i = 0; i < max; i++) {
      LUA->ReferencePush(callback_ref_);
      LUA->Call(0, 0);

      --event_count_;
    }
  }
}
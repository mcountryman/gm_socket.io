#pragma once

#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <atomic>

typedef struct lua_State;

namespace gm_socket_io {
  class BaseEventEmitter {
  protected:
    std::atomic<int> callback_ref_ = -1;
  public:
    void Set(lua_State *state, int position = 1);
    void UnSet(lua_State *state);
  };

  class BaseKeyedEventEmitter {
  protected:
    std::map<std::string, int> listeners_;
    std::mutex                 listeners_mtx_;
  protected:
    bool getListeners(std::string event_name, std::vector<int> &listeners);
  public:
    void AddListener(lua_State *state, std::string event_name, int position = 1);
    void ClearListeners(lua_State *state);
    void RemoveListener(std::string event_name);
  };

  class EmptyEventEmitter : public BaseEventEmitter {
  protected:
    std::atomic<int> event_count_ = 0;
  public:
    void Emit();
    void Think(lua_State *state); 
  };

  template<typename... Args>
  class EventEmitter : public BaseEventEmitter {
  public:
    typedef std::function<void(lua_State*, int, std::tuple<Args...>)> WrapperFn;
  private:
    std::queue<std::tuple<Args...>> events_;
    std::mutex events_mtx_;
    WrapperFn callback_wrapper_;
  private:
    bool dequeueEvent(std::tuple<Args...> &event) {
      std::unique_lock<std::mutex> lock(events_mtx_);

      if (events_.empty())
        return false;

      event = events_.front();
      events_.pop();

      return true;
    }
  public:
    void callback_wrapper(WrapperFn value) { callback_wrapper_ = value; }
  public:
    void Emit(Args... args) {
      std::unique_lock<std::mutex> lock(events_mtx_);
      events_.push(std::make_tuple(args...));
    }
    void Think(lua_State *state) {
      if (callback_ref_ == -1)
        return;
      if (callback_wrapper_ == nullptr)
        return;

      for (int i = 0; i < 1000; i++) {
        std::tuple<Args...> event;

        if (!dequeueEvent(event))
          break;

        callback_wrapper_(state, callback_ref_, event);
      }
    }
  };

  template<typename... Args>
  class KeyedEventEmitter : public BaseKeyedEventEmitter {
  public:
    typedef std::function<void(lua_State*, int, std::tuple<Args...>)> WrapperFn;
  private:
    std::queue<std::tuple<std::string, std::tuple<Args...>>> events_;
    std::mutex events_mtx_;
    WrapperFn callback_wrapper_;
  private:
    bool dequeueEvent(std::tuple<std::string, std::tuple<Args...>> &event) {
      std::unique_lock<std::mutex> lock(events_mtx_);

      if (events_.empty())
        return false;

      event = events_.front();
      events_.pop();

      return true;
    }
  public:
    void callback_wrapper(WrapperFn value) { callback_wrapper_ = value; }
  public:
    void Emit(std::string event_name, Args... args) {
      std::unique_lock<std::mutex> lock(events_mtx_);
      events_.push(std::make_tuple(event_name, std::make_tuple(args...)));
    }
    void Think(lua_State *state) {
      if (callback_wrapper_ == nullptr)
        return;

      std::vector<int> listeners;
      std::tuple<std::string, std::tuple<Args...>> event;

      if (!dequeueEvent(event))
        return;
      if (!getListeners(std::get<0>(event), listeners))
        return Think(state);

      for (auto listener : listeners) {
        callback_wrapper_(state, listener, std::get<1>(event));
      }
    }
  };
}
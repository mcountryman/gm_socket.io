#include "socket.hpp"
#include "message.hpp"

namespace gm_socket_io {
  void Socket::Think(lua_State *state) {
    error_handler_.Think(state);
    message_handler_.Think(state);
  }
  void Socket::Dispose(lua_State *state) {
    error_handler_.UnSet(state);
    message_handler_.ClearListeners(state);
  }
  void Socket::Register(lua_State *state) {
    error_handler_.callback_wrapper([](lua_State *state, int callback_ref, std::tuple<sio::message::ptr> args) {

      LUA->ReferencePush(callback_ref);
      int count = Message::PushMessage(state, std::get<0>(args));
      LUA->Call(count, 0);
    });

    message_handler_.callback_wrapper([](lua_State *state, int callback_ref, std::tuple<std::string, sio::message::ptr, bool, sio::array_message::ptr> args) {
      int count = 2;
      sio::array_message::ptr ack_message = std::get<3>(args);

      LUA->ReferencePush(callback_ref);
      LUA->PushString(std::get<0>(args).c_str());
      count += Message::PushMessage(state, std::get<1>(args));
      LUA->PushBool(std::get<2>(args));

      for (int i = 0; ack_message->get_vector().size(); i++) {
        count += Message::PushMessage(state, ack_message->get_vector()[i]);
      }

      LUA->Call(count, 0);
    });

    impl_->on_error([this](sio::message::ptr const& message) {
      error_handler_.Emit(message);
    });
  }

  int Socket::get_namespace(lua_State *state) {
    auto socket = GetObject(state, 1);

    LUA->PushString(socket->impl_->get_namespace().c_str());

    return 1;
  }

  int Socket::on(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
    LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

    auto socket = GetObject(state, 1);
    auto event_name = std::string(LUA->GetString(2));

    if (event_name == "error") {
      socket->error_handler_.Set(state, 3);
    } else {
      socket->impl_->on(event_name, [socket](std::string name, sio::message::ptr message, bool need_ack, sio::message::list &ack_message) {
        socket->message_handler_.Emit(name, name, message, need_ack, ack_message.to_array_message());
      });
      socket->message_handler_.AddListener(state, event_name, 3);
    }

    return 0;
  }
  int Socket::off(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

    auto socket = GetObject(state, 1);
    auto event_name = std::string(LUA->GetString(2));

    if (event_name == "error") {
      socket->error_handler_.UnSet(state);
    } else {
      socket->impl_->off(event_name);
      socket->message_handler_.RemoveListener(event_name);
    }

    return 0;
  }
  int Socket::off_all(lua_State *state) {
    auto socket = GetObject(state, 1);

    socket->impl_->off_error();
    socket->impl_->off_all();
    socket->error_handler_.UnSet(state);
    socket->message_handler_.ClearListeners(state);

    return 0;
  }
  int Socket::emit(lua_State *state) {
    LUA->CheckType(2, GarrysMod::Lua::Type::STRING);

    auto socket = GetObject(state, 1);
    auto event_name = std::string(LUA->GetString(2));
    sio::message::list message_list;

    int argc = LUA->Top();
    for (int i = 3; i <= argc; i++) {
      sio::message::ptr message = Message::ToMessage(state, i);

      if (message != nullptr)
        message_list.push(message);
    }

    socket->impl_->emit(event_name, message_list);

    return 0;
  }
}
#ifndef _GM_SOCKET_IO_CORE_MESSAGE_HPP_
#define _GM_SOCKET_IO_CORE_MESSAGE_HPP_

#include <sio_message.h>

namespace gm_socket_io {
  namespace Message {
    int push_message(lua_State *state, sio::message::ptr message) {
      switch (message->get_flag()) {
        case sio::message::flag::flag_integer:
          LUA->PushNumber(message->get_int());
          break;
        case sio::message::flag::flag_double:
          LUA->PushNumber(message->get_double());
          break;
        case sio::message::flag::flag_string:
          LUA->PushString(message->get_string().c_str());
          break;
        case sio::message::flag::flag_binary:
          LUA->PushString(message->get_binary()->c_str());
          break;
        case sio::message::flag::flag_array:
          LUA->CreateTable();

          for (auto item : message->get_vector()) {
            push_message(state, item);
            LUA->Insert(-2);
          }

          break;
        case sio::message::flag::flag_object:
          LUA->CreateTable();

          for (auto pair : message->get_map()) {
            push_message(state, pair.second);
            LUA->SetField(-2, pair.first.c_str());
          }
          
          break;
        case sio::message::flag::flag_boolean:
          LUA->PushBool(message->get_bool());
          break;
        case sio::message::flag::flag_null:
          LUA->PushNil();
          break;

        default:
          LUA->PushNil();
          break;
      }

      return 1;
    }

    sio::message::ptr get_message_from_args(lua_State *state, int position = 1) {
      sio::message::ptr result = nullptr;

      if (LUA->IsType(position, GarrysMod::Lua::Type::NUMBER)) {
        double value = LUA->GetNumber(position);

        if (floor(value) == value) {
          result = sio::int_message::create(value);
        } else {
          result = sio::double_message::create(value);
        }
      } else if (LUA->IsType(position, GarrysMod::Lua::Type::STRING)) {
        result = sio::string_message::create(std::string(LUA->GetString(position)));
      } else if (LUA->IsType(position, GarrysMod::Lua::Type::TABLE)) {
        result = sio::null_message::create();

      } else if (LUA->IsType(position, GarrysMod::Lua::Type::BOOL)) {
        result = sio::bool_message::create(LUA->GetBool(position));
      } else if (LUA->IsType(position, GarrysMod::Lua::Type::NIL)) {
        result = sio::null_message::create();
      } else {
        // TODO: Handle this error
      }

      return result;
    }
  }
}

#endif//_GM_SOCKET_IO_CORE_MESSAGE_HPP_
#include "message.hpp"

namespace gm_socket_io {
  namespace message {
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

    bool is_array(lua_State *state, int position) {
      int max = 0;

      // Push table copy
      LUA->Push(position);
      // TODO: figure out why I need this
      LUA->PushNil();

      while (LUA->Next(-2)) {
        bool is_array = false;

        // Check key type
        if (LUA->IsType(-2, GarrysMod::Lua::Type::NUMBER)) {
          double key = LUA->GetNumber(-2);

          // Check if is integer
          if (floor(key) == key && key >= 1 && max < key) {
            max = key;
            is_array = true;
          }
        }

        // Pop value + key copy
        LUA->Pop(1);

        if (!is_array) {
          // Pop table copy and exit
          LUA->Pop(1);
          return false;
        }
      }

      // Pop table copy
      LUA->Pop(1);
 
      return max >= kGM_SOCKET_MIN_ARRAY_SIZE;
    }

    sio::message::ptr get_message_from_table(lua_State *state, int position, int depth) {
      auto message = sio::object_message::create();

      if (depth > kGM_SOCKET_MAX_TABLE_DEPTH) {
        return sio::null_message::create();
      }

      LUA->Push(position);
      LUA->PushNil();

      while (LUA->Next(-2) != 0) {
        LUA->Push(-2);

        auto key = LUA->CheckString(-1);
        auto value = get_message(state, -2, depth + 1);

        ((sio::object_message*)message.get())->insert(key, value);

        LUA->Pop(2);
      }

      LUA->Pop(1);

      return message;
    }

    sio::message::ptr get_message_from_array(lua_State *state, int position, int depth) {
      auto message = sio::array_message::create();

      if (depth > kGM_SOCKET_MAX_ARRAY_DEPTH) {
        return sio::null_message::create();
      }

      LUA->Push(position);
      LUA->PushNil();

      while (LUA->Next(-2) != 0) {
        ((sio::array_message*)message.get())->push(get_message(state, -1, depth + 1));

        LUA->Pop(1);
      }

      LUA->Pop(1);

      return message;
    }

    sio::message::ptr get_message(lua_State *state, int position, int depth) {
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

        if (is_array(state, position)) {
          result = get_message_from_array(state, position, depth);
        } else {
          result = get_message_from_table(state, position, depth);
        }

      } else if (LUA->IsType(position, GarrysMod::Lua::Type::BOOL)) {
        result = sio::bool_message::create(LUA->GetBool(position));
      } else if (LUA->IsType(position, GarrysMod::Lua::Type::NIL)) {
        result = sio::null_message::create();
      } else {
        result = sio::null_message::create();
      }

      return result;
    }
  }
}
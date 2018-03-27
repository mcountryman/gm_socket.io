#include "message.hpp"
#include <map>
#include <string>
#include <vector>
#include <GarrysMod/Lua/Interface.h>

namespace gm_socket_io {
  int get_size(lua_State *state, int position) {
    int size = 0;

    LUA->Push(position);
    LUA->PushNil();

    while (LUA->Next(-2) != 0) {
      if (LUA->IsType(-1, GarrysMod::Lua::Type::NUMBER)) {
        auto number = LUA->GetNumber(-1);

        if (floor(number) == number) {
          size++;

          LUA->Pop();
          continue;
        }
      }

      LUA->Pop(3);
      return -1;
    }
    
    LUA->Pop();
    return size;
  }
  sio::message::ptr get_array(lua_State *state, int position, int size) {
    auto message = std::static_pointer_cast<sio::array_message>(sio::array_message::create());

    LUA->Push(position);
    LUA->PushNil();

    for (int i = 1; i <= size; i++) {
      LUA->PushNumber(i);
      LUA->RawGet(-2);

      message->push(Message::ToMessage(state, -1));
    }

    LUA->Pop();

    return message;
  }
  sio::message::ptr get_object(lua_State *state, int position) {
    auto message = std::static_pointer_cast<sio::object_message>(sio::object_message::create());

    LUA->Push(position);
    LUA->PushNil();

    while (LUA->Next(-2) != 0) {
      LUA->Push(-2);
      LUA->Push(-2);

      message->insert(
        std::string(LUA->GetString(-2)),
        Message::ToMessage(state, -1)
      );

      LUA->Pop(3);
    }

    LUA->Pop();

    return message;
  }

  int Message::get(lua_State *state) {
    auto message = LuaObject<Message>::GetObject(state, 1);
    return PushMessage(state, message->impl_);
  }

  int Message::to_null(lua_State *state) {
    return (new Message(state, sio::null_message::create()))->Push(state);
  }
  int Message::to_array(lua_State *state) {
    LUA->CheckType(1, GarrysMod::Lua::Type::TABLE);

    auto size = get_size(state, 1);
    auto message = get_array(state, 1, size);

    return (new Message(state, message))->Push(state);
  }
  int Message::to_object(lua_State *state) {
    LUA->CheckType(1, GarrysMod::Lua::Type::TABLE);

    return (new Message(state, get_object(state, 1)))->Push(state);
  }
  int Message::to_string(lua_State *state) {
    auto string = std::string(LUA->CheckString(1));

    return (new Message(state, sio::string_message::create(string)))->Push(state);
  }
  int Message::to_binary(lua_State *state) {
    auto string = std::make_shared<std::string>(LUA->CheckString(1));

    return (new Message(state, sio::binary_message::create(string)))->Push(state);
  }
  int Message::to_integer(lua_State *state) {
    auto number = static_cast<int64_t>(floor(LUA->CheckNumber(1)));

    return (new Message(state, sio::int_message::create(number)))->Push(state);
  }
  int Message::to_double(lua_State *state) {
    auto number = LUA->CheckNumber(1);

    return (new Message(state, sio::double_message::create(number)))->Push(state);
  }
  int Message::to_boolean(lua_State *state) {
    LUA->CheckType(1, GarrysMod::Lua::Type::BOOL);

    return (new Message(state, sio::bool_message::create(LUA->GetBool(1))))->Push(state);
  }
  
  sio::message::ptr Message::ToMessage(lua_State *state, int position) {
    int type = LUA->GetType(position); 
    int size;
    double number;

    switch (type) {
    case GarrysMod::Lua::Type::NIL:
      return sio::null_message::create();
    case GarrysMod::Lua::Type::BOOL:
      return sio::bool_message::create(LUA->GetBool(position));
    case GarrysMod::Lua::Type::TABLE:
      size = get_size(state, position);

      if (size != -1)
        return get_array(state, position, size);
      else
        return get_object(state, position);

    case GarrysMod::Lua::Type::NUMBER:
      number = LUA->GetNumber(position);

      if (floor(number) == number)
        return sio::int_message::create(static_cast<int64_t>(number));
      else
        return sio::double_message::create(number);
    case GarrysMod::Lua::Type::STRING:
      return sio::string_message::create(LUA->GetString(position));
    case Message::type_:
      return LuaObject<Message>::GetObject(state, position)->impl_;
    default:
      return nullptr;
    }
  }

  int Message::PushMessage(lua_State *state, sio::message::ptr message) {
    std::vector<sio::message::ptr> array;
    std::map<std::string, sio::message::ptr> object;

    switch (message->get_flag()) {
    case sio::message::flag::flag_null:
      LUA->PushNil();
      return 1;
    case sio::message::flag::flag_array:
      array = message->get_vector();

      LUA->CreateTable();
      for (int i = 0; i < array.size(); i++) {
        LUA->PushNumber(i);
        PushMessage(state, array[i]);
        LUA->RawSet(-3);
      }

      return 1;
    case sio::message::flag::flag_object:
      object = message->get_map();

      LUA->CreateTable();

      for (const auto &pair : object) {
        PushMessage(state, pair.second);
        LUA->SetField(-2, pair.first.c_str());
      }

      return 1;
    case sio::message::flag::flag_string:
      LUA->PushString(message->get_string().c_str());
      return 1;
    case sio::message::flag::flag_binary:
      LUA->PushString(message->get_binary()->c_str());
      return 1;
    case sio::message::flag::flag_integer:
      LUA->PushNumber(message->get_int());
      return 1;
    case sio::message::flag::flag_double:
      LUA->PushNumber(message->get_double());
      return 1;
    case sio::message::flag::flag_boolean:
      LUA->PushBool(message->get_bool());
      return 1;
    }

    return 0;
  }

}
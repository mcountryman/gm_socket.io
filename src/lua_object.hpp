#pragma once

#include "engine.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <cinttypes>
#include <functional>
#include <GarrysMod/Lua/Interface.h>

typedef struct lua_State;

namespace gm_socket_io {
  class LuaObjectBase {
  public:
    bool disposed_;

    std::map<std::string, GarrysMod::Lua::CFunc> methods_;
    std::map<std::string, GarrysMod::Lua::CFunc> getters_;
    std::map<std::string, GarrysMod::Lua::CFunc> setters_;
  protected:
    void AddMethod(std::string name, GarrysMod::Lua::CFunc method) { methods_[name] = method; }
    void AddGetter(std::string name, GarrysMod::Lua::CFunc method) { getters_[name] = method; }
    void AddSetter(std::string name, GarrysMod::Lua::CFunc method) { setters_[name] = method; }
  public:
    virtual void Make(lua_State *state) {}
    virtual void Think(lua_State *state) {}
    virtual void Dispose(lua_State *state) {}
  };

  template<class T>
  class LuaObject : public LuaObjectBase {
  private:
    int reference_;
  public:
    LuaObject(lua_State *state) {
      auto engine = Engine::FromState(state);

      AddMethod("__gc", __gc);
      AddMethod("__tostring", __tostring);

      disposed_ = false;
      reference_ = -1;

      engine->AddObject(this);
    }
  public:
    static int __gc(lua_State *state) {
      auto object = LuaObject<T>::GetObject(state, 1);
      auto engine = Engine::FromState(state);

      if (!object)
        return 0;
      
      if (!object->disposed_) {
        object->disposed_ = true;
        object->Dispose(state);
        engine->RemoveObject((LuaObject*)object);
        delete object;
      }

      return 0;
    }
    static int __index(lua_State *state) {
      auto object = LuaObject<T>::GetObject(state, 1);

      // Get field from mt
      LUA->GetMetaTable(1);
      LUA->Push(2);
      LUA->RawGet(-2);

      // Check if mt contains value and return
      if (!LUA->IsType(-1, GarrysMod::Lua::Type::NIL))
        return 1;

      // Pop field value
      LUA->Pop();

      // Check if key is string
      if (!LUA->IsType(2, GarrysMod::Lua::Type::STRING))
        return 0;

      auto key = std::string(LUA->GetString(2));

      // Check if getter exists
      if (object->getters_.find(key) == object->getters_.end())
        return 0;

      // Call getter
      LUA->PushCFunction(object->getters_[key]);
      LUA->Push(1);
      LUA->Push(2);
      LUA->Call(2, 1);

      return 1;
    }
    static int __newindex(lua_State *state) {
      auto object = LuaObject<T>::GetObject(state, 1);

      // Check if key is string
      if (!LUA->IsType(2, GarrysMod::Lua::Type::STRING))
        return 0;

      auto key = std::string(LUA->GetString(2));

      // Check if setter exists
      if (object->setters_.find(key) != object->setters_.end()) {
        // Call setter
        LUA->PushCFunction(object->setters_[key]);
        LUA->Push(1);
        LUA->Push(2);
        LUA->Push(3);
        LUA->Call(3, 0);

        return 0;
      }

      // Rawset to metatable
      LUA->GetMetaTable(1);
      LUA->Push(2);
      LUA->Push(3);
      LUA->RawSet(-3);

      return 0;
    }
    static int __tostring(lua_State *state) {
      auto object = GetObject(state, 1);

      std::stringstream ss;
      ss << T::name_ << ": " << std::hex << object;

      LUA->PushString(ss.str().c_str());

      return 1;
    }
  public:
    int Push(lua_State *state) {
      if (reference_ == -1) {
        GarrysMod::Lua::UserData *ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
        ud->type = T::type_;
        ud->data = this;
        LUA->CreateTable();
        LUA->PushCFunction(__index);
        LUA->SetField(-2, "__index");
        LUA->PushCFunction(__newindex);
        LUA->SetField(-2, "__newindex");

        for (const auto &pair : methods_) {
          LUA->PushCFunction(pair.second);
          LUA->SetField(-2, pair.first.c_str());
        }

        LUA->SetMetaTable(-2);
        reference_ = LUA->ReferenceCreate();
      }

      LUA->ReferencePush(reference_);

      return 1;
    }
  public:
    static int PushNew(lua_State *state) {
      auto object = new T(state);
      object->Push(state);

      return 1;
    }
    static T* GetObject(lua_State *state, int position = 1) {
      LUA->CheckType(position, T::type_);

      return (T*)((GarrysMod::Lua::UserData *)LUA->GetUserdata(position))->data;
    }

  };
}
//
//  sb_lua_context.h
//  sr-osx
//
//  Created by Andrey Kunitsyn on 8/13/13.
//  Copyright (c) 2013 Andrey Kunitsyn. All rights reserved.
//

#ifndef __sr_osx__sb_lua_context__
#define __sr_osx__sb_lua_context__

#include "luabind/sb_luabind_ref.h"
#include "luabind/impl/sb_luabind_wrapper_base.h"
#include "luabind/sb_luabind_stack.h"
#include "sb_notcopyable.h"
#include <sbstd/sb_traits.h>
#include <sbstd/sb_intrusive_ptr.h>

namespace Sandbox {
    
    class LuaContext : public luabind::impl::wrapper_base {
    private:
        lua_State* get_state_with_table_on_top(const char* path);
        lua_State* get_state_with_array_on_top();
        void set_value_on_top_of_stack_to_table(lua_State* L);
    public:
        explicit LuaContext();
        
        using luabind::impl::wrapper_base::call;
        using luabind::impl::wrapper_base::call_self;
        
        sb::intrusive_ptr<LuaContext> Create();
        sb::intrusive_ptr<LuaContext> CreateInherited();
        sb::intrusive_ptr<LuaContext> CreateChild(const char* name);
        
        bool IsSame(const sb::intrusive_ptr<LuaContext>& o);
        
        template <class T>
        inline T GetValue(const char* path) {
            lua_State* L = get_state_with_table_on_top(path);
            if (!L) return T();
            LUA_CHECK_STACK(-2)
            lua_gettable(L,-2);
            T val = luabind::stack<T>::get(L,-1);
            lua_pop(L,2);
            return val;
        }
        
        template <class T>
        inline T GetValueRaw(const char* name) {
            lua_State* L = GetVM();
            LUA_CHECK_STACK(0)
            GetObject(L);
            lua_getfield(L, -1, name);
            T val = luabind::stack<T>::get(L,-1);
            lua_pop(L,2);
            return val;
        }

        template <class T>
        inline void SetValue( const char* path, const sb::shared_ptr<T>& t ) {
            lua_State* L = get_state_with_table_on_top(path);
            LUA_CHECK_STACK(-2)
            if (!L) return;
            luabind::stack<sb::shared_ptr<T> >::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        template <class T>
        inline void SetValue( const char* path, typename sb::type_traits<T>::parameter_type t ) {
            lua_State* L = get_state_with_table_on_top(path);
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<T>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        template <class T>
        inline void SetValue( const char* path, T t ) {
            lua_State* L = get_state_with_table_on_top(path);
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<T>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        inline void SetValue( const char* path, const char* t ) {
            lua_State* L = get_state_with_table_on_top(path);
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<const char*>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        template <class T>
        inline void SetValue( const char* path, T* t ) {
            lua_State* L = get_state_with_table_on_top(path);
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<T*>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        
        
        template <class T>
        inline void AddValue(  const sb::shared_ptr<T>& t ) {
            lua_State* L = get_state_with_array_on_top();
            LUA_CHECK_STACK(-2)
            if (!L) return;
            luabind::stack<sb::shared_ptr<T> >::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        template <class T>
        inline void AddValue(  typename sb::type_traits<T>::parameter_type t ) {
            lua_State* L = get_state_with_array_on_top();
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<T>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        template <class T>
        inline void AddValue(  T t ) {
            lua_State* L = get_state_with_array_on_top();
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<T>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        inline void AddValue(  const char* t ) {
            lua_State* L = get_state_with_array_on_top();
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<const char*>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }
        template <class T>
        inline void AddValue(  T* t ) {
            lua_State* L = get_state_with_array_on_top();
            if (!L) return;
            LUA_CHECK_STACK(-2)
            luabind::stack<T*>::push(L,t);
            set_value_on_top_of_stack_to_table(L);
        }

    };
    typedef sb::intrusive_ptr<LuaContext> LuaContextPtr;
    namespace luabind {
        template <>
        struct stack<LuaContextPtr> : stack_lua_object_ptr<LuaContext> {};
    }
}

#endif /* defined(__sr_osx__sb_lua_context__) */

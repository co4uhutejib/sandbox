//
//  sb_luabind_stack.cpp
//  YinYang
//
//  Created by Андрей Куницын on 7/8/12.
//  Copyright (c) 2012 AndryBlack. All rights reserved.
//

#include "sb_luabind_stack.h"

#include "sb_inplace_string.h"

namespace Sandbox {
    namespace luabind {
        
        int lua_traceback (lua_State *L)
        {
            // look up Lua's 'debug.traceback' function
            lua_getglobal(L, "debug");
            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                return 1;
            }
            lua_getfield(L, -1, "traceback");
            if (!lua_isfunction(L, -1))
            {
                lua_pop(L, 2);
                return 1;
            }
            if (lua_isstring(L,1))
                lua_pushvalue(L, 1);  /* pass error message */
            else 
                lua_pushstring(L, "error");
            lua_pushinteger(L, 2);  /* skip this function and traceback */
            lua_call(L, 2, 1);  /* call debug.traceback */
            return 1;
        }
        
        void lua_argerror( lua_State* L, int idx, 
                          const char* expected, 
                          const char* got ) {
            char buf[128];
            if ( !got ) {
                got = lua_typename( L, lua_type(L,idx) );
            }
            ::snprintf(buf, 128, "invalid argument %d : got '%s' , expected '%s'\n", idx,got,expected);
            sb::string str = buf;
            int top = lua_gettop(L);
            lua_getglobal(L, "tostring");
            
            for (int i=1;i<=top;i++) {
                str+=lua_typename(L, lua_type(L,i) );
                
                const char *s;
                lua_pushvalue(L, -1);  /* function to be called */
                lua_pushvalue(L, i);   /* value to print */
                lua_call(L, 1, 1);
                s = lua_tostring(L, -1);  /* get result */
                if (s != NULL) {
                    str+="(";
                    str+=s;
                    str+=")";
                }
                str+=",";
                lua_pop(L, 1);
            }
            lua_pop(L, 1); // func
            str+="\n";
            lua_pushstring(L, str.c_str());
            lua_error(L);
        }
        
        void lua_access_error( lua_State* L, int idx, 
                              const char* got ) {
            char buf[128];
            ::snprintf(buf, 128, "invalid argument access %d : '%s' is const", idx,got);
            lua_pushstring(L, buf);
            lua_error(L);
        }
        
        void lua_unregistered_error( lua_State* L, 
                                    const char* type ) {
            char buf[128];
            ::snprintf(buf, 128, "unregistered type '%s'", type);
            lua_pushstring(L, buf);
            lua_error(L);
        }
        
        static void get_table(lua_State* L,const InplaceString& name) {
            const char* other = name.find(':');
            sb::string first = InplaceString(name.begin(),other).str();
            lua_getglobal(L,first.c_str());
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                lua_unregistered_error( L ,name.begin() );
                return;
            }
            if (*other==0) return;
            other+=2;
            InplaceString tail(other,name.end());
            while (!tail.empty()) {
                other = tail.find(':');
                InplaceString head( tail.begin(), other );
                lua_getfield(L, -1, head.str().c_str() );
                if (!lua_istable(L, -1)) {
                    lua_pop(L, 2);
                    lua_unregistered_error( L ,name.begin() );
                    return;
                }
                lua_remove(L, -2);
                if (*other==0) break;
                tail = InplaceString(other+2,name.end());
            }
        }
        template <char C, size_t count>
        static void get_create_table_impl(lua_State* L,const InplaceString& name) {
            const char* other = name.find(C);
            sb::string first = InplaceString(name.begin(),other).str();
            lua_getglobal(L,first.c_str());
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_pushvalue(L, -1);
                lua_setglobal(L, first.c_str());
            }
            if (*other==0) return;
            other+=count;
            if (other >= name.end()) return;
            InplaceString tail(other,name.end());
            while (!tail.empty()) {
                other = tail.find(C);
                InplaceString head( tail.begin(), other );
                lua_getfield(L, -1, head.str().c_str() );
                if (!lua_istable(L, -1)) {
                    lua_pop(L, 1);
                    lua_newtable(L);
                    lua_pushvalue(L, -1);
                    lua_setfield(L, -3, head.str().c_str());
                }
                lua_remove(L, -2);
                if (*other==0) break;
                tail = InplaceString(other+count,name.end());
            }
        }
        
        void lua_get_create_table(lua_State* L,const char* name) {
            get_create_table_impl<':',2>(L, InplaceString(name));
        }
        
        void lua_set_metatable( lua_State* L, const data_holder& holder ) {
            if (lua_isnil(L, -1))
                return;
            get_table( L, InplaceString(holder.info->name) );   /// obj mntbl
            sb_assert(lua_istable(L, -1));
            lua_pushliteral(L, "metatable");
            lua_rawget(L, -2);                          /// obj mntbl mt
            sb_assert(lua_istable(L, -1));
            lua_setmetatable(L, -3);                    /// obj mntbl
            lua_pop(L, 1);
        }
        
        void lua_set_value( lua_State* L, const char* path ) {
            InplaceString full_path(path);
            const char* name = full_path.rfind('.');
            if ( name == path ) {
                lua_setglobal(L, path);
            } else {
                get_create_table_impl<'.',1>(L, InplaceString(path,name));  /// val tbl
                lua_pushvalue(L, -2);               /// val tbl val
                lua_setfield(L, -2, name+1);        /// val tbl
                lua_pop(L, 2);
            }
        }
        
        void stack<char>::push( lua_State* L, char val ) {
            lua_pushlstring(L, &val, 1);
        }
        
        bool stack<bool>::get( lua_State* L, int idx ) {
            return lua_toboolean(L, idx);
        }
        
        void stack<bool>::push( lua_State* L, bool val ) {
            lua_pushboolean(L, val ? 1:0);
        }
        
        char stack<char>::get( lua_State* L, int idx ) {
            if ( lua_isnumber( L, idx ) ) {
                return char( lua_tointeger(L, idx) );
            } else if ( lua_isstring(L, idx ) ) {
                return *lua_tostring(L, idx);
            }
            lua_argerror(L, idx, "char", 0);
            return 0;
        }
        
        void stack<long>::push( lua_State* L, long val) {
            lua_pushnumber(L, val);
        }
        
        long stack<long>::get( lua_State* L, int idx ) {
            return long(lua_tonumber(L, idx));
        }
        
        void stack<short>::push( lua_State* L, short val) {
            lua_pushnumber(L, val);
        }
        
        short stack<short>::get( lua_State* L, int idx ) {
            return short(lua_tonumber(L, idx));
        }
        
        void stack<int>::push( lua_State* L, int val) {
            lua_pushinteger(L, val);
        }
        
        int stack<int>::get( lua_State* L, int idx ) {
            return int(lua_tointeger(L, idx));
        }
        
        void stack<float>::push( lua_State* L, float val) {
            lua_pushnumber(L, val);
        }
        
        float stack<float>::get( lua_State* L, int idx ) {
            return float(lua_tonumber(L, idx));
        }
        
        void stack<unsigned char>::push( lua_State* L, unsigned char val) {
            lua_pushunsigned(L, val);
        }
        
        unsigned char stack<unsigned char>::get( lua_State* L, int idx ) {
            return (unsigned char)lua_tounsigned(L, idx);
        }
        
        void stack<unsigned long>::push( lua_State* L, unsigned long val) {
            lua_pushnumber(L, val);
        }
        
        unsigned long stack<unsigned long>::get( lua_State* L, int idx ) {
            return (unsigned long)lua_tonumber(L, idx);
        }
        
        void stack<unsigned short>::push( lua_State* L, unsigned short val) {
            lua_pushunsigned(L, val);
        }
        
        unsigned short stack<unsigned short>::get( lua_State* L, int idx ) {
            return (unsigned short)lua_tounsigned(L, idx);
        }
        
        void stack<unsigned int>::push( lua_State* L, unsigned int val) {
            lua_pushunsigned(L, val);
        }
        
        unsigned int stack<unsigned int>::get( lua_State* L, int idx ) {
            return (unsigned int)lua_tounsigned(L, idx);
        }
        
        void stack<const char*>::push( lua_State* L, const char* val) {
            lua_pushstring(L, val);
        }
        
        const char* stack<const char*>::get( lua_State* L, int idx ) {
            return lua_tostring(L, idx);
        }
        
        
        stack_cleaner::stack_cleaner( lua_State* L ) : m_L(L) {
            m_top = lua_gettop(L);
        }
        
        stack_cleaner::~stack_cleaner() {
            int cnt = lua_gettop(m_L) - m_top;
            sb_assert( cnt>=0 );
            if (cnt > 0 ) {
                lua_pop(m_L, cnt);
            }
        }

        
        
        
        static int destructor_func( lua_State* L ) {
            if ( lua_isuserdata(L, 1 ) ) {
                data_holder* holder = reinterpret_cast<data_holder*>(lua_touserdata(L, 1));
                if ( holder->destructor ) {
                    holder->destructor( holder+1 );
                }
            }
            return 0;
        }
        
        
        
        static int getter_indexer (lua_State *L)
		{
            sb_assert(lua_isuserdata(L, 1));
            /// debug
#if 0
            data_holder* holder = reinterpret_cast<data_holder*>(lua_touserdata(L, 1));
            if (strcmp(holder->info->name,"Sandbox::Chipmunk::TransformAdapter")==0) {
                int a = 0;
            }
#endif
          	lua_getmetatable(L, 1);             /// mt
            do {
                lua_pushliteral(L, "props");
                lua_rawget(L, -2);                  /// mt props
                sb_assert(lua_istable(L, -1));
                
                // Look for the key in the metatable
				lua_pushvalue(L, 2);
				lua_rawget(L, -2);              /// mt props res
				// Did we get a non-nil result?  If so, return it
				if (!lua_isnil(L, -1)) {
                    lua_remove(L, -2);          /// mt res
                    lua_remove(L, -2);          /// res
                    lua_getfield(L, -1, "get"); /// res get
                    sb_assert(lua_isfunction(L, -1));
                    lua_remove(L, -2);          /// get
                    lua_pushvalue(L, 1);        /// get obj
                    lua_call(L, 1, 1);
                    return 1;
                }
				lua_pop(L, 2);                  /// mt 
                
                lua_pushliteral(L, "methods");
                lua_rawget(L, -2);                  /// mt methods
                sb_assert(lua_istable(L, -1));
                
                // Look for the key in the metatable
				lua_pushvalue(L, 2);
				lua_rawget(L, -2);              /// mt methods res
				// Did we get a non-nil result?  If so, return it
				if (!lua_isnil(L, -1)) {
                    lua_remove(L, -2);          /// mt res
                    lua_remove(L, -2);          /// res
                    sb_assert(lua_isfunction(L, -1));
                    return 1;
                }
				lua_pop(L, 2);                  /// mt 
                
                
				lua_pushliteral(L, "parent");
                lua_rawget(L, -2 );             /// mt prnt
				if (lua_isnil(L, -1)) {
					lua_pop(L, 2);
                    char buf[128];
                    lua_getglobal(L, "tostring");
                    lua_pushvalue(L, 1);
                    lua_call(L, 1, 1);
                    snprintf(buf, 128, "%s: get unknown field: %s",lua_tostring(L, -1),lua_tostring(L, 2));
                    lua_pop(L, 1);
                    lua_pushstring(L, buf);
                    lua_error(L);
                    return 0;
                }
				lua_remove(L, -2);              /// prnt
			} while (true);
			
			// Control never gets here
			return 0;
		}
        
        static int setter_indexer (lua_State *L)
		{
            sb_assert(lua_isuserdata(L, 1));
          	lua_getmetatable(L, 1);             /// mt
            do {
                lua_pushliteral(L, "props");
                lua_rawget(L, -2);                  /// mt props
                sb_assert(lua_istable(L, -1));
                
                // Look for the key in the metatable
				lua_pushvalue(L, 2);
				lua_rawget(L, -2);              /// mt props res
				// Did we get a non-nil result?  If so, return it
				if (!lua_isnil(L, -1)) {
                    lua_remove(L, -2);          /// mt res
                    lua_remove(L, -2);          /// res
                    lua_getfield(L, -1, "set"); /// res set
                    sb_assert(lua_isfunction(L, -1));
                    lua_remove(L, -2);          /// set
                    lua_pushvalue(L, 1);        /// set obj
                    lua_pushvalue(L, 3);        /// set obj val
                    lua_call(L, 2, 0);
                    return 0;
                }
				lua_pop(L, 2);                  /// mt 
				lua_pushliteral(L, "parent");
                lua_rawget(L, -2 );             /// mt prnt
				if (lua_isnil(L, -1)) {
					lua_pop(L, 2);
                    char buf[128];
                    snprintf(buf, 128, "%s: set unknown field: %s",lua_tostring(L, 1),lua_tostring(L, 2));
                    lua_pushstring(L, buf);
                    lua_error(L);
                    return 0;
                }
				lua_remove(L, -2);              /// prnt
			} while (true);
			
			// Control never gets here
			return 0;
		}
        
        static int object_to_string( lua_State* L ) {
            if (lua_isuserdata(L, 1)) {
                data_holder* holder = reinterpret_cast<data_holder*>(lua_touserdata(L, 1));
                void* res = 0;
                if ( holder->type == data_holder::raw_data ) {
                    res = holder+1;
                } else if ( holder->type == data_holder::raw_ptr ) {
                    res = *reinterpret_cast<void**>(holder+1);
                } else if ( holder->type == data_holder::shared_ptr ) {
                    res = *reinterpret_cast<void**>(holder+1); // hack
                }  
                char buf[128];
                snprintf(buf, 128, "object<%s>:%p",holder->info->name,res);
                lua_pushstring(L, buf);
                return 1;
            }
            return 0;
        }
        
        void lua_create_metatable(lua_State* L,const meta::type_info* info) {
            lua_get_create_table( L, info->name );   /// mntbl
            sb_assert( lua_istable(L, -1));   
            lua_createtable(L, 0, 0);                     /// mntbl props
            lua_setfield(L, -2, "props");                 /// mntbl 
            lua_createtable(L, 0, 0);                     /// mntbl props
            lua_setfield(L, -2, "methods");                 /// mntbl 
            if (info->parent && info->parent!=meta::type<void>::info() ) {
                lua_get_create_table(L,info->parent->name);
                lua_setfield(L, -2, "parent");
            }
            lua_createtable(L, 0, 7);                     /// mntbl raw_ptr
            lua_pushcfunction(L, &destructor_func);               /// mntbl raw_ptr destructor_func
			lua_setfield(L, -2, "__gc");                  /// mntbl raw_ptr
            lua_pushcfunction(L, &getter_indexer);               /// mntbl raw_ptr indexer
			lua_setfield(L, -2, "__index");               /// mntbl raw_ptr
            lua_pushcfunction(L, &setter_indexer);               /// mntbl raw_ptr indexer
			lua_setfield(L, -2, "__newindex");               /// mntbl raw_ptr
            lua_pushcfunction(L, &object_to_string);               /// mntbl raw_ptr object_to_string
			lua_setfield(L, -2, "__tostring");               /// mntbl raw_ptr
            lua_getfield(L, -2, "props");                 /// mntbl raw_ptr props
            lua_setfield(L, -2, "props");                 /// mntbl raw_ptr
            lua_getfield(L, -2, "methods");                 /// mntbl raw_ptr methods
            lua_setfield(L, -2, "methods");                 /// mntbl raw_ptr
            lua_getfield(L, -2, "parent");                 /// mntbl raw_ptr parent
            lua_setfield(L, -2, "parent");                 /// mntbl raw_ptr
            lua_setfield(L, -2, "metatable");               /// mntbl 
            lua_pushvalue(L, -1);
            lua_setmetatable(L, -2);
        }

    }
}

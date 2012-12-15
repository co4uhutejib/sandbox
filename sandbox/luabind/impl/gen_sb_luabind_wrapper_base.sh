#!/bin/sh

GUARD="SB_LUABIND_WRAPPER_BASE_H_INCLUDED"
MAXARGS=8

echo "#ifndef $GUARD" 
echo "#define $GUARD" 
echo
echo "/**"
echo "	generated by $0 at $(date)"
echo "*/"
echo "#include \"../sb_luabind_stack.h\""
echo "namespace Sandbox {"
echo "	namespace luabind { namespace impl {"
echo "		template <class Holder>"
echo "		class wrapper_base {"
echo "		protected:"
echo "			Holder	m_self;"
echo "		protected:"
for (( args=0; args<=$MAXARGS; args++ ))
do
	echo "			// implementation for $args args"
	tmpl_args=""
	func_args=""
	call_args=""
	for (( i=1; i<=$args; i++ ))
	do
		if [ "$func_args""x" != "x" ]; then
			func_args="$func_args,\n						"
			call_args="$call_args\n				"
			tmpl_args="$tmpl_args,\n				"
		fi
		tmpl_args="$tmpl_args""typename A$i"
		func_args="$func_args""typename sb::type_traits<A$i>::parameter_type a$i"
		call_args="$call_args""stack<A$i>::push(L,a$i);"
	done
	
	if [ "$args" == "0" ]; then
		echo "			// void method void"
		echo "			void call(const char* name){"
		echo "				lua_State* L = m_self.GetVM();"
		echo "				m_self.push(L);"
		echo "				lua_pushstring(L,name);"
		echo "				lua_gettable(L,-2);"
		echo "				lua_remove(L,-2);"
		echo "				if (!lua_isfunction(L,-1)) {"
		echo "					sb_assert(false);"
		echo "					return;"
		echo "				}"
		echo "				m_self.push(L);"
		echo "				lua_call_method(L,1,0,name);"
		echo "			}"

		echo "			// res method void"
		echo "			template<class R>"
		echo "			R call(const char* name,R* = 0){"
		echo "				lua_State* L = m_self.GetVM();"
		echo "				m_self.push(L);"
		echo "				lua_pushstring(L,name);"
		echo "				lua_gettable(L,-2);"
		echo "				lua_remove(L,-2);"
		echo "				if (!lua_isfunction(L,-1)) {"
		echo "					sb_assert(false);"
		echo "					return R();"
		echo "				}"
		echo "				m_self.push(L);"
		echo "				lua_call_method(L,1,1,name);"
		echo "				R res = stack<R>::get(L,-1);"
		echo "				lua_pop(L,1);"
		echo "				return res;"
		echo "			}"
	else
		echo "			// void method void"
		echo "			template<$tmpl_args>"
		echo "			void call(const char* name,$func_args){"
		echo "				lua_State* L = m_self.GetVM();"
		echo "				m_self.push(L);"
		echo "				lua_pushstring(L,name);"
		echo "				lua_gettable(L,-2);"
		echo "				lua_remove(L,-2);"
		echo "				if (!lua_isfunction(L,-1)) {"
		echo "					sb_assert(false);"
		echo "					return;"
		echo "				}"
		echo "				m_self.push(L);"
		echo "				$call_args";
		echo "				lua_call_method(L,1+$args,0,name);"
		echo "			}"

		echo "			// res method void"
		echo "			template<class R,$tmpl_args>"
		echo "			R call(const char* name,$func_args,R* = 0){"
		echo "				lua_State* L = m_self.GetVM();"
		echo "				m_self.push(L);"
		echo "				lua_pushstring(L,name);"
		echo "				lua_gettable(L,-2);"
		echo "				lua_remove(L,-2);"
		echo "				if (!lua_isfunction(L,-1)) {"
		echo "					sb_assert(false);"
		echo "					return R();"
		echo "				}"
		echo "				m_self.push(L);"
		echo "				$call_args";
		echo "				lua_call_method(L,1+$args,1,name);"
		echo "				R res = stack<R>::get(L,-1);"
		echo "				lua_pop(L,1);"
		echo "				return res;"
		echo "			}"
	fi
	
	
done
echo "		};"
echo "	} }"
echo "}"
echo "#endif /*$GUARD*/" 
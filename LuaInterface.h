#ifndef MY_LUA_INTERFACE_H
#define MY_LUA_INTERFACE_H

#include<vector>
extern "C"
{
#include "lua542/include/lua.h";
#include "lua542/include/lauxlib.h"
#include "lua542/include/lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua542/liblua54.a")
#endif
#ifdef _WIN64
#pragma comment(lib, "lua542/liblua54.a")
#endif

using namespace std;
#include<string>
class LuaInterface
{
	public:
		// ID reference for the Vertex Array Object
		lua_State* L;
		
		bool CheckLua(lua_State* L, int r);

		LuaInterface();

		void Close();

		void AddFunction(string name, lua_CFunction func);
};
#endif
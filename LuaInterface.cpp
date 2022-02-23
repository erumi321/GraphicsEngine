#include"LuaInterface.h"
#include<Windows.h>

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

//Check if lua has no syntax issues
bool LuaInterface::CheckLua(lua_State* L, int r)
{
	if (r != LUA_OK) {
		std::string errormsg = lua_tostring(L, -1);
		std::string lineBr = "\n";
		std::string output = lineBr + errormsg + lineBr;
		OutputDebugStringA(output.c_str());
		return false;
	}
	return true;
}

//Init lua
LuaInterface::LuaInterface()
{
	L = luaL_newstate();
	luaL_openlibs(L);
}

void LuaInterface::Close()
{
	lua_close(L);
}

//Expose function to lua wusing the name of the first paramter
void LuaInterface::AddFunction(string name, lua_CFunction func)
{
	lua_register(L, name.c_str(), func);
}


#include"GUIButton.h"
#include<Windows.h>
#include <typeinfo>
#include"LuaInterface.h"
using namespace std;
GUIButton::GUIButton(float x, float y, float width, float height, float r, float g, float b, string onClickName)
	: x(x)
	, y(y)
	, width(width)
	, height(height)
	, r(r)
	, g(g)
	, b(b)
	, onClickName(onClickName) {
}

void GUIButton::CheckClick(float inpX, float inpY)
{
	if (inpX >= x - (width / 2) && inpX <= x + (width / 2) && inpY >= y - (height / 2) && inpY <= y + (height / 2))
	{
		DoClick();
	}
}

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

extern LuaInterface l;

void GUIButton::DoClick()
{
	lua_getglobal(l.L, onClickName.c_str());

	if (lua_isfunction(l.L, -1))
	{

		lua_newtable(l.L);
		lua_pushinteger(l.L, x);
		lua_setfield(l.L, -2, "x");
		lua_pushinteger(l.L, y);
		lua_setfield(l.L, -2, "y");
		lua_pushinteger(l.L, width);
		lua_setfield(l.L, -2, "width");
		lua_pushinteger(l.L, height);
		lua_setfield(l.L, -2, "height");
		lua_call(l.L, 1, 0);
	}
	else
	{
		string s = "no";
		std::string lineBr = "\n";
		std::string output = lineBr + s + lineBr;
		OutputDebugStringA(output.c_str());
	}
}

float GUIButton::GetDrawParameters()
{
	return x, y, width, height, r, g, b;
}
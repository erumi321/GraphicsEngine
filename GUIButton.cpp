#include"GUIButton.h"
#include<Windows.h>
#include <typeinfo>
#include"LuaInterface.h"
#include<string>
#include<sstream>
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

//Get the LuaInterface created in main.cpp
extern LuaInterface l;

//Debug Purposes
void _myPrint(string s)
{
	std::string lineBr = "\n";
	std::string output = lineBr + s + lineBr;
	OutputDebugStringA(output.c_str());
}
void GUIButton::DoClick()
{
	//get the lua function named when button inited
	lua_getglobal(l.L, onClickName.c_str());

	if (lua_isfunction(l.L, -1))
	{
		//create table of button x, y, width, hiehgt, and id
		lua_newtable(l.L);
		lua_pushinteger(l.L, x);
		lua_setfield(l.L, -2, "x");
		lua_pushinteger(l.L, y);
		lua_setfield(l.L, -2, "y");
		lua_pushinteger(l.L, width);
		lua_setfield(l.L, -2, "width");
		lua_pushinteger(l.L, height);
		lua_setfield(l.L, -2, "height");
		lua_pushstring(l.L, id.c_str());
		lua_setfield(l.L, -2, "ID");

		//call function passing it the table as the only argument
		lua_call(l.L, 1, 0);
	}
	else
	{
		string s = "Button click: Not a lua function";
		std::string lineBr = "\n";
		std::string output = lineBr + s + lineBr;
		OutputDebugStringA(output.c_str());
	}
}
#include"GUIButton.h"
#include<Windows.h>
#include <typeinfo>
#include"LuaInterface.h"
#include<map>
#include<string>
#include<sstream>

GUIButton::GUIButton(float x, float y, float width, float height, float r, float g, float b, std::string onClickName, LuaTable args)
	: x(x)
	, y(y)
	, width(width)
	, height(height)
	, r(r)
	, g(g)
	, b(b)
	, onClickName(onClickName)
	, args(args){
}

bool GUIButton::CheckClick(float inpX, float inpY)
{
	if (inpX >= x - (width / 2) && inpX <= x + (width / 2) && inpY >= y - (height / 2) && inpY <= y + (height / 2))
	{
		DoClick();
		return true;
	}
	return false;
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

#include<LuaBridge/LuaBridge.h>

//Get the LuaInterface created in main.cpp
extern LuaInterface l;

//Debug Purposes
void _myPrint(string s)
{
	std::string lineBr = "\n";
	std::string output = lineBr + s + lineBr;
	OutputDebugStringA(output.c_str());
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

//Assumes a base lua table is created beofre this function is called
void tableCreation(lua_State* L, LuaTable currentTable)
{
	//create the easy non-table objects
	map<string, LuaObject>::iterator it;
	for (it = currentTable.nonTableChildren.begin(); it != currentTable.nonTableChildren.end(); it++)
	{
		string key = it->first;
		LuaObject value = it->second;

		//push key
		lua_pushstring(L, key.c_str());

		//push value as correct type
		switch (value.type)
		{
			case LUA_TBOOLEAN:
				lua_pushboolean(L, value.val != "0");
				break;
			case LUA_TNUMBER:
				lua_pushnumber(L, std::stof(value.val));
				break;
			case LUA_TSTRING:
				lua_pushstring(L, value.val.c_str());
				break;
		}

		//set it to original table (created outside this function)
		lua_settable(L, -3);
	}

	map<string, LuaTable>::iterator tabIt;
	for (tabIt = currentTable.tableChildren.begin(); tabIt != currentTable.tableChildren.end(); tabIt++)
	{
		string key = tabIt->first;
		LuaTable value = tabIt->second;

		//push key of new table
		lua_pushstring(L, key.c_str());

		//create blank table and push it onto stack at position -1
		lua_newtable(L);

		//call this function and because the new table is on top it will now have values added to it
		tableCreation(L, value);

		//set the newly created and populated child table to the parent table 9created outside this function)
		lua_settable(L, -3);
	}		
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
		lua_setfield(l.L, -2, "X");
		lua_pushinteger(l.L, y);
		lua_setfield(l.L, -2, "Y");
		lua_pushinteger(l.L, width);
		lua_setfield(l.L, -2, "Width");
		lua_pushinteger(l.L, height);
		lua_setfield(l.L, -2, "Height");
		lua_pushnumber(l.L, r);
		lua_setfield(l.L, -2, "R");
		lua_pushnumber(l.L, g);
		lua_setfield(l.L, -2, "G");
		lua_pushnumber(l.L, b);
		lua_setfield(l.L, -2, "B");
		lua_pushstring(l.L, onClickName.c_str());
		lua_setfield(l.L, -2, "onClick");
		lua_pushnumber(l.L, id);
		lua_setfield(l.L, -2, "ID");

		//Base Arg table
		lua_newtable(l.L);

		tableCreation(l.L, args);

		lua_setfield(l.L, -2, "Args");

		//call function passing it the table as the only argument
		l.CheckLua(l.L, lua_pcall(l.L, 1, 0, 0));
	}
	else
	{
		string s = "Button click: Not a lua function";
		std::string lineBr = "\n";
		std::string output = lineBr + s + lineBr;
		OutputDebugStringA(output.c_str());
	}
}
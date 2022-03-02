#ifndef MY_LUA_TABLE_H
#define MY_LUA_TABLE_H

#include<map>
#include"LuaObject.h"
class LuaTable
{
	public:
		map<string, LuaTable> tableChildren;
		map<string, LuaObject> nonTableChildren;

		LuaTable(map<string, LuaTable> tC, map<string, LuaObject>nTC);
};
#endif
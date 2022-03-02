#include"LuaTable.h"

LuaTable::LuaTable(map<string, LuaTable> tC, map<string, LuaObject>nTC)
{
	tableChildren = tC;
	nonTableChildren = nTC;
}
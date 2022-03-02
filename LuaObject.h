#ifndef MY_LUA_OBJECT_H
#define MY_LUA_OBJECT_H

using namespace std;
#include<string>
class LuaObject
{
public:
	string val;
	int type;
	LuaObject(string v, int t);
};
#endif
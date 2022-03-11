#ifndef BUTTON_CLASS_H
#include<string>
extern "C"
{
#include "lua542/include/lua.h";
#include "lua542/include/lauxlib.h"
#include "lua542/include/lualib.h"
}
#include"LuaTable.h"
#ifdef _WIN32
#pragma comment(lib, "lua542/liblua54.a")
#endif
#ifdef _WIN64
#pragma comment(lib, "lua542/liblua54.a")
#endif
#define BUTTON_CLASS_H
class GUIButton
{
	public:
		//Event
		std::string onClickName;
		int id;
		LuaTable args;

		//Functions
		GUIButton(float inpX, float inpY, float inpWidth, float inpHeight, float inpR, float inpG, float inpB, std::string inpOnClick, LuaTable args);
		bool CheckClick(float inpX, float inpY);
		float GetDrawParameters();

		//All normalized rendering values
		float x;
		float y;
		float width;
		float height;
		//Color
		float r;
		float g;
		float b;

		int rect;

	private:
		void DoClick();
};
#endif
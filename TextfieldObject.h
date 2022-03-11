#ifndef TEXTFIELD_OBJECT_H
#define TEXTFIELD_OBJECT_H

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
class TextfieldObject
{
	public:
		//Event
		std::string text;
		int id;

		//Functions
		TextfieldObject(string text, float inpX, float inpY, float inpScale, float inpWidth, float inpHeight, float inpBackR, float inpBackG, float inpbackB, float inpTextR, float inpTextG, float inpTextB );
		bool CheckClick(float inpX, float inpY);

		//All normalized rendering values
		float x;
		float y;
		float scale;
		float width;
		float height;
		//Backing Color
		float backR;
		float backG;
		float backB;
		//Text Color
		float textR;
		float textG;
		float textB;

		int rect;
		int textObjId;

	private:
		void DoClick();
};
#endif
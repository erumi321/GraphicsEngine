#include"TextfieldObject.h"
#include<Windows.h>
#include <typeinfo>
#include"LuaInterface.h"
#include<map>
#include<string>
#include<sstream>

TextfieldObject::TextfieldObject(string text, float x, float y, float scale, float width, float height, float backR, float backG, float backB, float textR, float textG, float textB)
	: text(text)
	, x(x)
	, y(y)
	, scale(scale)
	, width(width)
	, height(height)
	, backR(backR)
	, backG(backG)
	, backB(backB)
	, textR(textR)
	, textG(textG)
	, textB(textB) {
}

bool TextfieldObject::CheckClick(float inpX, float inpY)
{
	if (inpX >= x - (width / 2) && inpX <= x + (width / 2) && inpY >= y - (height / 2) && inpY <= y + (height / 2))
	{
		return true;
	}
	return false;
}
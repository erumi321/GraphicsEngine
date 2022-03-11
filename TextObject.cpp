#include"TextObject.h"

#include<string>

using namespace std;

TextObject::TextObject(string text, float x, float y, float width, float scale, float r, float g, float b)
	: text(text)
	, x(x)
	, y(y)
	, width(width)
	, scale(scale)
	, r(r)
	, g(g)
	, b(b) {};
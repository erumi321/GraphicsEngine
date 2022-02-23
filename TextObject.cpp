#include"TextObject.h"

#include<string>

using namespace std;

TextObject::TextObject(string text, float x, float y, float scale, float r, float g, float b)
	: text(text)
	, x(x)
	, y(y)
	, scale(scale)
	, r(r)
	, g(g)
	, b(b) {};
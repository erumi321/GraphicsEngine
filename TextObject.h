#pragma once
#ifndef TEXTOBJECT_CLASS_H
#define TEXTOBJECT_CLASS_H

#include<string>

using namespace std;

class TextObject
{
	public:
		string text;
		float x, y, scale;
		float r, g, b;
		TextObject(string text, float x, float y, float scale, float r, float g, float b);
};
#endif
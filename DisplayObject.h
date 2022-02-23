#pragma once
#ifndef DISPLAYOBJECT_CLASS_H
#define DISPLAYOBJECT_CLASS_H

#include<glad/glad.h>
#include<vector>
#include<string>
using namespace std;

class DisplayObject
{
	public:
		vector<GLfloat> vertices;
		vector<GLuint> indices;
		int indexPoints;
		string id;
		DisplayObject(vector<GLfloat> _vertices, vector<GLuint> _indices, int _indexPoint);
};
#endif
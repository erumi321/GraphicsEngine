#include"DisplayObject.h"

DisplayObject::DisplayObject(vector<GLfloat> _vertices, vector<GLuint> _indices, int _indexPoints)
{
	vertices = _vertices;
	indices = _indices;
	indexPoints = _indexPoints;
}
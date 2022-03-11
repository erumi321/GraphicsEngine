#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
#include<Windows.h>
#include<string.h>
#include<string>
#include<sstream>
#include<list>
#include <iterator>
#include <vector>;
#include<tchar.h>
#include <unordered_map>
#include <ft2build.h>
#include <map>
#include <thread>
#include <chrono>
#include FT_FREETYPE_H 

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<map>

#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Texture.h"

#include"DisplayObject.h"
#include "GUIButton.h"
#include"TextObject.h"

#include<ft2build.h>
#include<freetype/freetype.h>

#include"LuaInterface.h"
#include"TextfieldObject.h"

extern "C"
{
	#include "lua542/include/lua.h";
	#include "lua542/include/lauxlib.h"
	#include "lua542/include/lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua542/liblua54.a")
#endif
#ifdef _WIN64
#pragma comment(lib, "lua542/liblua54.a")
#endif
#include"LuaTable.h"
#include<LuaBridge/LuaBridge.h>

using namespace std;
using namespace luabridge;

//Window to be used in most functions
GLFWwindow* window;
//Init LuaInterface in the global so that other scripts can use it
LuaInterface l;

#pragma region ID Handling

//Due to the 2048 we are limited to 2048 objects at once but hopefully we never need more then that, this can always be increased if needed
//The reason this isn't 1 million lets say is that it still takes up memory even when nothing is in it, so we wan't the most functional yet not massive id count
const int idAmount = 2048;
int availableIDs[idAmount] = {};

//populate availableIDs with numbers 1 to idAmount and then shuffle them
void generateIDs()
{
	for (int i = 0; i < idAmount; i++)
	{
		availableIDs[i] = i;
	}

	// Seed our random number generator.
	srand((int)time(NULL));
	// Create large number of swaps 
	// This example takes the size and times it by 20 for the number of swaps
	for (int i = 0; i < (idAmount * 20); i++) {
		// Generate random values for subscripts, not values!
		int randvalue1 = (rand() % idAmount) + 0;
		int randvalue2 = (rand() % idAmount) + 0;
		int temp = availableIDs[randvalue1];
		availableIDs[randvalue1] = availableIDs[randvalue2];
		availableIDs[randvalue2] = temp;
	}

}

//get the first index of availableIDs without or with a null value in it depending on input bool (Not null if true, Null if false)
//if true find first one not null, if false find first one that is null
int getIDNullorNotIndex(bool isNull)
{
	int nonNullIndex = 0;
	for (int i : availableIDs)
	{
		if ((i != NULL) == isNull)
		{
			break;
		}
		nonNullIndex++;
	}
	return nonNullIndex;
}

//Gets the first non-null id value in availableIDs, and sets it to null
int getAvailableID()
{
	int idIndex = getIDNullorNotIndex(true);
	int ID = availableIDs[idIndex];
	availableIDs[idIndex] = NULL;

	return ID;
}

//Change a null value in index closest to 0 to the input ID
void refreshID(int ID)
{
	int newestIDIndex = getIDNullorNotIndex(false);
	availableIDs[newestIDIndex] = ID;
}

#pragma endregion ID Handling

#pragma region Object lists

//All non-special objects (the actual geometry and not the behaviours)
vector<DisplayObject> activeObjects;

//All text drawn on screen
vector<TextObject> activeTextObjects;

//All buttons for event handling, the actual shapes for the buttons is in activeObjects
vector<GUIButton> activeButtons;

//For click and text-editing handling, actual shapes already in activeObjects and text in activeTextObjects
vector<TextfieldObject> activeTextfieldObjects;

#pragma endregion Object lists

#pragma region Create Object Functions

//Add a DisplayObject in the shape of a rectangle to activeObjects and then return its id
int CreateRectangle(GLFWwindow* window, float x, float y, float width, float height, float red = 1.0f, float green = 1.0f, float blue = 1.0f) {
	//For normalization
	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	//normalize x and y to be between -1 and 1
	float normalX = (x - (screenWidth / 2)) / (screenWidth / 2);
	float normalY = ((screenHeight / 2) - y) / (screenHeight / 2);

	//normalize width to between 0 and 1
	float normalWidth = width / screenWidth;
	float normalHeight = height / screenHeight;

	//get 4 corner points
	vector<GLfloat> ver =
	{//		COORDINATES											/		 COLORS		   /	TexCoord	//
		normalX - normalWidth,	normalY - normalHeight,	0.0f,		red, green, blue,	  0.0f, 0.0f,	// Lower left corner
		normalX - normalWidth,	normalY + normalHeight,	0.0f,		red, green, blue,	  0.0f,	1.0f,	//Upper left corner
		normalX + normalWidth,	normalY + normalHeight,	0.0f,		red, green, blue,	  1.0f, 1.0f,	//Upper right corner
		normalX + normalWidth,	normalY - normalHeight,	0.0f,		red, green, blue,	  1.0f, 0.0f	//Lower right corner
	};

	//default rectangle indices
	vector<GLuint> ind =
	{
		0, 2, 1,
		0, 3, 2
	};

	//Initalize a new DisplayObject with 4 vertex points and the previous created verticies and indicies, then push it into activeObjects
	DisplayObject rect(ver, ind, 4);
	activeObjects.push_back(rect);

	int newId = getAvailableID();

	activeObjects.back().id = newId;

	return newId;
}

//Create a new TextObject then push it into activeTextObjects then return the id of the text object
int CreateText(GLFWwindow* window, string text, float x, float y, float width, float scale, float r, float g, float b, int idOverride = -1)
{
	TextObject textObj(text, x, y, width, scale, r, g, b);

	activeTextObjects.push_back(textObj);

	int newId = getAvailableID();

	if (idOverride != -1)
	{
		newId = idOverride;
	}


	activeTextObjects.back().id = newId;

	return newId;
}

//Create a new GUIBUtton object, create the Rectangle for it, then push the button object into activeButtons then return the id of the button object
int CreateButton(GLFWwindow* window, float x, float y, float width, float height, string onClickName, LuaTable args, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
{
	GUIButton btn(x, y, width, height, red, green, blue, onClickName, args);

	int rect = CreateRectangle(window, x, y, width, height, red, green, blue);

	btn.rect = rect;

	activeButtons.push_back(btn);

	int newId = getAvailableID();

	activeButtons.back().id = newId;

	return newId;
}

//Create a new TextfieldObject object, create the backing rectangle for it, and the text for it, then push it into activeTextfieldObjects, returning the id of the Textfield Object
int CreateTextfield(GLFWwindow* window, string text, float x, float y, float width, float height, float scale, float backR, float backG, float backB, float textR, float textG, float textB)
{
	TextfieldObject field(text, x, y, scale, width, height, backR, backG, backB, textR, textG, textB);

	int rect = CreateRectangle(window, x, y, width, height, backR, backG, backB);

	field.rect = rect;

	float xPos = x - (width / 2);
	float yPos = y - (height / 2);

	int textObj = CreateText(window, text, xPos, yPos, width, scale, textR, textG, textB);
	field.textObjId = textObj;

	activeTextfieldObjects.push_back(field);

	int newId = getAvailableID();

	activeTextfieldObjects.back().id = newId;

	return newId;
}

#pragma endregion Create Object Functions

//Setup GLFW
void glfwSetup()
{
	//Init GLFW
	glfwInit();

	//Tell GLFW what version of OpenGL we are using
	//In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Tell GLFW we are using the CORE profile
	//means we onyl have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

}

#pragma region Geometry Rendering

//Get all the vertices of all active objects (not text)
vector<GLfloat> compileVertices()
{
	vector<GLfloat> ret;
	for (DisplayObject obj : activeObjects)
	{
		for (GLfloat f : obj.vertices)
		{
			ret.push_back(f);
		}
	}

	return ret;
}

//Get all the indicies of all the active objects (not text) and offset them so that all indices are in one array
vector<GLuint> compileIndices()
{
	vector<GLuint> ret;
	int offset = 0;
	for (DisplayObject obj : activeObjects)
	{
		int num = 0;
		for (GLuint i : obj.indices)
		{
			ret.push_back(i + offset);
		}
		offset += obj.indexPoints;
	}
	return ret;
}

//Setup VAO, VBO, and EBO then renderall objects in activeObjects using compileVertices() and compileIndices()
void renderObjects(Shader shaderProgram)
{
	vector<GLfloat> v = compileVertices();
	vector<GLuint> i = compileIndices();

	//Create refrence contains for the Vertex Array Object and the Vertex Buffer Object
	VAO VAO1;

	VAO1.Bind();

	VBO VBO1(v.data(), v.size() * sizeof(GLfloat));
	EBO EBO1(i.data(), i.size() * sizeof(GLuint));

	float vertexMemLength = 8 * sizeof(float);

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, vertexMemLength, (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, vertexMemLength, (void*)(3 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, vertexMemLength, (void*)(6 * sizeof(float)));

	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	//Bind the VAO so OpenGL knows to use it
	VAO1.Bind();

	//Draw the triangle using the GL_TRIANGLES primitive
	glDrawElements(GL_TRIANGLES, i.size(), GL_UNSIGNED_INT, 0);
	VAO1.Unbind();

	//Clean-up
	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
}

//Recursivly loop through a lua table and convert it to c++ objects of LuaTable containing LuaObject
LuaTable getLuaTable(lua_State* L, int index)
{
	map<string, LuaObject> objs;
	map<string, LuaTable> tabs;
	lua_pushnil(L);  /* first key */
	while (lua_next(L, index) != 0) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		int valueType = lua_type(L, index + 2);
		int keyType = lua_type(L, index + 1);

		string key = "";

		//if key is a number (which is most common when one is not defined at all), then we need to get it as a number otherwise a bunch of stuff is messed up
		if (keyType == LUA_TNUMBER)
		{
			int num = lua_tonumber(L, index + 1);

			// declaring output string stream
			ostringstream str1;

			// Sending a number as a stream into output
			// string
			str1 << num;

			// the str() converts number into string
			key = str1.str();
		}
		else {
			key = lua_tostring(L, index + 1);

		}

		//if value is not a table, just create a new object and push it into the base table, if it is recursivly call this and push the table returned into the base table
		if (valueType != LUA_TTABLE)
		{
			string value = lua_tostring(L, index + 2);
			LuaObject newObj(value, valueType);

			objs.insert(pair<string, LuaObject>(key, newObj));

		}
		else
		{
			LuaTable childTable = getLuaTable(L, index + 2);

			tabs.insert(pair<string, LuaTable>(key, childTable));
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);

	}

	LuaTable tableObj(tabs, objs);

	return tableObj;
}

#pragma endregion Geometry Rendering

#pragma region Modify Object Functions

//Modifies an existing DisplayObject to have new values
void ModifyRectangle(GLFWwindow* window, int id, float x, float y, float width, float height, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
{
	//Init blank object
	DisplayObject obj(vector<GLfloat>(), vector<GLuint>(), 0);

	//Get original DisplayObject using id
	int index = 0;
	for (DisplayObject g : activeObjects)
	{
		if (g.id == id)
		{
			obj = g;
			break;
		}
		index++;
	}
	
	//For normalization
	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);

	//normalize x and y to be between -1 and 1
	float normalX = (x - (screenWidth / 2)) / (screenWidth / 2);
	float normalY = ((screenHeight / 2) - y) / (screenHeight / 2);

	//normalize width to between 0 and 1
	float normalWidth = width / screenWidth;
	float normalHeight = height / screenHeight;

	//get 4 corner points
	vector<GLfloat> ver =
	{//		COORDINATES											/		 COLORS		   /	TexCoord	//
		normalX - normalWidth,	normalY - normalHeight,	0.0f,		red, green, blue,	  0.0f, 0.0f,	// Lower left corner
		normalX - normalWidth,	normalY + normalHeight,	0.0f,		red, green, blue,	  0.0f,	1.0f,	//Upper left corner
		normalX + normalWidth,	normalY + normalHeight,	0.0f,		red, green, blue,	  1.0f, 1.0f,	//Upper right corner
		normalX + normalWidth,	normalY - normalHeight,	0.0f,		red, green, blue,	  1.0f, 0.0f	//Lower right corner
	};

	//default rectangle indices
	vector<GLuint> ind =
	{
		0, 2, 1,
		0, 3, 2
	};

	//set new indices and verticies
	obj.vertices = ver;
	obj.indices = ind;

	//overwrite old object with new
	activeObjects[index] = obj;
}

//Modify existing TextObject object 
void ModifyText(GLFWwindow* window, int id, string text, float x, float y, float width, float scale, float r, float g, float b)
{
	//Get original text using id, and assign id if found
	int index = 0;
	int objId = 0;

	for (TextObject t : activeTextObjects)
	{
		if (t.id == id)
		{
			objId = id;
			break;
		}
		index++;
	}

	//if any values are null (nil when used in lua) then just take old values so each doesn't have to be redefined everytime you modify something
	string newText = text;
	float newX = x != NULL ? x : activeTextObjects[index].x;
	float newY = y != NULL ? y : activeTextObjects[index].y;
	float newWidth = width != NULL ? width : activeTextObjects[index].width;
	float newScale = scale != NULL ? scale : activeTextObjects[index].scale;
	float newR = r != NULL ? r : activeTextObjects[index].r;
	float newG = g != NULL ? g : activeTextObjects[index].g; 
	float newB = b != NULL ? b : activeTextObjects[index].b;

	//Init the new text
	TextObject txt(newText, newX, newY, newWidth, newScale, newR, newG, newB);

	//update text
	activeTextObjects[index] = txt;
	activeTextObjects[index].id = objId;
}

//Modifies an existing GUIButton object and its linked rectangle
void ModifyButton(GLFWwindow* window, int id, float x, float y, float width, float height, string onClickName, LuaTable args, float r = 1.0f, float g = 1.0f, float b = 1.0f)
{


	//Get original button using id
	int index = 0;
	for (GUIButton g : activeButtons)
	{
		if (g.id == id)
		{
			break;
		}
		index++;
	}

	//if any values are null (nil when used in lua) then just take old values so each doesn't have to be redefined everytime you modify something, also takes old if the input args table is empty
	float newX = x != NULL ? x : activeButtons[index].x;
	float newY = y != NULL ? y : activeButtons[index].y;
	float newWidth = width != NULL ? width : activeButtons[index].width;
	float newHeight = height != NULL ? height : activeButtons[index].height;
	string newOnClickName = onClickName != "" ? onClickName : activeButtons[index].onClickName;
	LuaTable newArgs = args.nonTableChildren.size() != 0 && args.tableChildren.size() != 0 ? args : activeButtons[index].args;
	float newR = r != NULL ? r : activeButtons[index].r;
	float newG = g != NULL ? g : activeButtons[index].g;
	float newB = b != NULL ? b : activeButtons[index].b;

	//Init new button
	GUIButton btn(newX, newY, newWidth, newHeight, newR, newG, newB, newOnClickName, newArgs);

	btn.rect = activeButtons[g].rect;

	//change rectangle
	ModifyRectangle(window, btn.rect, x, y, width, height, newR, newG, newB);

	//update button
	activeButtons[index] = btn;
}

//Modifies an existing TextfieldObject object and its linked rectangle and text
void ModifyTextfield(GLFWwindow* window, int id, string text, float x, float y, float width, float height, float scale, float backR, float backG, float backB, float textR, float textG, float textB)
{
	//Get original button using id
	int index = 0;
	for (TextfieldObject g : activeTextfieldObjects)
	{
		if (g.id == id)
		{
			break;
		}
		index++;
	}

	//if any values are null (nil when used in lua) then just take old values so each doesn't have to be redefined everytime you modify something, also takes old if the input args table is empty
	string newText = text != "" ? text : activeTextfieldObjects[index].text;
	float newX = x != NULL ? x : activeTextfieldObjects[index].x;
	float newY = y != NULL ? y : activeTextfieldObjects[index].y;
	float newWidth = width != NULL ? width : activeTextfieldObjects[index].width;
	float newHeight = height != NULL ? height : activeTextfieldObjects[index].height;
	float newScale = scale != NULL ? scale : activeTextfieldObjects[index].scale;
	float newBackR = backR != NULL ? backR : activeTextfieldObjects[index].backR;
	float newBackG = backG != NULL ? backG : activeTextfieldObjects[index].backG;
	float newBackB = backB != NULL ? backB : activeTextfieldObjects[index].backB;
	float newTextR = textR != NULL ? textR : activeTextfieldObjects[index].textR;
	float newTextG = textG != NULL ? textG : activeTextfieldObjects[index].textG;
	float newTextB = textB != NULL ? textB : activeTextfieldObjects[index].textB;

	//Init new textfield object
	TextfieldObject field(newText, newX, newY, newScale, newWidth, newHeight, newBackR, newBackG, newBackB, newTextR, newTextG, newTextB);

	field.rect = activeTextfieldObjects[index].rect;

	field.textObjId = activeTextfieldObjects[index].textObjId;

	field.id = id;

	//change rectangle
	ModifyRectangle(window, field.rect, newX, newY, newWidth, newHeight, newBackR, newBackG, newBackB);

	//change text
	float xPos = newX - (newWidth / 2);
	float yPos = newY - (newHeight / 2);
	ModifyText(window, field.textObjId, text, xPos, yPos, width, scale, textR, textG, textB);

	//update button
	activeTextfieldObjects[index] = field;
}

#pragma endregion Modify Object Functions

#pragma region Delete Object Functions

//Modify existing TextObject object 
void DeleteText(GLFWwindow* window, int id)
{

	//Get original text using id, and assign id if found
	int index = 0;
	for (TextObject t : activeTextObjects)
	{
		if (t.id == id)
		{
			break;
		}
		index++;
	}

	refreshID(activeTextObjects[index].id);
	activeTextObjects.erase(activeTextObjects.begin() + index);
}

//Delete existing DisplayObject
void DeleteRectangle(GLFWwindow* window, int id)
{
	int index = 0;

	string rectId = "";

	for (DisplayObject g : activeObjects)
	{
		if (g.id == id)
		{
			break;
		}
		index++;
	}

	refreshID(activeObjects[index].id);
	activeObjects.erase(activeObjects.begin() + index);
}

//Delete existing Button from ActiveButtons then delete it's rectangle
void DeleteButton(GLFWwindow* window, int id)
{
	//Get original button using id
	int index = 0;
	
	int rectId = 0;

	for (GUIButton g : activeButtons)
	{
		if (g.id == id)
		{
			rectId = g.rect;
			break;
		}
		index++;
	}

	refreshID(activeObjects[index].id);

	activeButtons.erase(activeButtons.begin() + index);

	//change rectangle
	DeleteRectangle(window, rectId);
}

//Delete existing Textfield from ActiveTextfields then delete it's rectangle and text
void DeleteTextfield(GLFWwindow* window, int id)
{
	//Get original button using id
	int index = 0;

	int rectId = 0;
	int textId = 0;

	for (TextfieldObject g : activeTextfieldObjects)
	{
		if (g.id == id)
		{
			rectId = g.rect;
			textId = g.textObjId;
			break;
		}
		index++;
	}

	refreshID(activeTextfieldObjects[index].id);

	activeTextfieldObjects.erase(activeTextfieldObjects.begin() + index);

	//delete linked objects
	DeleteRectangle(window, rectId);
	DeleteText(window, textId);
}

#pragma endregion Delete Object Functions

#pragma region Lua Create Functions

//Lua function to call internal CreateRectangle
int lua_CreateRectangle(lua_State* L)
{
	float x = (float)lua_tonumber(L, 1);
	float y = (float)lua_tonumber(L, 2);
	float width = (float)lua_tonumber(L, 3);
	float height = (float)lua_tonumber(L, 4);
	float r = (float)lua_tonumber(L, 5);
	float g = (float)lua_tonumber(L, 6);
	float b = (float)lua_tonumber(L, 7);

	int id = CreateRectangle(window, x, y, width, height, r, g, b);

	//Return id to lua
	lua_pushnumber(L, id);
	return 1;
}

//Lua function to call internal CreateText
int lua_CreateText(lua_State* L)
{
	string text = (string)lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float width = (float)lua_tonumber(L, 4);
	float scale = (float)lua_tonumber(L, 5);
	float r = (float)lua_tonumber(L, 6);
	float g = (float)lua_tonumber(L, 7);
	float b = (float)lua_tonumber(L, 8);
	int id = CreateText(window, text, x, y, width, scale, r, g, b);

	//Return id to lua
	lua_pushnumber(L, id);

	return 1;
}

//Lua function to call internal CreateButton
int lua_CreateButton(lua_State* L)
{
	float x = (float)lua_tonumber(L, 1);
	float y = (float)lua_tonumber(L, 2);
	float width = (float)lua_tonumber(L, 3);
	float height = (float)lua_tonumber(L, 4);
	string clickEvent = (string)lua_tostring(L, 5);
	float r = (float)lua_tonumber(L, 6);
	float g = (float)lua_tonumber(L, 7);
	float b = (float)lua_tonumber(L, 8);

	LuaTable args = getLuaTable(L, 9);

	int id = CreateButton(window, x, y, width, height, clickEvent, args, r, g, b);

	//Return id to lua
	lua_pushnumber(L, id);
	return 1;
}

//Lua function to call internal CreateTextfield
int lua_CreateTextfield(lua_State* L)
{
	string text = (string)lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float width = (float)lua_tonumber(L, 4);
	float height = (float)lua_tonumber(L, 5);
	float scale = (float)lua_tonumber(L, 6);
	float backR = (float)lua_tonumber(L, 7);
	float backG = (float)lua_tonumber(L, 8);
	float backB = (float)lua_tonumber(L, 9);
	float textR = (float)lua_tonumber(L, 10);
	float textG = (float)lua_tonumber(L, 11);
	float textB = (float)lua_tonumber(L, 12);
	
	int id = CreateTextfield(window, text, x, y, width, height, scale, backR, backG, backB, textR, textG, textB);


	//Return id to lua
	lua_pushnumber(L, id);
	return 1;
}

#pragma endregion Lua Create Functions

#pragma region Lua Modify Functions

//Lua function to call internal ModifyButton
int lua_ModifyRectangle(lua_State* L)
{
	int objectID = (float)lua_tonumber(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float width = (float)lua_tonumber(L, 4);
	float height = (float)lua_tonumber(L, 5);
	float r = (float)lua_tonumber(L, 6);
	float g = (float)lua_tonumber(L, 7);
	float b = (float)lua_tonumber(L, 8);

	ModifyRectangle(window, objectID, x, y, width, height, r, g, b);



	return 0;
}

//Lua function to call internal ModifyText
int lua_ModifyText(lua_State* L)
{
	float objectID = (float)lua_tonumber(L, 1);
	string text = (string)lua_tostring(L, 2);
	float x = (float)lua_tonumber(L, 3);
	float y = (float)lua_tonumber(L, 4);
	float width = (float)lua_tonumber(L, 5);
	float scale = (float)lua_tonumber(L, 6);
	float r = (float)lua_tonumber(L, 7);
	float g = (float)lua_tonumber(L, 8);
	float b = (float)lua_tonumber(L, 9);

	ModifyText(window, objectID, text, x, y, width, scale, r, g, b);

	return 0;
}

//Lua function to call internal ModifyRectangle
int lua_ModifyButton(lua_State* L)
{
	float id = (float)lua_tonumber(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float width = (float)lua_tonumber(L, 4);
	float height = (float)lua_tonumber(L, 5);
	string clickEvent = (string)lua_tostring(L, 6);
	float r = (float)lua_tonumber(L, 7);
	float g = (float)lua_tonumber(L, 8);
	float b = (float)lua_tonumber(L, 9);

	LuaTable args = getLuaTable(L, 10);

	ModifyButton(window, id, x, y, width, height, clickEvent, args, r, g, b);

	return 0;
}

//Lua function to call internal ModifyTextfield
int lua_ModifyTextfield(lua_State* L)
{
	float id = (float)lua_tonumber(L, 1);
	string text = (string)lua_tostring(L, 2);
	float x = (float)lua_tonumber(L, 3);
	float y = (float)lua_tonumber(L, 4);
	float width = (float)lua_tonumber(L, 5);
	float height = (float)lua_tonumber(L, 6);
	float scale = (float)lua_tonumber(L, 7);
	float backR = (float)lua_tonumber(L, 8);
	float backG = (float)lua_tonumber(L, 9);
	float backB = (float)lua_tonumber(L, 10);
	float textR = (float)lua_tonumber(L, 11);
	float textG = (float)lua_tonumber(L, 12);
	float textB = (float)lua_tonumber(L, 13);

	ModifyTextfield(window, id, text, x, y, width, height, scale, backR, backG, backB, textR, textG, textB);

	return 0;
}
#pragma endregion Lua Modify Functions

#pragma region Lua Delete Functions

//Lua function to call internal DeleteRectangle
int lua_DeleteRectangle(lua_State* L)
{
	float id = (float)lua_tonumber(L, 1);

	DeleteRectangle(window, id);

	return 0;
}

//Lua function to call internal DeleteText
int lua_DeleteText(lua_State* L)
{
	float id = (float)lua_tonumber(L, 1);

	DeleteText(window, id);

	return 0;
}

//Lua function to call internal DeleteButton
int lua_DeleteButton(lua_State* L)
{
	float id = (float)lua_tonumber(L, 1);

	DeleteButton(window, id);

	return 0;
}

//Lua function to call internal DeleteTextfield
int lua_DeleteTextfield(lua_State* L)
{
	float id = (float)lua_tonumber(L, 1);

	DeleteTextfield(window, id);

	return 0;
}

#pragma endregion Lua Delete Functions

//Debug Purposes
void myPrint(string s)
{
	std::string lineBr = "\n";
	std::string output = lineBr + s + lineBr;
	OutputDebugStringA(output.c_str());
}

//Debug
int lua_myPrint(lua_State* L)
{
	string s = lua_tostring(L, -1);
	myPrint(s);
	return 0;
}

//Define alll the variables needed for cursor display and simple text editing
int activeTextfieldObjectID = -1;
int cursorOffset = 0;
int currentTextfieldTextID = -1;

int currentCursorID = -1;
int cursorLifeTime = 0;
int cursorMaxLifeTime = 530; //This is the default windows setting

#pragma region Callbacks

//"character" here refers to a literla character key, nothing like left arrow or home, etc.
void character_pressed_callback(GLFWwindow* window, unsigned int codepoint)
{

	int index = 0;
	if (activeTextfieldObjectID != -1)
	{
		for (TextfieldObject field : activeTextfieldObjects)
		{
			//search through the textfields for the active one, then add the new character onto the end and update the text
			if (field.id == activeTextfieldObjectID)
			{
				field.text += (unsigned char)codepoint;
				activeTextfieldObjects[index].text += (unsigned char)codepoint;
				ModifyText(window, field.textObjId, field.text, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				cursorLifeTime = cursorMaxLifeTime;
				break;
			}
			index++;
		}
	}
}

//Special key handling like left arrow, right arrow, backspace, etc.
void key_pressed_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//automatically refresh cursor
		cursorLifeTime = cursorMaxLifeTime;
		if (key == GLFW_KEY_BACKSPACE)
		{
			if (activeTextfieldObjectID != -1)
			{
				int index = 0;
				for (TextfieldObject field : activeTextfieldObjects)
				{
					//search through textfields for current one, and remove the last character, then update its text
					if (field.id == activeTextfieldObjectID)
					{
						string newText = field.text;
						int deleteIndex = newText.length() + cursorOffset - 1;
						if (deleteIndex < 0)
						{
							deleteIndex = 0;
						}

						newText.erase(deleteIndex, 1);
						field.text = newText;
						activeTextfieldObjects[index].text = newText;
						ModifyText(window, field.textObjId, newText, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
						break;
					}
					index++;
				}
			}
		}
		else if (key == GLFW_KEY_LEFT)
		{
			//move cursor left 1
			cursorOffset--;
		}
		else if (key == GLFW_KEY_RIGHT)
		{
			//move cursor right 1 and clamp it to 0
			cursorOffset++;
			if (cursorOffset > 0)
			{
				cursorOffset = 0;
			}
		}
	}

}

//Check buttons for click when window clicked (mostly handled by GLFW)
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_RELEASE)
	{
		//reset textfield cursor
		activeTextfieldObjectID = -1;
		currentTextfieldTextID = -1;
		cursorOffset = 0;
		if (currentCursorID != -1)
		{
			DeleteRectangle(window, currentCursorID);
			currentCursorID = -1;
			cursorLifeTime = 0;
		}

		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		//buttons block textfield click events
		for (GUIButton btn : activeButtons)
		{
			if (btn.CheckClick(xPos, yPos)) {
				return;
			}
		}

		for (TextfieldObject field : activeTextfieldObjects)
		{
			if (field.CheckClick(xPos, yPos))
			{
				//if clicked and textfield is not currently selected (because why reselect if it is), then select it
				if (activeTextfieldObjectID != field.id)
				{
					activeTextfieldObjectID = field.id;
					currentTextfieldTextID = field.textObjId;
					cursorOffset = 0;
					//Force instant cursor creation
					cursorLifeTime = cursorMaxLifeTime;
				}
				return;
			}
		}
	}
}

//in gl specify new window size when window is resized so things scale correctly
void window_resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

#pragma endregion Callbacks

#pragma region Text Rendering
//Character stuct for text rendering
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;

FT_Library ft;
FT_Face face;

//Create text and font stuff needed to render text
int initText() {

	//Setup text
	if (FT_Init_FreeType(&ft))
	{
		myPrint("ERROR::FREETYPE: Could not init FreeType Library");
		return -1;
	}

	if (FT_New_Face(ft, "Fonts/Roboto-Medium.ttf", 0, &face))
	{
		myPrint("ERROR::FREETYPE: Failed to load font");
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		myPrint("ERROR::FREETYTPE: Failed to load Glyph");
		return -1;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	//Load first 128 ascii characters into the glypc cache
	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}
}

//The actual computations to render text, takes a VAO and VBO for rendering purposes
void RenderText(Shader& s, VAO VAO, VBO VBO, int textID, std::string text, float x, float y, float width, float scale, glm::vec3 color)
{
	//Get screen width and screen height for use in normalizing positions
	int sW, sH;
	glfwGetWindowSize(window, &sW, &sH);
	float screenWidth = static_cast<float>(sW);
	float screenHeight = static_cast<float>(sH);

	// activate corresponding render state	
	s.Activate();
	GLuint uniID2 = glGetUniformLocation(s.ID, "projection");

	//setup shader variables
	glm::mat4 projection = glm::ortho(0.0f, screenWidth, 0.0f, screenHeight);
	glUniformMatrix4fv(uniID2, 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);

	VAO.Bind();

	//split the text into a vector of lines
	vector<string> lines;
	std::string::const_iterator w;
	float curXPos = x;
	float curWordXOffset = 0;
	string currentLine = "";
	string currentWord = "";

	for (w = text.begin(); w != text.end(); w++)
	{
		if (w[0] == *" ")
		{
			currentLine += currentWord + " ";
			currentWord = "";
			curXPos += curWordXOffset;
			curWordXOffset = 0;
		}
		Character ch = Characters[*w];
		curWordXOffset += (ch.Advance >> 6) * scale + (ch.Bearing.x * scale * 2); // bitshift by 6 to get value in pixels (2^6 = 64)

		//if text has spaces and it exceeds the length wrap the current word onto a new one so its not split off, if line has no spaces then just split the word
		if (currentLine.find(" ") != string::npos)
		{
			if (curXPos + curWordXOffset > x + width)
			{
				lines.push_back(currentLine);
				currentLine = "";
				curXPos = x;
			}
		}
		else
		{
			if (curXPos + curWordXOffset > x + width)
			{
				currentLine += currentWord;
				lines.push_back(currentLine);
				currentLine = "";

				currentWord = "";
				curWordXOffset = 0;
				curXPos = x;
			}
		}
		if (w[0] != *" ")
		{
			currentWord += w[0];
		}
	}


	currentLine += currentWord;
	lines.push_back(currentLine);

	// iterate through all characters
	std::string::const_iterator c;
	//For the use of wrapping lines
	float baseY = y;
	float baseX = x;
	float cursorX = x;
	int cursorIndex = text.length() + cursorOffset - 1;
	if (lines[0] == "")
	{
		baseY += face->height / 16 * scale;
		Character ch = Characters[*" "];
		//get xPos and offset it by xOffset calculated before so that text is centered
		float xpos = (baseX + ch.Bearing.x * scale);
		//with how this system works y(0) is at the bottom, of we just reverse it to be at the top like everything else
		float ypos = (screenHeight - baseY) - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		VBO.Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		VBO.Unbind();
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
	}
	else
	{
		int currentIndex = 0;
		for (string line : lines)
		{
			baseX = x;
			for (c = line.begin(); c != line.end(); c++)
			{

				//if the 1st character isn't a space add a space to give some spacing
				if (c == line.begin())
				{
					baseY += face->height / 48 * scale;
				}
				if (c == line.begin() && c[0] != *" ")
				{
					Character spa = Characters[*" "];
					baseX += (spa.Advance >> 6) * scale;
				}
				Character ch = Characters[*c];
				//get xPos and offset it by xOffset calculated before so that text is centered
				float xpos = (baseX + ch.Bearing.x * scale);
				//with how this system works y(0) is at the bottom, of we just reverse it to be at the top like everything else
				float ypos = (screenHeight - baseY) - (ch.Size.y - ch.Bearing.y) * scale;

				float w = ch.Size.x * scale;
				float h = ch.Size.y * scale;
				// update VBO for each character
				float vertices[6][4] = {
					{ xpos,     ypos + h,   0.0f, 0.0f },
					{ xpos,     ypos,       0.0f, 1.0f },
					{ xpos + w, ypos,       1.0f, 1.0f },

					{ xpos,     ypos + h,   0.0f, 0.0f },
					{ xpos + w, ypos,       1.0f, 1.0f },
					{ xpos + w, ypos + h,   1.0f, 0.0f }
				};
				// render glyph texture over quad
				glBindTexture(GL_TEXTURE_2D, ch.TextureID);
				// update content of VBO memory
				VBO.Bind();
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
				VBO.Unbind();
				// render quad
				glDrawArrays(GL_TRIANGLES, 0, 6);
				// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				baseX += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)

				if (currentIndex == cursorIndex)
				{
					cursorX = baseX;
				}

				currentIndex++;
			}

		}
	}

	if (activeTextfieldObjectID != -1 && textID == currentTextfieldTextID)
	{
		if (cursorLifeTime >= cursorMaxLifeTime)
		{
			if (currentCursorID == -1)
			{
				currentCursorID = CreateRectangle(window, cursorX, baseY - (face->height * (0.01076680672 * scale)), face->height * scale / 1024, face->height / 32 * scale, 1.0, 1.0, 1.0);
			}
			else
			{
				DeleteRectangle(window, currentCursorID);
				currentCursorID = -1;
			}
			cursorLifeTime = 0;
		}
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//setubs text VAO and VBO then calls rendertext for each TextObject inside activeTextObjects
void setupText(Shader shader)
{
	//Text rendering
	VAO VAO;
	VBO VBO(NULL, sizeof(float) * 6 * 4);

	VAO.Bind();

	VAO.LinkAttrib(VBO, 0, 4, GL_FLOAT, 4 * sizeof(float), (void*)0);

	VAO.Unbind();
	VBO.Unbind();

	for (TextObject t : activeTextObjects)
	{
		glm::vec3 color = glm::vec3(t.r, t.g, t.b);
		RenderText(shader, VAO, VBO, t.id, t.text, t.x, t.y, t.width, t.scale, color);
	}
}

#pragma endregion Text Rendering

//in a seperate thread so text rendering doesn't slow this down ,as when there is mroe text the cursor updates slower
// wait for cursorMaxLifetime, then set it to cursorMaxLifeTime to force refresh it
void updateCursor() {
	while (1)
	{
		this_thread::sleep_for(chrono::milliseconds(cursorMaxLifeTime));
		if (activeTextfieldObjectID != -1)
		{
			if (cursorLifeTime < cursorMaxLifeTime)
			{
				cursorLifeTime = cursorMaxLifeTime;
			}
		}
	}

}

int main() 
{
	generateIDs();
	//basic setup
	glfwSetup();
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	int windowWidth = desktop.right;
	int windowHeight = desktop.bottom;
	//Create a GLFWwindow object of 800 by 800 pixels, named GraphicEngine
	window = glfwCreateWindow(windowWidth, windowHeight, "GraphicEngine", NULL, NULL);
	
	glfwSetWindowSizeCallback(window, window_resize_callback);
	glfwSetCharCallback(window, character_pressed_callback);
	
	glfwSetKeyCallback(window, key_pressed_callback);

	//Check if window fails to create
	if (window == NULL)
	{
		myPrint("Failed to created GLFW window");
		glfwTerminate();
		return -1;
	}

	thread uC(updateCursor);
	uC.detach();

	//Expose lua functions to the lua code
	l.AddFunction("CreateRectangle", lua_CreateRectangle);
	l.AddFunction("CreateButton", lua_CreateButton);
	l.AddFunction("CreateText", lua_CreateText);
	l.AddFunction("CreateTextfield", lua_CreateTextfield);
	l.AddFunction("ModifyRectangle", lua_ModifyRectangle);
	l.AddFunction("ModifyButton", lua_ModifyButton);
	l.AddFunction("ModifyText", lua_ModifyText);
	l.AddFunction("ModifyTextfield", lua_ModifyTextfield);
	l.AddFunction("DeleteRectangle", lua_DeleteRectangle);
	l.AddFunction("DeleteButton", lua_DeleteButton);
	l.AddFunction("DeleteText", lua_DeleteText);
	l.AddFunction("DeleteTextfield", lua_DeleteTextfield);
	l.AddFunction("MyPrint", lua_myPrint);

	//Run the lua
	l.CheckLua(l.L, luaL_dofile(l.L, "Scripts/Main.lua"));

	//Set mouse callback
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//Introduce the window to the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	//Specify OpenGL viewport size to glad

	
	glViewport(0, 0, windowWidth, windowHeight);
	glfwMaximizeWindow(window);

	initText();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Init normal and Text shaders
	Shader shaderProgram("default.vert", "default.frag");

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	Shader shaderProgramtext("Text.vert", "Text.frag");

	int i = 1;

	//Dont end the program until the window is closed
	while (!glfwWindowShouldClose(window))
	{
		//Specify background color
		glClearColor(0.07f, 0.13f, 0.17f, 1.0);
		//Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);
		//Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();
		glUniform1f(uniID, 0.0f);

		//render non-text
		renderObjects(shaderProgram);

		//render text
		setupText(shaderProgramtext);

		//Swap the back buffer with the front buffer
		glfwSwapBuffers(window);

		//Allow all GLFW events to happen (if thsi isnt called then the window is entierly unresponsive
		glfwPollEvents();
	}

	//Delete all the objects we've created
	shaderProgram.Delete();
	shaderProgramtext.Delete();
	//Clean-up
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	l.Close();

	//Delete window before ending the program
	glfwDestroyWindow(window);
	//Terminate GLFW before ending the program
	glfwTerminate();
	
	return 0;
}
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
#include<Windows.h>
#include<string.h>
#include<string>
#include<list>
#include <iterator>
#include <vector>;
#include<tchar.h>

#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Texture.h"

#include"DisplayObject.h"
#include "GUIButton.h"

#include"LuaInterface.h"

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

using namespace std;

GLFWwindow* window;

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

vector<DisplayObject> activeObjects;

//Drawing parameters already in activeObjects this is just for click handling
vector<GUIButton> activeButtons;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_RELEASE)
	{
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		for (GUIButton btn : activeButtons)
		{

			btn.CheckClick(xPos, yPos);
		}
	}

}

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

//If normalized then we do -1 to 1 (for both size and position), otherwise use pixel values (and consequentially normalize using math)
void CreateRectangle(GLFWwindow* window, float x, float y, float width, float height, float red = 1.0f, float green = 1.0f, float blue = 1.0f) {
	int screenWidth, screenHeight;
	glfwGetWindowSize(window, &screenWidth, &screenHeight);
	
	float normalX = (x - (screenWidth / 2)) / (screenWidth / 2);
	float normalY = ((screenHeight / 2) - y) / (screenHeight / 2);

	float normalWidth = width / screenWidth;
	float normalHeight = height / screenHeight;

	vector<GLfloat> ver =
	{//		COORDINATES											/		 COLORS		   /	TexCoord	//
		normalX - normalWidth,	normalY - normalHeight,	0.0f,		red, green, blue,	  0.0f, 0.0f,	// Lower left corner
		normalX - normalWidth,	normalY + normalHeight,	0.0f,		red, green, blue,	  0.0f,	1.0f,	//Upper left corner
		normalX + normalWidth,	normalY + normalHeight,	0.0f,		red, green, blue,	  1.0f, 1.0f,	//Upper right corner
		normalX + normalWidth,	normalY - normalHeight,	0.0f,		red, green, blue,	  1.0f, 0.0f	//Lower right corner
	};

	vector<GLuint> ind =
	{
		0, 2, 1,
		0, 3, 2
	};

	DisplayObject rect(ver, ind, 4);
	activeObjects.push_back(rect);
}

void CreateButton(GLFWwindow* window, float x, float y, float width, float height, string onClickName, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
{
	GUIButton btn(x, y, width, height, red, green, blue, onClickName);

	CreateRectangle(window, x, y, width, height, red, green, blue);

	activeButtons.push_back(btn);
}

int lua_CreateRectangle(lua_State* L)
{
	float x = (float)lua_tonumber(L, 1);
	float y = (float)lua_tonumber(L, 2);
	float width = (float)lua_tonumber(L, 3);
	float height = (float)lua_tonumber(L, 4);
	float r = (float)lua_tonumber(L, 5);
	float g = (float)lua_tonumber(L, 6);
	float b = (float)lua_tonumber(L, 7);
	CreateRectangle(window,x, y, width, height, r, g, b);

	return 0;
}

int lua_myPrint(lua_State* L)
{
	string s = lua_tostring(L, -1);
	std::string lineBr = "\n";
	std::string output = lineBr + s + lineBr;
	OutputDebugStringA(output.c_str());
	return 0;
}

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
	CreateButton(window, x, y, width, height, clickEvent, r, g, b);

	return 0;
}

LuaInterface l;

int main() 
{
	glfwSetup();

	//Create a GLFWwindow object of 800 by 800 pixels, named YoutubeOpenGL
	window = glfwCreateWindow(800, 800, "GraphicEngine", NULL, NULL);
	
	//Check if window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to created GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}


	l.AddFunction("CreateRectangle", lua_CreateRectangle);
	l.AddFunction("MyPrint", lua_myPrint);
	l.AddFunction("CreateButton", lua_CreateButton);

	l.CheckLua(l.L, luaL_dofile(l.L, "VideoExample.lua"));


	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//CreateRectangle(window, -0.5f, 0.5f, 0.5f, 0.5f, true, 1.0f, 0.8f, 0.0f);
	//Introduce the window to the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	//Specify OpenGL viewport size to glad
	glViewport(0, 0, 800, 800);

	Shader shaderProgram("default.vert", "default.frag");

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	//Dont end the program until the window is closed
	while (!glfwWindowShouldClose(window))
	{

		vector<GLfloat> v = compileVertices();
		vector<GLuint> i = compileIndices();

		//Create refrence contains for the Vertex Array Object and the Vertex Buffer Object
		VAO VAO1;

		VAO1.Bind();

		VBO VBO1(v.data(), v.size() * sizeof(GLfloat));
		EBO EBO1(i.data(), i.size() * sizeof(GLuint));
		//VBO VBO1(ver, sizeof(ver));
		//EBO EBO1(ind, sizeof(ind));

		float vertexMemLength = 8 * sizeof(float);

		VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, vertexMemLength, (void*)0);
		VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, vertexMemLength, (void*)(3 * sizeof(float)));
		VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, vertexMemLength, (void*)(6 * sizeof(float)));


		//Clean-up
		VAO1.Unbind();
		VBO1.Unbind();
		EBO1.Unbind();
		//Specify background color
		glClearColor(0.07f, 0.13f, 0.17f, 1.0);
		//Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);
		//Tell OpenGL which Shader Program we want to use
		shaderProgram.Activate();
		glUniform1f(uniID, 0.0f);

		//Bind the VAO so OpenGL knows to use it
		VAO1.Bind();

		//Draw the triangle using the GL_TRIANGLES primitive
		glDrawElements(GL_TRIANGLES, i.size(), GL_UNSIGNED_INT, 0);
		//Swap the back buffer with the front buffer
		glfwSwapBuffers(window);

		//Allow all GLFW events to happen (if thsi isnt called then the window is entierly unresponsive
		glfwPollEvents();
		VAO1.Delete();
		VBO1.Delete();
		EBO1.Delete();
	}

	//Delete all the objects we've created
	shaderProgram.Delete();
	l.Close();

	//Delete window before ending the program
	glfwDestroyWindow(window);
	//Terminate GLFW before ending the program
	glfwTerminate();
	
	return 0;
}
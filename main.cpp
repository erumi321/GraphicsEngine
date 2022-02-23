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
#include <ft2build.h>
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
vector<TextObject> activeTextObjects;

//Drawing parameters already in activeObjects this is just for click handling
vector<GUIButton> activeButtons;

//Check buttons for click when window clicked (mostly handled by GLFW)
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

//Character stuct for text rendering
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;

//The actual computations to render text, takes a VAO and VBO for rendering purposes
void RenderText(Shader& s, VAO VAO, VBO VBO, std::string text, float x, float y, float scale, glm::vec3 color)
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
	
	//calculate how large the text will be so that we can center it
	float xOffset = 0.0f;
	std::string::const_iterator a;
	for (a = text.begin(); a != text.end(); a++)
	{
		Character ch = Characters[*a];
		xOffset += (ch.Advance >> 6) * scale / 2; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];
		//get xPos and offset it by xOffset calculated before so that text is centered
		float xpos = (x + ch.Bearing.x * scale) - xOffset;
		//with how this system works y(0) is at the bottom, of we just reverse it to be at the top like everything else
		float ypos = (screenHeight - y) - (ch.Size.y - ch.Bearing.y) * scale;

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
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
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
		RenderText(shader, VAO, VBO, t.text, t.x, t.y, t.scale, color);
	}
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

//Add a DisplayObject in the shape of a rectangle to activeObjects and then return its id
string CreateRectangle(GLFWwindow* window, float x, float y, float width, float height, float red = 1.0f, float green = 1.0f, float blue = 1.0f) {
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

	//Turn pointer into string for use as id
	const void* rectAddress = static_cast<const void*>(&activeObjects.back());
	std::stringstream idSs;
	idSs << rectAddress;
	std::string id = idSs.str();

	activeObjects.back().id = id;

	return id;
}

//Create a new GUIBUtton object, create the Rectangle for it, then push the button object into activeButtons then return the id of the button object
string CreateButton(GLFWwindow* window, float x, float y, float width, float height, string onClickName, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
{
	GUIButton btn(x, y, width, height, red, green, blue, onClickName);

	string rect = CreateRectangle(window, x, y, width, height, red, green, blue);

	btn.rect = rect;

	activeButtons.push_back(btn);

	//Turn pointer into string for use as id
	const void* buttonAddress = static_cast<const void*>(&activeButtons.back());
	std::stringstream idSs;
	idSs << buttonAddress;
	std::string id = idSs.str();

	activeButtons.back().id = id;

	return id;
}

//Create a new TextObject then push it into activeTextObjects then return the id of the text object
string CreateText(GLFWwindow* window, string text, float x, float y, float scale, float r, float g, float b)
{
	TextObject textObj(text, x, y, scale, r, g, b);

	activeTextObjects.push_back(textObj);


	//Turn pointer into string for use as id
	const void* textAddress = static_cast<const void*>(&activeButtons.back());
	std::stringstream idSs;
	idSs << textAddress;
	std::string id = idSs.str();

	activeTextObjects.back().id = id;

	return id;
}

//Modifies an existing DisplayObject to have new values
void ModifyRectangle(GLFWwindow* window, string id, float x, float y, float width, float height, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
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

//Modifies an existing GUIButton object and its linked rectangle
void ModifyButton(GLFWwindow* window, string id, float x, float y, float width, float height, string onClickName, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
{
	//Init blank button
	GUIButton btn(x, y, width, height, red, green, blue, onClickName);

	//Get original button using id
	int index = 0;
	for (GUIButton g : activeButtons)
	{
		if (g.id == id)
		{
			btn = g;
			break;
		}
		index++;
	}

	//Change values to new values
	btn.x = x;
	btn.y = y;
	btn.width = width;
	btn.height = height;
	btn.onClickName = onClickName;
	btn.r = red;
	btn.g = green;
	btn.b = blue;

	//change rectangle
	ModifyRectangle(window, btn.rect, x, y, width, height, red, green, blue);

	//update button
	activeButtons[index] = btn;
}

//Modify existing TextObject object 
void ModifyText(GLFWwindow* window, string id, string text, float x, float y, float scale, float r, float g, float b)
{
	//Init the new text
	TextObject txt(text, x, y, scale, r, g, b);

	//Get original text using id, and assign id if found
	int index = 0;
	for (TextObject t : activeTextObjects)
	{
		if (t.id == id)
		{
			txt.id = id;
			break;
		}
		index++;
	}

	//update text
	activeTextObjects[index] = txt;
}

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

	string id = CreateRectangle(window, x, y, width, height, r, g, b);

	//Return id to lua
	lua_pushstring(L, id.c_str());
	return 1;
}

//Lua function to call internal CreateText
int lua_CreateText(lua_State* L)
{
	string text = (string)lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float scale = (float)lua_tonumber(L, 4);
	float r = (float)lua_tonumber(L, 5);
	float g = (float)lua_tonumber(L, 6);
	float b = (float)lua_tonumber(L, 7);
	string id = CreateText(window, text, x, y, scale, r, g, b);

	//Return id to lua
	lua_pushstring(L, id.c_str());

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
	string id = CreateButton(window, x, y, width, height, clickEvent, r, g, b);

	//Return id to lua
	lua_pushstring(L, id.c_str());
	return 1;
}

//Lua function to call internal ModifyRectangle
int lua_ModifyButton(lua_State* L)
{
	string id = (string)lua_tostring(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float width = (float)lua_tonumber(L, 4);
	float height = (float)lua_tonumber(L, 5);
	string clickEvent = (string)lua_tostring(L, 6);
	float r = (float)lua_tonumber(L, 7);
	float g = (float)lua_tonumber(L, 8);
	float b = (float)lua_tonumber(L, 9);

	ModifyButton(window, id, x, y, width, height, clickEvent, r, g, b);

	return 0;
}

//Lua function to call internal ModifyButton
int lua_ModifyRectangle(lua_State* L)
{
	string objectID = (string)lua_tostring(L, 1);
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

int lua_ModifyText(lua_State* L)
{
	string objectID = (string)lua_tostring(L, 1);
	string text = (string)lua_tostring(L, 2);
	float x = (float)lua_tonumber(L, 3);
	float y = (float)lua_tonumber(L, 4);
	float scale = (float)lua_tonumber(L, 5);
	float r = (float)lua_tonumber(L, 6);
	float g = (float)lua_tonumber(L, 7);
	float b = (float)lua_tonumber(L, 8);

	ModifyText(window, objectID, text, x, y, scale, r, g, b);

	return 0;
}

//Init LuaInterface in the global so that other scripts can use it
LuaInterface l;

int initText() {
	FT_Library ft;
	FT_Face face;
	//Setup text
	if (FT_Init_FreeType(&ft))
	{
		myPrint("ERROR::FREETYPE: Could not init FreeType Library");
		return -1;
	}

	if (FT_New_Face(ft, "Fonts/FreeSans.ttf", 0, &face))
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

	//Clean-up
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}
int main() 
{

	//basic setup
	glfwSetup();

	int windowWidth = 800;
	int windowHeight = 800;
	//Create a GLFWwindow object of 800 by 800 pixels, named GraphicEngine
	window = glfwCreateWindow(windowWidth, windowHeight, "GraphicEngine", NULL, NULL);
	
	//Check if window fails to create
	if (window == NULL)
	{
		myPrint("Failed to created GLFW window");
		glfwTerminate();
		return -1;
	}

	//Expose lua functions to the lua code
	l.AddFunction("CreateRectangle", lua_CreateRectangle);
	l.AddFunction("MyPrint", lua_myPrint);
	l.AddFunction("CreateButton", lua_CreateButton);
	l.AddFunction("CreateText", lua_CreateText);
	l.AddFunction("ModifyRectangle", lua_ModifyRectangle);
	l.AddFunction("ModifyButton", lua_ModifyButton);
	l.AddFunction("ModifyText", lua_ModifyText);

	//Run the lua
	l.CheckLua(l.L, luaL_dofile(l.L, "Scripts/Main.lua"));

	//Set mouse callback
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//Introduce the window to the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();

	//Specify OpenGL viewport size to glad
	glViewport(0, 0, 800, 800);

	initText();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Init normal and Text shaders
	Shader shaderProgram("default.vert", "default.frag");

	GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

	Shader shaderProgramtext("Text.vert", "Text.frag");

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
	l.Close();

	//Delete window before ending the program
	glfwDestroyWindow(window);
	//Terminate GLFW before ending the program
	glfwTerminate();
	
	return 0;
}
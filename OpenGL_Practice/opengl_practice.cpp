
using namespace std;

#include "stdlib.h"
#include "time.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <string>
#include <fstream>
#include <array>


// Prototypes
int setShader(char* shaderType, char* shaderFile);
char* readShader(std::string fileName);
void timer(int);
void createModel(int n);
void createCystal();

#define BUFFER_OFFSET(x)  ((const void*) (x))
#define FPS 60
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,0.9,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)


static unsigned int
program,
vertexShaderId,
fragmentShaderId;

glm::mat4 MVP, View, Projection;
GLuint vao, points_vbo, colors_vbo, modelID, indices_vbo;
int randomColorSeed[10];

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
float cameraSpeed = 0.2;

float scale = 1.0f, inc = -0.05f, angle1 = 0.0f, angle2 = 0.0f;


static float R = 1.0; // Radius of circle.
static float X = 0; // X-coordinate of center of circle.
// for 2 units
static float YUpside = 1.0f; // Y-coordinate of center of circle.
static float YDownside = -1.0f; // Y-coordinate of center of circle.
const int MaxNumVertices = 500; // Number of vertices on circle.
static int numVertices = 10;
#define PI 3.14159265358979324

float theta = 0.0f;
float movementOffset = 0.0f;



GLfloat shape_vertices_upside[MaxNumVertices][3] = { 0.0f, };
GLfloat shape_colors_upside[MaxNumVertices][3] = { 0.0f, };
GLuint shape_indices_upside[MaxNumVertices] = { 0, };

GLfloat shape_vertices_downside[MaxNumVertices][3] = { 0.0f, };
GLfloat shape_colors_downside[MaxNumVertices][3] = { 0.0f, };
GLuint shape_indices_downside[MaxNumVertices] = { 0, };

GLfloat shape_vertices_column[MaxNumVertices][3] = { 0.0f, };
GLfloat shape_colors_column[MaxNumVertices][3] = { 0.0f, };
GLuint shape_indices_column[MaxNumVertices] = { 0, };

GLfloat crystal_shape_vertices_upside[MaxNumVertices][3] = { 0.0f, };
GLfloat crystal_shape_colors_upside[MaxNumVertices][3] = { 0.0f, };
GLuint crystal_shape_indices_upside[MaxNumVertices] = { 0, };

GLfloat crystal_shape_vertices_downside[MaxNumVertices][3] = { 0.0f, };
GLfloat crystal_shape_colors_downside[MaxNumVertices][3] = { 0.0f, };
GLuint crystal_shape_indices_downside[MaxNumVertices] = { 0, };

void init(void)
{
	// Create shader program executable.
	vertexShaderId = setShader((char*)"vertex", (char*)"basic.vert");
	fragmentShaderId = setShader((char*)"fragment", (char*)"basic.frag");
	program = glCreateProgram();
	glAttachShader(program, vertexShaderId);
	glAttachShader(program, fragmentShaderId);
	glLinkProgram(program);
	glUseProgram(program);

	modelID = glGetUniformLocation(program, "MVP");

	// Projection matrix : 45¡Ä Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//Projection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	View = glm::lookAt(
		glm::vec3(0, 0, 3), // Origin. Camera is at (0,0,3), in World Space
		glm::vec3(0, 0, 0),	  // Look target. Looks at the origin
		glm::vec3(0, 1, 0)   // Up vector. Head is up (set to 0,-1,0 to look upside-down)
	);


	// Create and set-up the vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);



	// vertex
	glGenBuffers(1, &points_vbo);
	// Populate the position buffer
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	// don't need to do now
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), cube_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(0); // for Vertex position

	// color
	glGenBuffers(1, &colors_vbo);
	// Populate the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	// don't need to do now
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	glEnableVertexAttribArray(1); // for Vertex color



	glGenBuffers(1, &indices_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);


	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	timer(0);
}

//---------------------------------------------------------------------
//
// transformModel
//
void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);

	View = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	//View = glm::lookAt(
	//	glm::vec3(xVal, yVal, 5), // Origin. Camera is at (0,0,3), in World Space
	//	glm::vec3(0, 0, -4),	  // Look target. Looks at the origin
	//	glm::vec3(0, 1, 0)   // Up vector. Head is up (set to 0,-1,0 to look upside-down)
	//);
	MVP = Projection * View * Model;
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &MVP[0][0]);
}

//---------------------------------------------------------------------
//
// display
//

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	glEnable(GL_DEPTH_TEST);
	transformObject({1.5f ,0.25f, 1.5f}, X_AXIS, 0, glm::vec3(0.0f, -1.2f, 0.0f));

	createModel(numVertices);
	//createPlatform();
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices_upside), shape_vertices_upside, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_colors_upside), shape_colors_upside, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices_upside), shape_indices_upside, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLE_FAN, numVertices, GL_UNSIGNED_INT, NULL);
	//glDrawElements(GL_LINE_LOOP, numVertices, GL_UNSIGNED_INT, NULL);



	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices_downside), shape_vertices_downside, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_colors_downside), shape_colors_downside, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices_downside), shape_indices_downside, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLE_FAN, numVertices, GL_UNSIGNED_INT, NULL);


	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices_column), shape_vertices_column, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_colors_column), shape_colors_column, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices_column), shape_indices_column, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLE_FAN, numVertices * 6, GL_UNSIGNED_INT, NULL);

	glDisable(GL_DEPTH_TEST);
	/* by array
	glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);*/

	// by element array

	glEnable(GL_CULL_FACE);

	// upside
	transformObject(glm::vec3(0.5f), Y_AXIS, angle1 += 0.8, glm::vec3(0.0f, 0.3f + cos(movementOffset)*0.1, 0.0f));
	createCystal();

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crystal_shape_vertices_upside), crystal_shape_vertices_upside, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crystal_shape_colors_upside), crystal_shape_colors_upside, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(crystal_shape_indices_upside), crystal_shape_indices_upside, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLE_FAN, 3*6, GL_UNSIGNED_INT, NULL);


	// downside
	createCystal();

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crystal_shape_vertices_downside), crystal_shape_vertices_downside, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crystal_shape_colors_downside), crystal_shape_colors_downside, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(crystal_shape_indices_downside), crystal_shape_indices_downside, GL_STATIC_DRAW);

	glDrawElements(GL_TRIANGLE_FAN, 3 * 6, GL_UNSIGNED_INT, NULL);


	glDisable(GL_CULL_FACE);

	glutSwapBuffers(); // Now for a potentially smoother render.
}

void idle() // Not even called.
{
	glutPostRedisplay();
}

void timer(int) {
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, timer, 0);
	movementOffset += 0.05;
}


// Keyboard input processing routine.
void keyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			exit(0);
			break;
		case 'w':
			cameraPos += cameraSpeed * cameraFront;
			break;
		case 's':
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case 'r':
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			break;
		case 'f':
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			break;
		case 'a':
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case 'd':
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case '+':
			numVertices++;
			break;
		default:
			break;
	}
}

//---------------------------------------------------------------------
//
// main
//

int main(int argc, char** argv)
{
	//Before we can open a window, theremust be interaction between the windowing systemand OpenGL.In GLUT, this interaction is initiated by the following function call :
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	//if you comment out this line, a window is created with a default size
	glutInitWindowSize(800, 800);

	//the top-left corner of the display
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Dynamic Polygon");

	// background color is white
	glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init(); // Our own custom function.

	//If there are events in the queue, our program responds to them through functions
	//called callbacks.A callback function is associated with a specific type of event.
	//A display callback is generated when the application programm or the
	//operating system determines that the graphics in a window need to be redrawn.
	glutDisplayFunc(display); // Output.
	//glutIdleFunc(idle);

	glutKeyboardFunc(keyDown); // Input.

	//begin an event-processing loop
	glutMainLoop();
}

// Function to initialize shaders.
int setShader(char* shaderType, char* shaderFile)
{
	int shaderId;
	char* shader = readShader(shaderFile);

	if (shaderType == "vertex") shaderId = glCreateShader(GL_VERTEX_SHADER);
	if (shaderType == "tessControl") shaderId = glCreateShader(GL_TESS_CONTROL_SHADER);
	if (shaderType == "tessEvaluation") shaderId = glCreateShader(GL_TESS_EVALUATION_SHADER);
	if (shaderType == "geometry") shaderId = glCreateShader(GL_GEOMETRY_SHADER);
	if (shaderType == "fragment") shaderId = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(shaderId, 1, (const char**)&shader, NULL);
	glCompileShader(shaderId);

	return shaderId;
}
// Function to read external shader file.
char* readShader(std::string fileName)
{
	// Initialize input stream.
	std::ifstream inFile(fileName.c_str(), std::ios::binary);

	// Determine shader file length and reserve space to read it in.
	inFile.seekg(0, std::ios::end);
	int fileLength = inFile.tellg();
	char* fileContent = (char*)malloc((fileLength + 1) * sizeof(char));

	// Read in shader file, set last character to NUL, close input stream.
	inFile.seekg(0, std::ios::beg);
	inFile.read(fileContent, fileLength);
	fileContent[fileLength] = '\0';
	inFile.close();

	return fileContent;
}

void createCystal()
{

	//making vertices firstly

	std::array<glm::vec3, MaxNumVertices> midVertices = {};
	std::array<glm::vec3, MaxNumVertices> crystalColors = {};
	int i = 0;

	float radius = 0.5f;
	float height = 4.0f;

	theta = 0;
	for (i = 0; i < 6; ++i)
	{
		midVertices[i] = glm::vec3(X + radius * cos(theta), 0, radius *  -1 * sin(theta));
		//colors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		//crystalColors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		crystalColors[i] = glm::vec3(0.f, 1.f, 0.f);

		crystal_shape_vertices_upside[i][0] = midVertices[i].x;
		crystal_shape_vertices_upside[i][1] = midVertices[i].y;
		crystal_shape_vertices_upside[i][2] = midVertices[i].z;

		crystal_shape_colors_upside[i][0] = crystalColors[i][0];
		crystal_shape_colors_upside[i][1] = crystalColors[i][1];
		crystal_shape_colors_upside[i][2] = crystalColors[i][2];
		// for indices
		//crystal_shape_indices_column[i] = i;

		theta += 2 * PI / 6;
	}

	// upside vectex
	crystal_shape_vertices_upside[6][0] = 0.0f;
	crystal_shape_vertices_upside[6][1] = height/2;
	crystal_shape_vertices_upside[6][2] = 0.0f;

	crystal_shape_colors_upside[6][0] = 0.f;
	crystal_shape_colors_upside[6][1] = 1.f;
	crystal_shape_colors_upside[6][2] = 0.f;



	// for indices
	int indexOfindices = 0;
	for(i = 0; i < 6; i++)
	{
		crystal_shape_indices_upside[indexOfindices++] = 6;
		crystal_shape_indices_upside[indexOfindices++] = i;
		//crystal_shape_indices_column[indexOfindices++] = 7;
		if(i+1 > 5)
		{
			crystal_shape_indices_upside[indexOfindices++] = 0;
		}
		else
		{
			crystal_shape_indices_upside[indexOfindices++] = i + 1;
		}
	}



	theta = 0;
	for (i = 0; i < 6; ++i)
	{
		midVertices[i] = glm::vec3(X + radius * cos(theta), 0, radius * sin(theta));
		//colors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		//crystalColors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		crystalColors[i] = glm::vec3(0.f, 1.f, 0.f);

		crystal_shape_vertices_downside[i][0] = midVertices[i].x;
		crystal_shape_vertices_downside[i][1] = midVertices[i].y;
		crystal_shape_vertices_downside[i][2] = midVertices[i].z;

		crystal_shape_colors_downside[i][0] = crystalColors[i][0];
		crystal_shape_colors_downside[i][1] = crystalColors[i][1];
		crystal_shape_colors_downside[i][2] = crystalColors[i][2];
		// for indices
		//crystal_shape_indices_column[i] = i;

		theta += 2 * PI / 6;
	}



	// downside vertex
	crystal_shape_vertices_downside[6][0] = 0.0f;
	crystal_shape_vertices_downside[6][1] = height / -2;
	crystal_shape_vertices_downside[6][2] = 0.0f;

	crystal_shape_colors_downside[6][0] = 0.f;
	crystal_shape_colors_downside[6][1] = 1.f;
	crystal_shape_colors_downside[6][2] = 0.f;

	// for indices
	indexOfindices = 0;
	for (i = 0; i < 6; i++)
	{
		crystal_shape_indices_downside[indexOfindices++] = 6;
		crystal_shape_indices_downside[indexOfindices++] = i;
		//crystal_shape_indices_column[indexOfindices++] = 7;
		if (i + 1 > 5)
		{
			crystal_shape_indices_downside[indexOfindices++] = 0;
		}
		else
		{
			crystal_shape_indices_downside[indexOfindices++] = i + 1;
		}
	}


}

void createModel(int n)
{
	theta = 0.0f;

	std::array<glm::vec3, MaxNumVertices> upVertices = {};
	std::array<glm::vec3, MaxNumVertices> upColors = {};
	std::array<glm::vec3, MaxNumVertices> downVertices = {};
	std::array<glm::vec3, MaxNumVertices> downColors = {};
	for (int i = 0; i < n; ++i)
	{

		upVertices[i] = glm::vec3(X + R * cos(theta), YUpside, R * sin(theta));
		//colors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		upColors[i] = glm::vec3(0.f, 0.f, 1.f);

		shape_vertices_upside[i][0] = upVertices[i].x;
		shape_vertices_upside[i][1] = upVertices[i].y;
		shape_vertices_upside[i][2] = upVertices[i].z;

		shape_colors_upside[i][0] = upColors[i][0];
		shape_colors_upside[i][1] = upColors[i][1];
		shape_colors_upside[i][2] = upColors[i][2];
		// for indices
		shape_indices_upside[i] = i;


		downVertices[i] = glm::vec3(X + R * cos(theta), YDownside, R *  sin(theta));
		//colors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		downColors[i] = glm::vec3(0.f, 0.f, 1.f);
		shape_vertices_downside[i][0] = downVertices[i].x;
		shape_vertices_downside[i][1] = downVertices[i].y;
		shape_vertices_downside[i][2] = downVertices[i].z;

		shape_colors_downside[i][0] = downColors[i][0];
		shape_colors_downside[i][1] = downColors[i][1];
		shape_colors_downside[i][2] = downColors[i][2];

		// for indices
		shape_indices_downside[i] = i;

		theta += 2 * PI / n;
	}

	for (int i = 0; i < n; ++i)
	{

		shape_vertices_column[i * 6][0] = shape_vertices_upside[i][0];
		shape_vertices_column[i * 6][1] = shape_vertices_upside[i][1];
		shape_vertices_column[i * 6][2] = shape_vertices_upside[i][2];

		if (i + 1 == n)
		{
			shape_vertices_column[i * 6 + 1][0] = shape_vertices_upside[0][0];
			shape_vertices_column[i * 6 + 1][1] = shape_vertices_upside[0][1];
			shape_vertices_column[i * 6 + 1][2] = shape_vertices_upside[0][2];
		}
		else
		{
			shape_vertices_column[i * 6 + 1][0] = shape_vertices_upside[i + 1][0];
			shape_vertices_column[i * 6 + 1][1] = shape_vertices_upside[i + 1][1];
			shape_vertices_column[i * 6 + 1][2] = shape_vertices_upside[i + 1][2];
		}

		shape_vertices_column[i * 6 + 2][0] = shape_vertices_downside[i][0];
		shape_vertices_column[i * 6 + 2][1] = shape_vertices_downside[i][1];
		shape_vertices_column[i * 6 + 2][2] = shape_vertices_downside[i][2];



		if (i + 1 == n)
		{
			shape_vertices_column[i * 6 + 3][0] = shape_vertices_upside[0][0];
			shape_vertices_column[i * 6 + 3][1] = shape_vertices_upside[0][1];
			shape_vertices_column[i * 6 + 3][2] = shape_vertices_upside[0][2];

			shape_vertices_column[i * 6 + 5][0] = shape_vertices_downside[0][0];
			shape_vertices_column[i * 6 + 5][1] = shape_vertices_downside[0][1];
			shape_vertices_column[i * 6 + 5][2] = shape_vertices_downside[0][2];
		}
		else
		{
			shape_vertices_column[i * 6 + 3][0] = shape_vertices_upside[i + 1][0];
			shape_vertices_column[i * 6 + 3][1] = shape_vertices_upside[i + 1][1];
			shape_vertices_column[i * 6 + 3][2] = shape_vertices_upside[i + 1][2];

			shape_vertices_column[i * 6 + 5][0] = shape_vertices_downside[i + 1][0];
			shape_vertices_column[i * 6 + 5][1] = shape_vertices_downside[i + 1][1];
			shape_vertices_column[i * 6 + 5][2] = shape_vertices_downside[i + 1][2];
		}

		shape_vertices_column[i * 6 + 4][0] = shape_vertices_downside[i][0];
		shape_vertices_column[i * 6 + 4][1] = shape_vertices_downside[i][1];
		shape_vertices_column[i * 6 + 4][2] = shape_vertices_downside[i][2];





		//glm::vec3 color = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		glm::vec3 color = glm::vec3(0.f, 0.f, 1.f);
		for (int j = 0; j < 6; j++)
		{
			shape_colors_column[i * 6 + j][0] = color.x;
			shape_colors_column[i * 6 + j][1] = color.y;
			shape_colors_column[i * 6 + j][2] = color.z;

			// for indices
			shape_indices_column[i * 6 + j] = i * 6 + j;
		}

	}

	//theta = 0.0f;
	//for (int i = 0; i < n; ++i)
	//{

	//	vertices[i] = glm::vec3(X + R * cos(theta), YDownside, R * sin(theta));
	//	//colors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
	//	colors[i] = glm::vec3(1.f, 1.f, 0.f);
	//	shape_vertices_downside[i][0] = vertices[i].x;
	//	shape_vertices_downside[i][1] = vertices[i].y;
	//	shape_vertices_downside[i][2] = vertices[i].z;

	//	shape_colors_downside[i][0] = colors[i][0];
	//	shape_colors_downside[i][1] = colors[i][1];
	//	shape_colors_downside[i][2] = colors[i][2];

	//	// for indices
	//	shape_indices_downside[i] = i;

	//	theta += 2 * PI / n;
	//}


}
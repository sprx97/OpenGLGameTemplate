//
//  OpenGLTemplate.cpp
//  
//
//  Created by Jeremy Vercillo on 2/27/14.
//
//

/*
	Here are the defines for Mac and Linux. You'll have to
	work on the Windows ones.
*/
#include <GL/glew.h>
#include "OVR.h"
#ifdef __APPLE__
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	
	#include <AL/alut.h>
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>

	#include <AL/al.h>
	#include <AL/alc.h>
	#include <AL/alut.h>
#endif
// OpenGL/AL


#include <SOIL/soil.h> // Simple OpenGL Image Library
#include <CSE40166/CSE40166.h>
/*	This is a library that our professor made for us.
	It has Cameras, Materials, Lights, Objects, and more! */

#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
// C++ libraries

using namespace std;
using namespace CSE40166;

int width = 1280, height = 750; // width and height in windowed mode
int lastframe = 0; // time last frame was rendered at
float MAX_FPS = 60.0; // FPS cap

int GLUT_ESC_KEY = 27;
char keys[256]; // struct for storing which keys are down

GLint leftmouse = GLUT_UP, rightmouse = GLUT_UP, middlemouse = GLUT_UP;
bool firstmousepos = false; // has mouse been found before
int mousex = 0, mousey = 0, mousedx = 0, mousedy = 0;
// mouse movement/button variables

#define NUM_SOURCES 4
#define NUM_SOUNDS 4
ALuint source[NUM_SOURCES];
ALuint buffer[NUM_SOUNDS];
// sounds

Camera* arccam = NULL;
Camera* firstpersoncam = NULL;
#define ARCCAM 0
#define FIRSTPERSONCAM 1
int CAMERA = ARCCAM;
/* Two Cameras
	The Arcball Camera is the standard camera that focuses on one object
	and can zoom and rotate around it. The first person camera looks in the
	direction the user is facing from the user's position.
*/

PointLight* pointLight;
// a single light

Material* defaultwhite;
// materials

Object* dummyObject;
Point* point;
// just an example

GLuint skybox[6];
GLuint skyboxindex;
float skyboxwidth = 2000;
// background textures

GLuint vertshader, fragshader, shaderprogram;
// GLSL shaders

/* void resize(int w, int h)
	GLUT reshpae function. Sets the width and height variables to the
	new width and height of the screen.
*/
void resize(int w, int h) {
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, width/(float)height, .1, 100000);
}

/* void key_press(unsigned char key, int x, int y)
	Updates which keys are pressed. Key is
	the character of the pressed key and x and y are the
	location of the mouse at that time.
*/
void key_press(unsigned char key, int x, int y) {
	if(key == GLUT_ESC_KEY) exit(0);
	if(key == 'c') {
		if (CAMERA == ARCCAM) {
			CAMERA = FIRSTPERSONCAM;
		} else {
			CAMERA = ARCCAM;
		}
	}
	// one-time immediate actions by key go here

	keys[key] = 1;
}

/* void key_release(unsigned char key, int x, int y)
	Updates which keys are not pressed anymore. Key is
	the character of the released key and x and y are the
	location of the mouse at that time.
*/
void key_release(unsigned char key, int x, int y) {
	keys[key] = 0;
}

/* void mouse_click(int button, int state, int x, int y)
	Reacts to mouse button pressed. Button is the button (left, right, middle),
	state is the state of the button (pressed, released), and 
	x and y are the position of the cursor at that time.
*/
void mouse_click(int button, int state, int x, int y) {
	if(button == GLUT_LEFT_BUTTON) leftmouse = state;
	if(button == GLUT_RIGHT_BUTTON) rightmouse = state;
	if(button == GLUT_MIDDLE_BUTTON) middlemouse = state;
	
	// user x and y for location of clicks if necessary
}

/* void motion(int x, int y)
	GLUT callback for mouse motion (both pressed and unpressed). These
	could be separated into two functions (for regular motion and click+drag
	for example).
	
	Updates global variables to the new x and y for later use. Also updates 
	how far the mouse has moved since the last call to motion 
	(mousedx and mousedy).
*/
void mouse_motion(int x, int y) {
	if(firstmousepos) {
		mousedx = x - mousex;
		mousedy = y - mousey;
	}
	
	mousex = x;
	mousey = y;
	firstmousepos = true;
}

/* void drawGrid()
	Draws a white grid on the ground (for debugging).
*/
void drawGrid() {
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1); // white
	glBegin(GL_LINES);
		for(double x = -10.0; x <= 10.01; x += 1) {
			glVertex3f(x, 0, -10);
			glVertex3f(x, 0, 10);

			glVertex3f(-10, 0, x);
			glVertex3f(10, 0, x);
		}
	glEnd();
	glEnable(GL_LIGHTING);
}

/* void drawAxes()
	Draws x, y, and z axes in red, green, and blue. For debugging.
*/
void drawAxes() {
	glDisable(GL_LIGHTING);
	
	glLineWidth(5.0);
	
	glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 0);
		// +x

		glColor3f(.5, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(-1, 0, 0);
		// -x

		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);
		// +y
		
		glColor3f(0, .5, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, -1, 0);
		// -y

		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);

		glColor3f(0, 0, .5);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, -1);
	glEnd();

	glLineWidth(1.0);
	
	glEnable(GL_LIGHTING);
}

/* void drawFPS()
	This function prints the current FPS to the bottom left corner of the screen.
*/
void drawFPS() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set up for text to screen
	
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0); // white
	char* fps = new char[20];
	sprintf(fps, "%0.2f FPS", 1000.0/(glutGet(GLUT_ELAPSED_TIME)-lastframe));
	glRasterPos2f(0, 0);
	for(int n = 0; n < (int)strlen(fps); n++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, fps[n]);
	}
	glEnable(GL_LIGHTING);
	
	lastframe = glutGet(GLUT_ELAPSED_TIME);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

// Functions for drawing on the screen for overlays and debug

/* void drawLighting()
	Shines the lights from their location.
*/
void drawLighting() {
	glEnable(GL_LIGHTING);
	if(pointLight->isLightOn()) pointLight->shine();
}

/* void drawCameras()
	This function orients the cameras
*/
void drawCameras() {
	arccam->setRadius(15.0);
	arccam->setTheta(M_PI/3.0);
	arccam->setPhi(-2.0*M_PI/3.0);
	arccam->followObject(dummyObject);
	arccam->computeArcballPosition();

	firstpersoncam->setEye(new Point(-5, 2, 0));
	firstpersoncam->setLookAt(new Point(0, 0, 0));
	firstpersoncam->setUp(new Vector(0, 1, 0));
	
	if(CAMERA == ARCCAM) arccam->look();
	else if(CAMERA == FIRSTPERSONCAM) firstpersoncam->look();
	
	// remember to move listeners with the cameras
}

/* void display()
	This function draws the entire scene
*/
void display() {
	glClearColor(0, 0, 0, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	drawCameras();
	drawLighting();

	// bind shaders if necessary

	glPushMatrix();
		drawGrid();
		drawAxes();
		glCallList(skyboxindex);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	// bind and display textures
	// draw non-textured GL objects
	
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
		// draw overlays
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	
	glUseProgram(0); // no GLSL shader program
	
	drawFPS();
	// draw debug info
	
	glutSwapBuffers();
}

/* float distance(float x1, float y1, float z1, float x2, float y2, float z2)
	calculate distance between two points
*/
float distance(float x1, float y1, float z1, float x2, float y2, float z2) {
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
}

/* void timer(int val)
	This function is called 60 times per second.
	It updates variables based on what keys, mouse buttons, or mouse movements
	have happened since the last timer call. Then it redraws the screen.
*/
void timer(int val) {
	// check specific keys and perform actions if pressed
	// move physics
	
	glutTimerFunc(1000.0/MAX_FPS, timer, 0);
	glutPostRedisplay();
}

/* void initLighting()
	This function initializes all the lights in the scene
*/
void initLighting() {
	float inkyBlackness[4] = {0.1, 0.1, 0.1, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, inkyBlackness);
	// very little ambient lighting

	glEnable(GL_LIGHTING);

	pointLight = new PointLight(0);

	GLfloat color[4] = {.5, .5, .5, 1.0};
	pointLight->setDiffuse(color);
	pointLight->setSpecular(color);
	pointLight->setAmbient(color);
	// color
	
	pointLight->setPosition(new Point(0.0, 10.0, 0.0));
	pointLight->turnLightOn();
	// position
}

/* void initMaterials()
	This function initializes all of the materials used in the scene
*/
void initMaterials() {
	defaultwhite = new Material(CSE40166_MATERIAL_WHITE);
}

/* void drawSkybox()
	Displays skybox as background for game
*/
void drawSkybox() {
	glColor4f(1,1,1,1);
	
	glBindTexture(GL_TEXTURE_2D, skybox[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP); {
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-skyboxwidth/2.0, skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, -1, 0);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(skyboxwidth/2.0, skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, -1, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-skyboxwidth/2.0, skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, -1, 0);
		
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(skyboxwidth/2.0, skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, -1, 0);
	};  glEnd();
	glDisable(GL_TEXTURE_2D);
	// top of skybox
	
	glBindTexture(GL_TEXTURE_2D, skybox[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP); {
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-skyboxwidth/2.0, -skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, 0, -1);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(skyboxwidth/2.0, -skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, 0, -1);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-skyboxwidth/2.0, skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, 0, -1);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(skyboxwidth/2.0, skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, 0, -1);
	};  glEnd();
	glDisable(GL_TEXTURE_2D);
	// front
	
	glBindTexture(GL_TEXTURE_2D, skybox[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP); {
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-skyboxwidth/2.0, -skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(1, 0, 0);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-skyboxwidth/2.0, -skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(1, 0, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-skyboxwidth/2.0, skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(1, 0, 0);
		
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-skyboxwidth/2.0, skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(1, 0, 0);
	};  glEnd();
	glDisable(GL_TEXTURE_2D);
	// left
	
	glBindTexture(GL_TEXTURE_2D, skybox[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP); {
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(skyboxwidth/2.0, -skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, 0, 1);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-skyboxwidth/2.0, -skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, 0, 1);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(skyboxwidth/2.0, skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, 0, 1);
		
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-skyboxwidth/2.0, skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, 0, 1);
	};  glEnd();
	glDisable(GL_TEXTURE_2D);
	// back

	glBindTexture(GL_TEXTURE_2D, skybox[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP); {
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(skyboxwidth/2.0, -skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(-1, 0, 0);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(skyboxwidth/2.0, -skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(-1, 0, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(skyboxwidth/2.0, skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(-1, 0, 0);
		
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(skyboxwidth/2.0, skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(-1, 0, 0);
	};  glEnd();
	glDisable(GL_TEXTURE_2D);
	// right	

	glBindTexture(GL_TEXTURE_2D, skybox[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP); {
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-skyboxwidth/2.0, -skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, 1, 0);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(skyboxwidth/2.0, -skyboxwidth/2.0, skyboxwidth/2.0);
		glNormal3f(0, 1, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-skyboxwidth/2.0, -skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, 1, 0);
		
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(skyboxwidth/2.0, -skyboxwidth/2.0, -skyboxwidth/2.0);
		glNormal3f(0, 1, 0);
	};  glEnd();
	glDisable(GL_TEXTURE_2D);
	// bottom
}

/*	void initSkybox(strings skyboxname)
		Initialzies a skybox from textures with the given name.
*/
void initSkybox(string skyboxname) {
 	skybox[0] = SOIL_load_OGL_texture((skyboxname + "/" + skyboxname + "_top.jpg").c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, skybox[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	// top

	skybox[1] = SOIL_load_OGL_texture((skyboxname + "/" + skyboxname + "_front.jpg").c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, skybox[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	// front

	skybox[2] = SOIL_load_OGL_texture((skyboxname + "/" + skyboxname + "_left.jpg").c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, skybox[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	// left	

	skybox[3] = SOIL_load_OGL_texture((skyboxname + "/" + skyboxname + "_back.jpg").c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, skybox[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	// back	

	skybox[4] = SOIL_load_OGL_texture((skyboxname + "/" + skyboxname + "_right.jpg").c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, skybox[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// right

	skybox[5] = SOIL_load_OGL_texture((skyboxname + "/" + skyboxname + "_bottom.jpg").c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, skybox[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// floor		

	skyboxindex = glGenLists(1);
	glNewList(skyboxindex, GL_COMPILE);
		drawSkybox();		
	glEndList();
	// creates displaylist to be used in display()
}

/*	void readTextFile(string filename, char* &output)
		Reads in a text file from filename to a single string stored in output.
*/
void readTextFile(string filename, char* &output) {
    string buf = string("");
    string line;
	
    ifstream in(filename.c_str());
    while(getline(in, line))
        buf += line + "\n";
    output = new char[buf.length()+1];
    strncpy(output, buf.c_str(), buf.length());
    output[buf.length()] = '\0';
	
    in.close();
}

/*	void printLog(GLuint handle)
		prints the log for the program whos handle is passed
*/
void printLog(GLuint handle) {
	int loglength = 0;
	int maxlength;
	
	if(glIsShader(handle)) glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &maxlength);
	else glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &maxlength);
	
	char infolog[maxlength];
	
	if(glIsShader(handle)) glGetShaderInfoLog(handle, maxlength, &loglength, infolog);
	else glGetShaderInfoLog(handle, maxlength, &loglength, infolog);
	// get info log
	
	if(loglength > 0) printf("[INFO]: Shader Handle %d: %s\n", handle, infolog);
}

/*	void setupShaders()
		Compiles and registers our Vertex and Fragment shaders
*/
int setupShaders() {
	// read shaders from glsl files
	// glCreateShader
	// compile shaders
	// attach shaders to program
	// glLinkProgram
	// repeat as necessary
	return 0;
}

/*	GLuint loadTexture(string texname)
		loads the texture with the given file name
*/
GLuint loadTexture(string texname) {
	// SOIL_load_OGL_texture
	// glBindTexture
	// Texture parameters
}

/* void initSounds()
      This function sets up the listener and sources for OpenAL
*/
void initSounds() {
	ALsizei size, freq;
	ALenum format;
	ALvoid* data;
	ALboolean loop;

	// create listeners
	
	// load WAV files to buffers
	// store buffered data to alSources
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	alutInit(&argc, argv);
	glutCreateWindow("Demo");
	// create window
	
	CSE40166Init(true, true); // using GLUT and ALUT

	if(glewInit() != GLEW_OK) {
		printf("Error initializing GLEW\n");
		return 0;
	} // init GLEW and error check
	if(!glewIsSupported("GL_VERSION_2_0")) {
		printf("System does not support OpenGL 2.0 and GLSL\n");
		return 0;
	}
	printf("System supports OpenGL2.0 and GLSL!\n\n");
	// checks GL Version

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	dummyObject = new Object(); // load obj file
	dummyObject->getLocation()->setX(0.0);
	dummyObject->getLocation()->setY(0.0);
	dummyObject->getLocation()->setZ(0.0);
	// load texture for object
	// create objects
	
	arccam = new Camera(ARCBALLCAM);
	arccam->setRadius(15.0);
	arccam->setTheta(M_PI/3.0);
	arccam->setPhi(-2.0*M_PI/3.0);
	arccam->followObject(dummyObject);
	arccam->computeArcballPosition();
	arccam->look();

	firstpersoncam = new Camera(OTHER);
	firstpersoncam->setEye(new Point(-5, 2, 0));
	firstpersoncam->setLookAt(new Point(0, 0, 0));
	firstpersoncam->setUp(new Vector(0, 1, 0));
	firstpersoncam->look();
	// create cameras
	
	// initSkybox(NAME)
	initLighting();
	initMaterials();
	initSounds();
	// other parts of scene
	
	int shaderResult = setupShaders();
	if(shaderResult != 0) {
		printf("Could not open shader files.");
		return 0;
	}
	// setup shaders
	
	glutKeyboardFunc(key_press);
	glutKeyboardUpFunc(key_release);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse_click);
	glutMotionFunc(mouse_motion);
	glutPassiveMotionFunc(mouse_motion);
	glutTimerFunc(1000.0/MAX_FPS, timer, 0);
	// glut callbacks
	
//	glutFullScreen();
	glutMainLoop();
	return 0;
}

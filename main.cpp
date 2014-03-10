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

	#include <QuartzCore/QuartzCore.h> // Apple pointer warp
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
#include <vector>
// C++ libraries

using namespace std;
//using namespace CSE40166;
using namespace OVR;

bool RIFT = false;
// OVR Init

// default keybindings
char key_Forward = 'w';
char key_Backward = 's';
char key_Right = 'd';
char key_Left = 'a';
char key_Change_Camera = 'c';
char key_Enable_Oculus = 'r';

bool fullscreen = false;
int width = 1280, height = 750; // width and height in windowed mode
int y_mouse_offset = 0; // offset to properly calculate mousey in windowed mode
int lastframe = 0; // time last frame was rendered at
float MAX_FPS = 60.0; // FPS cap
float bodyYaw = 3.1*M_PI/2.0, bodyPitch = -.1*M_PI, bodyRoll = 0.0; // body orientation values. Controls camera when not using rift
float headYaw = 3.1*M_PI/2.0, headPitch = -.1*M_PI, headRoll = 0.0; // head orientation values. Controls camera when using rift

float movespeed = 0.1;
float mousespeed = 0.002;
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

CSE40166::Camera* arccam = NULL;
CSE40166::Camera* firstpersoncam = NULL;
#define ARCCAM 0
#define FIRSTPERSONCAM 1
int CAMERA = ARCCAM;
/* Two Cameras
	The Arcball Camera is the standard camera that focuses on one object
	and can zoom and rotate around it. The first person camera looks in the
	direction the user is facing from the user's position.
*/

CSE40166::PointLight* pointLight;
// a single light

CSE40166::Material* defaultwhite;
// materials

CSE40166::Object* dummyObject;
CSE40166::Object* body;
CSE40166::Object* head;
CSE40166::Point* point;
CSE40166::Point* bodyPos;
CSE40166::Point* headPos;
// just an example

GLuint skybox[6];
GLuint skyboxindex;
float skyboxwidth = 2000;
// background textures

GLuint vertshader, fragshader, shaderprogram;
// GLSL shaders

#define delta .1
#define mapwidth 50
#define mapheight 50
#define tilefactor 10.0
double heightmap[(int)(mapwidth/delta)][(int)(mapheight/delta)];
CSE40166::Vector normals[(int)(mapwidth/delta)][(int)(mapheight/delta)];
// height of each point on the grid

GLuint sandtexture; // texture for terrain
GLuint groundList; // call list for terrain

/* void resize(int w, int h)
	GLUT reshpae function. Sets the width and height variables to the
	new width and height of the screen.
*/
void resize(int w, int h) {
	width = w;
	height = h;
	firstmousepos = false;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, width/(float)height, .1, 100000);
#ifdef __APPLE__
		CGPoint warpPoint= CGPointMake(width/2, height/2);
		CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
		CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
		glutWarpPointer(width/x, height/2);
#endif
}

/* void key_press(unsigned char key, int x, int y)
	Updates which keys are pressed. Key is
	the character of the pressed key and x and y are the
	location of the mouse at that time.
*/
void key_press(unsigned char key, int x, int y) {
	if(key == GLUT_ESC_KEY) exit(0);
	if(key == key_Change_Camera) {
		if (CAMERA == ARCCAM) {
			CAMERA = FIRSTPERSONCAM;
		} else {
			CAMERA = ARCCAM;
		}
	}
	if(key == key_Enable_Oculus) {
		RIFT = !RIFT; 
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
		mousedy = (y_mouse_offset + y) - mousey;
	}else {
		y_mouse_offset = (height - (2*y))/2;
	}
	
	bodyPitch -= mousedy * mousespeed;
	bodyYaw -= mousedx * mousespeed;
	if(bodyYaw < -M_PI) bodyYaw += M_PI * 2;
	if(bodyYaw > M_PI) bodyYaw -= M_PI * 2;
	if(bodyPitch < -M_PI * 0.49) bodyPitch = -M_PI * 0.49;
	if(bodyPitch > M_PI * 0.49) bodyPitch = M_PI * 0.49;
	// updates body pitch and yaw based on mouse movement then checks that
	// camera is in range

	mousex = x;
	mousey = y;

	firstmousepos = true;
}

/* void drawGrid()
	Draws a white grid on the ground (for debugging).
*/
void drawGrid() {
	glColor4f(1, 1, 1, 1); // white
	glBindTexture(GL_TEXTURE_2D, sandtexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_TEXTURE_2D);
	for(double z = 0.0; z < mapheight-delta; z += delta) {
		glBegin(GL_TRIANGLE_STRIP);
			for(double x = 0.0; x < mapwidth-delta; x += delta*2) {
				glNormal3f(normals[(int)(x/mapwidth)][(int)((z+delta)/mapheight)].getX(),
						   normals[(int)(x/mapwidth)][(int)((z+delta)/mapheight)].getY(),
						   normals[(int)(x/mapwidth)][(int)((z+delta)/mapheight)].getZ());
				glTexCoord2f(tilefactor*x/mapwidth, tilefactor*(z+delta)/mapheight);
				glVertex3f(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)+1], z+delta-mapheight/2.0);

				glNormal3f(normals[(int)(x/mapwidth)][(int)(z/mapheight)].getX(),
						   normals[(int)(x/mapwidth)][(int)(z/mapheight)].getY(),
						   normals[(int)(x/mapwidth)][(int)(z/mapheight)].getZ());
				glTexCoord2f(tilefactor*x/mapwidth, tilefactor*z/mapheight);
				glVertex3f(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)], z-mapheight/2.0);

				glNormal3f(normals[(int)((x+delta)/mapwidth)][(int)((z+delta)/mapheight)].getX(),
						   normals[(int)((x+delta)/mapwidth)][(int)((z+delta)/mapheight)].getY(),
						   normals[(int)((x+delta)/mapwidth)][(int)((z+delta)/mapheight)].getZ());
				glTexCoord2f(tilefactor*(x+delta)/mapwidth, tilefactor*(z+delta)/mapheight);
				glVertex3f(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)+1], z+delta-mapwidth/2.0);
				
				glNormal3f(normals[(int)((x+delta)/mapwidth)][(int)(z/mapheight)].getX(),
						   normals[(int)((x+delta)/mapwidth)][(int)(z/mapheight)].getY(),
						   normals[(int)((x+delta)/mapwidth)][(int)(z/mapheight)].getZ());
				glTexCoord2f(tilefactor*(x+delta)/mapwidth, tilefactor*z/mapheight);
				glVertex3f(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)], z-mapheight/2.0);
			}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}

/* void drawDebugInfo()
	This function writes a bunch of info to the top of the screen
*/
void drawDebugInfo() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// set up for text to screen
	
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	char *info = new char[1000];
	sprintf(info, "x=%0.1f  y=%0.1f  z=%0.1f  broll=%0.3f  bpitch=%0.3f  byaw=%0.3f mousedx=%0.1d mousedy=%0.1d", bodyPos->getX(), bodyPos->getY(), bodyPos->getZ(), bodyRoll, bodyPitch, bodyYaw, mousedx, mousedy);
	glRasterPos2f(0, .98);
	for(int n = 0; n < (int)strlen(info); n++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[n]);
	}
	glEnable(GL_LIGHTING);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
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

// Can add more functions for drawing on the screen for overlays and debug

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
	
	float dx = cos(bodyPitch)*sin(bodyYaw);
	float dy = sin(bodyPitch);
	float dz = cos(bodyPitch)*cos(bodyYaw);

	if(RIFT) {
		firstpersoncam->setEye(new CSE40166::Point(headPos->getX() + dx, headPos->getY() + dy, headPos->getZ() + dz));
		firstpersoncam->setLookAt(new CSE40166::Point(headPos->getX() + 2*dx,
											headPos->getY() + 2*dy,
											headPos->getZ() + 2*dz));
	}else {
		firstpersoncam->setEye(new CSE40166::Point(bodyPos->getX() + dx, bodyPos->getY() + dy, bodyPos->getZ() + dz));
		firstpersoncam->setLookAt(new CSE40166::Point(bodyPos->getX() + 2*dx,
											bodyPos->getY() + 2*dy,
											bodyPos->getZ() + 2*dz));
	}
	//firstpersoncam->setLookAt(new Point(0,0,0));
	firstpersoncam->setUp(new CSE40166::Vector(0, 1, 0));
	
	if(CAMERA == ARCCAM) arccam->look();
	else if(CAMERA == FIRSTPERSONCAM) firstpersoncam->look();
	
	// remember to move listeners with the cameras
}

/* void display()
	This function draws the entire scene
*/
void display() {
	drawCameras();
	drawLighting();

	// bind shaders if necessary

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glPushMatrix();
		glCallList(groundList);
		drawAxes();
		glCallList(skyboxindex);
	glPopMatrix();

	// bind and display textures
	// draw non-textured GL objects
	
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
		// draw overlays
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

/* void displayMulti()
	This function displays to multiple viewports
*/
void displayMulti() {
	glClearColor(0, 0, 0, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(RIFT) {
		glViewport(0, 0, width/2, height);
		// look from left eye
		display();
		// left eye

		glLoadIdentity();
		glViewport(width/2, 0, width/2, height);
		// look from right eye
		display();
		// right eye
	} // Rift mode displays from two eyes
	else {
		glViewport(0, 0, width, height);
		// basic projections
		display();
	} // regular mode

	glUseProgram(0); // no GLSL shader program

	glViewport(0, 0, width, height);
	drawFPS(); // writes FPS to screen
	drawDebugInfo(); // writes debug info to screen

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
	if(keys[key_Forward]) {
	bodyPos->setX(bodyPos->getX() + movespeed * sin(bodyYaw));
	bodyPos->setZ(bodyPos->getZ() + movespeed * cos(bodyYaw));
	}
	if(keys[key_Backward]) {
	bodyPos->setX(bodyPos->getX() - movespeed * sin(bodyYaw));
	bodyPos->setZ(bodyPos->getZ() - movespeed * cos(bodyYaw));
	}
	if(keys[key_Left]) {
	bodyPos->setX(bodyPos->getX() - movespeed * -cos(bodyYaw));
	bodyPos->setZ(bodyPos->getZ() - movespeed * sin(bodyYaw));
	}
	if(keys[key_Right]) {
	bodyPos->setX(bodyPos->getX() + movespeed * -cos(bodyYaw));
	bodyPos->setZ(bodyPos->getZ() + movespeed * sin(bodyYaw));
	}
	headPos->setX(bodyPos->getX());
	headPos->setY(bodyPos->getY());
	headPos->setZ(bodyPos->getZ());

	width = glutGet(GLUT_WINDOW_WIDTH);
	height = glutGet(GLUT_WINDOW_HEIGHT);
	mousex = width/2;
	mousey = height/2;

#ifdef __APPLE__
	CGPoint warpPoint= CGPointMake(width/2, height/2);
	CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
	CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
	glutWarpPointer(width/x, height/2);
#endif

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

	pointLight = new CSE40166::PointLight(0);

	GLfloat color[4] = {.7, .7, .7, 1.0};
	pointLight->setDiffuse(color);
	pointLight->setSpecular(color);
	pointLight->setAmbient(color);
	// color
	
	pointLight->setPosition(new CSE40166::Point(0.0, 10.0, 0.0));
	pointLight->turnLightOn();
	// position
}

/* void initMaterials()
	This function initializes all of the materials used in the scene
*/
void initMaterials() {
	defaultwhite = new CSE40166::Material(CSE40166::CSE40166_MATERIAL_WHITE);
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

/*	void readKeyBindings()
		Reads in key bindings from KeyBindings.txt and updates bindings map.
*/
void readKeyBindings() {
	bool isBind = false;
	int bindCount = 0;
	char *savedKeys, *tempBind;
	vector<char> bindings;
	readTextFile("KeyBindings.txt", savedKeys);
	tempBind = strtok(savedKeys, " ");
	while(tempBind != NULL) {
		if (isBind) {
			bindings.push_back(*tempBind);
			//printf ("%s\n", tempBind);
			isBind = !isBind;
			bindCount++;
		}else {
			isBind = !isBind;
		}
		tempBind = strtok (NULL, " \n");
	}
	key_Forward = bindings[0];
	key_Backward = bindings[1];
	key_Right = bindings[2];
	key_Left = bindings[3];
	key_Change_Camera = bindings[4];
	key_Enable_Oculus = bindings[5];
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
	GLuint tex = SOIL_load_OGL_texture(texname.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	return tex;
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

/* void initTerrain()
	Randomizes the heights of the terrain and its normals
*/
void initTerrain() {
	for(int x = 0; x < mapwidth/delta; x++) {
		for(int z = 0; z < mapheight/delta; z++) {
//			heightmap[x][z] = ((float)rand()/(float)RAND_MAX) * .5 - .25; // random height between -.25 and .25
			heightmap[x][z] = sin(z/25.0);
//			heightmap[x][z] = 0.0;
		}
	} // heights
	
	/* 
		http://blog.habrador.com/2013/02/how-to-generate-random-terrain.html
		
		heightmaps can be generated in many ways... One simple option for a single-terrain map
		is a hill algorithm (http://www.stuffwithstuff.com/robot-frog/3d/hills/hill.html). This 
		algorithm creates randomly-sized bumps at random points. That would be nice to add some
		variation to a flat stage.
		
		Another option is the Value Noise algorithm (http://lodev.org/cgtutor/randomnoise.html).
		This algorithm generates a random noise array and smoothing it, then overlaying different
		sizes of the smoothed array. This can be applied to different initial images to generate
		things like clouds, marble, or wood. Can also be done in 3D (clouds?)
		
		Perlin Noise is similar to Value Noise but I haven't read up on it yet so I'n not sure where
		it differs. http://www.float4x4.net/index.php/2010/06/generating-realistic-and-playable-terrain-height-maps/
		
		Apparently the collision on uneven terrain can be done with surface/vertex normals, too. Good
		thing I spent so much time calculating those.
	*/
	
	// actually this probably should be an obj//ect. Can I create an object without importing it? Probalby!

	for(double z = 0.0; z < mapheight; z += delta) {
		for(double x = 0.0; x < mapwidth; x += delta) {
			CSE40166::Point* myPoint = new CSE40166::Point(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)], z-mapheight/2.0);
			CSE40166::Point* adjPoints[6] = { NULL }; // max 6 adjacent points
			if(x > 0.0 && z > 0.0) // up and left
				adjPoints[0] = new CSE40166::Point(x-delta-mapwidth/2.0, heightmap[(int)(x/delta)-1][(int)(z/delta)-1], z-delta-mapheight/2.0);
			if(z > 0.0) // up
				adjPoints[1] = new CSE40166::Point(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)-1], z-delta-mapheight/2.0);
			if(x < mapwidth-delta) // right
				adjPoints[2] = new CSE40166::Point(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)], z-mapheight/2.0);
			if(x < mapwidth-delta && z < mapheight-delta) // down right
				adjPoints[3] = new CSE40166::Point(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)+1], z+delta-mapheight/2.0);
			if(z < mapheight-delta) // down
				adjPoints[4] = new CSE40166::Point(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)+1], z+delta-mapheight/2.0);
			if(x > 0.0) // left
				adjPoints[5] = new CSE40166::Point(x-delta-mapwidth/2.0, heightmap[(int)(x/delta)-1][(int)(z/delta)], z-mapheight/2.0);
			// all adjacent points

			normals[(int)(x/delta)][(int)(z/delta)] = CSE40166::Vector(0, 0, 0);
			if(adjPoints[0] != NULL && adjPoints[1] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[1])-*myPoint, *(adjPoints[0])-*myPoint);
			if(adjPoints[1] != NULL && adjPoints[2] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[2])-*myPoint, *(adjPoints[1])-*myPoint);
			if(adjPoints[2] != NULL && adjPoints[3] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[3])-*myPoint, *(adjPoints[2])-*myPoint);
			if(adjPoints[3] != NULL && adjPoints[4] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[4])-*myPoint, *(adjPoints[3])-*myPoint);
			if(adjPoints[4] != NULL && adjPoints[5] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[5])-*myPoint, *(adjPoints[4])-*myPoint);
			if(adjPoints[5] != NULL && adjPoints[0] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[0])-*myPoint, *(adjPoints[5])-*myPoint);
			normals[(int)(x/delta)][(int)(z/delta)].normalize();
		}
	} // calculates normals
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	alutInit(&argc, argv);
	glutCreateWindow("Demo");
	glutSetCursor(GLUT_CURSOR_NONE); 
	if(fullscreen) glutFullScreen();
#ifdef __APPLE__
		CGPoint warpPoint= CGPointMake(width/2, height/2);
		CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
		CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
		glutWarpPointer(width/x, height/2);
#endif
	// create window and center mouse
	
	CSE40166::CSE40166Init(true, true); // using GLUT and ALUT

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

	readKeyBindings(); // loads key bindings

	OVR::System::Init(); // init Oculus Rift

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	dummyObject = new CSE40166::Object(); // load obj file
	dummyObject->getLocation()->setX(0.0);
	dummyObject->getLocation()->setY(0.0);
	dummyObject->getLocation()->setZ(0.0);
	body = new CSE40166::Object();
	bodyPos = body->getLocation();
	bodyPos->setX(-5.0);
	bodyPos->setY(2.0);
	bodyPos->setZ(0);
	head = new CSE40166::Object();
	headPos = head->getLocation();
	headPos->setX(-5.0);
	headPos->setY(2.0);
	headPos->setZ(0);
	// load texture for object
	// create objects
	
	arccam = new CSE40166::Camera(CSE40166::ARCBALLCAM);
	arccam->setRadius(15.0);
	arccam->setTheta(M_PI/3.0);
	arccam->setPhi(-2.0*M_PI/3.0);
	arccam->followObject(dummyObject);
	arccam->computeArcballPosition();
	arccam->look();


	firstpersoncam = new CSE40166::Camera(CSE40166::OTHER);
	firstpersoncam->setEye(new CSE40166::Point(-5, 2, 0));
	firstpersoncam->setLookAt(new CSE40166::Point(0, 0, 0));
	firstpersoncam->setUp(new CSE40166::Vector(0, 1, 0));
	firstpersoncam->look();
	// create cameras
	
	sandtexture = loadTexture("sand.jpg");
	initTerrain();
	groundList = glGenLists(1);
	glNewList(groundList, GL_COMPILE);
		drawGrid();
	glEndList();
	// initializes a (hopefully) randomly generated terrain
	
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
	glutDisplayFunc(displayMulti);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse_click);
	glutPassiveMotionFunc(mouse_motion);
	glutTimerFunc(1000.0/MAX_FPS, timer, 0);
	// glut callbacks
	
//	glutFullScreen();
	glutMainLoop();
	return 0;
}

//
//  OpenGLTemplate.cpp
//  
//
//  Created by Jeremy Vercillo on 2/27/14.
//
//

#define USE_FRAMEBUFFER

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

#include <SOIL/SOIL.h> // Simple OpenGL Image Library
#include <CSE40166/CSE40166.h> // This is a library that our professor made for us. It has Cameras, Materials, Lights, Objects, and more!
#include "globals.h"
#include "Terrain.h"
#include "Voronoi.h"

#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
// C++ libraries

using namespace std;
//using namespace CSE40166;
using namespace OVR;
using namespace OVR::Util::Render;

bool RIFT = false;
// OVR Init

// default keybindings
char key_Forward = 'w';
char key_Backward = 's';
char key_Right = 'd';
char key_Left = 'a';
char key_Change_Camera = 'c';
char key_Enable_Oculus = 'r';
char key_Toggle_Fullscreen = 'o';

bool fullscreen = false;
int width = 1280, height = 800; // width and height in windowed mode
int y_mouse_offset = 0; // offset to properly calculate mousey in windowed mode
int lastframe = 0; // time last frame was rendered at
float MAX_FPS = 60.0; // FPS cap
float bodyYaw = 3.1*M_PI/2.0, bodyPitch = -.1*M_PI, bodyRoll = 0.0; // body orientation values. Controls camera when not using rift
float headYaw = 3.1*M_PI/2.0, headPitch = -.1*M_PI, headRoll = 0.0; // head orientation values. Controls camera when using rift
float LastSensorYaw =  3.1*M_PI/2.0;

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

CSE40166::Camera* topdowncam = NULL;
CSE40166::Camera* firstpersoncam = NULL;
#define TOPDOWNCAM 0
#define FIRSTPERSONCAM 1
int CAMERA = FIRSTPERSONCAM;
// Two Cameras

CSE40166::SpotLight* light;
CSE40166::Point* lightpos;
CSE40166::Vector* lightdir;
// a single light

CSE40166::Material* defaultwhite;
// materials

CSE40166::Object* body;
CSE40166::Object* head;
CSE40166::Point* point;
CSE40166::Point* bodyPos;
CSE40166::Point* headPos;
// just an example

GLuint skybox[6];
GLuint skyboxindex;
float skyboxwidth = 2000;
GLuint skydome;
GLuint skydomeindex;
float skydomeradius = 100;
float skydomerot = 0.0;
// background textures

GLuint passthroughShader, barrelShader, simpleShader; // GLSL shaders

#ifdef USE_FRAMEBUFFER
GLuint framebuffer, depthbuffer, renderedTexture; // for rendering
#endif

GLuint texture; // texture for terrain
GLuint groundList; // call list for terrain

DeviceManager* pManager;
HMDDevice* pHMD;
HMDInfo hmdInfo;
SensorDevice *pSensor;
SensorFusion *SFusion;
StereoConfig sconfig;
// Oculus rift variables

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

#ifdef USE_FRAMEBUFFER
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	// resizes renderedTexture
	
	glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// resizes renderbuffer
#endif
	
#ifdef __APPLE__
	CGPoint warpPoint= CGPointMake(width/2, height/2);
	CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
	CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
	glutWarpPointer(width/2, height/2);
#endif
}

/* void key_press(unsigned char key, int x, int y)
	Updates which keys are pressed. Key is
	the character of the pressed key and x and y are the
	location of the mouse at that time.
*/
void key_press(unsigned char key, int x, int y) {
	if(key == GLUT_ESC_KEY) {
		// OVR System Destroy?
		exit(0);
	}
	if(key == key_Change_Camera) {
		if (CAMERA == TOPDOWNCAM) CAMERA = FIRSTPERSONCAM;
		else CAMERA = TOPDOWNCAM;
	}
	if(key == key_Enable_Oculus) {
		RIFT = !RIFT; 
	}
	if (key == key_Toggle_Fullscreen) {
		fullscreen = !fullscreen;
		firstmousepos = false;
		y_mouse_offset = 0;
		if (fullscreen) {
			glutFullScreen();
			width = glutGet(GLUT_WINDOW_WIDTH);
            height = glutGet(GLUT_WINDOW_HEIGHT);
		}else {
			glutReshapeWindow(1280,800);
            width = glutGet(GLUT_WINDOW_WIDTH);
            height = glutGet(GLUT_WINDOW_HEIGHT);
		}
#ifdef __APPLE__
		CGPoint warpPoint= CGPointMake(width/2, height/2);
		CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
		CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
		glutWarpPointer(width/2, height/2);
#endif
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

/* void drawDirectionVectors()
	This function draws the directional vector of the body and head
*/
void drawDirectionVectors() {
	glDisable(GL_LIGHTING);
	glColor3f(0, 0, 1); // blue is body
	glLineWidth(5);
	glBegin(GL_LINES);
		glVertex3f(bodyPos->getX(), bodyPos->getY(), bodyPos->getZ());
		glVertex3f(bodyPos->getX() + 5*cos(bodyPitch)*sin(bodyYaw),
				   bodyPos->getY() + 5*sin(bodyPitch),
				   bodyPos->getZ() + 5*cos(bodyPitch)*cos(bodyYaw));
	glEnd();
	glColor3f(1, 0, 0); // red is head
	glBegin(GL_LINES);
		glVertex3f(headPos->getX(), headPos->getY(), headPos->getZ());
		glVertex3f(headPos->getX() + 5*cos(headPitch)*sin(headYaw),
				   headPos->getY() + 5*sin(headPitch),
				   headPos->getZ() + 5*cos(headPitch)*cos(headYaw));
	glEnd();
	glLineWidth(1);
	glEnable(GL_LIGHTING);
}

/* void drawCrosshairs()
	This function draws crosshairs where the head and body are looking
*/
void drawCrosshairs() {
	glDisable(GL_LIGHTING);
	glColor3f(.6, 0.0, 0.0);
	glLineWidth(3);
	glPushMatrix();
		glTranslatef(headPos->getX() + 100*cos(headPitch)*sin(headYaw),
				     headPos->getY() + 100*sin(headPitch),
				     headPos->getZ() + 100*cos(headPitch)*cos(headYaw));
		
		float modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				if(i == j) modelview[i*4+j] = 1.0;
				else modelview[i*4+j] = 0.0;
			}
		}
		glLoadMatrixf(modelview);
		// billboarding
		
		glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3f(3.0, 0, 0);
			glVertex3f(1.0, 0, 0);
			
			glVertex3f(-1.0, 0, 0);
			glVertex3f(-3.0, 0, 0);
			
			glVertex3f(0, 3.0, 0);
			glVertex3f(0, 1.0, 0);
			
			glVertex3f(0, -1.0, 0);
			glVertex3f(0, -3.0, 0);
		glEnd();
	glPopMatrix();
	glPushMatrix();
		glTranslatef(bodyPos->getX() + 100*cos(bodyPitch)*sin(bodyYaw),
				     bodyPos->getY() + 100*sin(bodyPitch),
				     bodyPos->getZ() + 100*cos(bodyPitch)*cos(bodyYaw));
		
		float modelview2[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview2);
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				if(i == j) modelview2[i*4+j] = 1.0;
				else modelview2[i*4+j] = 0.0;
			}
		}
		glLoadMatrixf(modelview2);
		// billboarding
		
		glBegin(GL_LINES);
			glColor3f(0, 0, 1);
			glVertex3f(3.0, 0, 0);
			glVertex3f(1.0, 0, 0);
			
			glVertex3f(-1.0, 0, 0);
			glVertex3f(-3.0, 0, 0);
			
			glVertex3f(0, 3.0, 0);
			glVertex3f(0, 1.0, 0);
			
			glVertex3f(0, -1.0, 0);
			glVertex3f(0, -3.0, 0);
		glEnd();
	glPopMatrix();
	glLineWidth(1);
	glEnable(GL_LIGHTING);
}

/* void drawAxes()
	Draws x, y, and z axes in red, green, and blue. For debugging.
*/
void drawAxes() {	
	glLineWidth(2.0);
	
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
	lightpos->setX(bodyPos->getX());
	lightpos->setY(25.0);
	lightpos->setZ(bodyPos->getZ());

	glEnable(GL_LIGHTING);
	if(light->isLightOn()) light->shine();
}

/* void drawCameras()
	This function orients the cameras
*/
void drawCameras() {
	if(RIFT) {
		float dx = cos(headPitch)*sin(headYaw);
		float dy = sin(headPitch);
		float dz = cos(headPitch)*cos(headYaw);

		firstpersoncam->setEye(new CSE40166::Point(headPos->getX() + dx, headPos->getY() + dy, headPos->getZ() + dz));
		firstpersoncam->setLookAt(new CSE40166::Point(headPos->getX() + 2*dx,
											headPos->getY() + 2*dy,
											headPos->getZ() + 2*dz));
	}
	else {
		float dx = cos(bodyPitch)*sin(bodyYaw);
		float dy = sin(bodyPitch);
		float dz = cos(bodyPitch)*cos(bodyYaw);

		firstpersoncam->setEye(new CSE40166::Point(bodyPos->getX() + dx, bodyPos->getY() + dy, bodyPos->getZ() + dz));
		firstpersoncam->setLookAt(new CSE40166::Point(bodyPos->getX() + 2*dx,
											bodyPos->getY() + 2*dy,
											bodyPos->getZ() + 2*dz));
	}
	
	if(CAMERA == TOPDOWNCAM) topdowncam->look();
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
		glPushMatrix();
			skydomerot += .01;
			if(skydomerot >= 360) skydomerot = 0;
			glRotatef(-skydomerot, 0, 1, 0);
			glCallList(skydomeindex);
		glPopMatrix();
//		glCallList(skyboxindex);
	glPopMatrix();

	// bind and display textures
	// draw non-textured GL objects
	
	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glPushMatrix();
		if(CAMERA == TOPDOWNCAM) {
			glDisable(GL_LIGHTING);
			drawAxes();
		}
	glPopMatrix();
}

/* void screenshot(char filename[60], int x, int y)
	Takes a x-by-y pixel screenshot of the current draw buffer and saves it in filename.
*/
void screenshot (char filename[160],int x, int y) {
	// get the image data
	long imageSize = x * y * 3;
	unsigned char *data = new unsigned char[imageSize];
	glReadPixels(0,0,x,y, GL_BGR,GL_UNSIGNED_BYTE,data);// split x and y sizes into bytes
	int xa= x % 256;
	int xb= (x-xa)/256;int ya= y % 256;
	int yb= (y-ya)/256;//assemble the header
	unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,24,0};
	// write header and data to file
	fstream File(filename, ios::out | ios::binary);
	File.write (reinterpret_cast<char *>(header), sizeof (char)*18);
	File.write (reinterpret_cast<char *>(data), sizeof (char)*imageSize);
	File.close();
	
	delete[] data;
	data=NULL;
}

/* void displayMulti()
	This function displays to multiple viewports
*/
void displayMulti() {
	glUseProgram(0); // no GLSL shader program

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// clears the main screen
	
#ifdef USE_FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // all drawing renders to this texture
	glClearColor(0, 0, 0, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// clears the framebuffer
#endif

	glUseProgram(passthroughShader);

#ifndef USE_FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(RIFT) {
		/*glViewport(0, 0, width/2, height);
		// look from left eye
		display();
		// left eye

		glLoadIdentity();
		glViewport(width/2, 0, width/2, height);
		// look from right eye
		display();
		// right eye*/
		glViewport(0, 0, width, height);
		display();
#ifdef USE_FRAMEBUFFER
//	screenshot("test.tga", width, height);
	int uniform_WindowSize = glGetUniformLocation(barrelShader, "WindowSize");
	glUseProgram(barrelShader); // should use barrel transform shader
	glUniform2f(uniform_WindowSize, width, height);
	
/*	GLuint lenscenter = glGetUniformLocation(barrelShader, "LensCenter");
	float x = 0.0, y = 0.0, w = 1.0, h = 1.0;
	float lenscenterarray[2] = {x + (w + Distortion.XCenterOffset * 0.5f)*0.5f, y + h*0.5f};
	glUniform2fv(lenscenter, 1, lenscenterarray);
*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif 
	} // Rift mode displays from two eyes
	else {
		glViewport(0, 0, width, height);
		// basic projections
		display();
#ifdef USE_FRAMEBUFFER
//	screenshot("test.tga", width, height);

	glUseProgram(simpleShader); // should use barrel transform shader
	
/*	GLuint lenscenter = glGetUniformLocation(barrelShader, "LensCenter");
	float x = 0.0, y = 0.0, w = 1.0, h = 1.0;
	float lenscenterarray[2] = {x + (w + Distortion.XCenterOffset * 0.5f)*0.5f, y + h*0.5f};
	glUniform2fv(lenscenter, 1, lenscenterarray);
*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif 
	} // regular mode

/*#ifdef USE_FRAMEBUFFER
//	screenshot("test.tga", width, height);

	glUseProgram(barrelShader); // should use barrel transform shader
	*/
/*	GLuint lenscenter = glGetUniformLocation(barrelShader, "LensCenter");
	float x = 0.0, y = 0.0, w = 1.0, h = 1.0;
	float lenscenterarray[2] = {x + (w + Distortion.XCenterOffset * 0.5f)*0.5f, y + h*0.5f};
	glUniform2fv(lenscenter, 1, lenscenterarray);
*/
/*	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif */

	glViewport(0, 0, width, height);
#ifdef USE_FRAMEBUFFER
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, .99);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
		glTexCoord2f(0, 1); glVertex3f(0, height, 0);
		glTexCoord2f(1, 1); glVertex3f(width, height, 0);
		glTexCoord2f(1, 0); glVertex3f(width, 0, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
#endif // renders framebuffer to screen

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
		if(CAMERA == FIRSTPERSONCAM) {
			drawDirectionVectors();
			drawCrosshairs();
		}
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
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

//	width = glutGet(GLUT_WINDOW_WIDTH);
//	height = glutGet(GLUT_WINDOW_HEIGHT);
	mousex = width/2;
	mousey = height/2;

#ifdef __APPLE__
	CGPoint warpPoint= CGPointMake(width/2, height/2);
	CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
	CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
	glutWarpPointer(width/2, height/2);
#endif

	glutTimerFunc(1000.0/MAX_FPS, timer, 0);
	glutPostRedisplay();
    
    if (pSensor) {
    Quatf hmdOrient = SFusion->GetOrientation();

    float yaw = 0.0f;
    hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &headPitch, &headRoll);

    headYaw += (yaw - LastSensorYaw);
    LastSensorYaw = yaw;

    cout << "headYaw: " << headYaw << " headPitch: " << headPitch << " headRoll: " << headRoll << endl;
    // NOTE: We can get a matrix from orientation as follows:
    // Matrix4f hmdMat(hmdOrient);

    // Test logic - assign quaternion result directly to view:
    // Quatf hmdOrient = SFusion.GetOrientation();
    // View = Matrix4f(hmdOrient.Inverted()) * Matrix4f::Translation(-EyePos);
	}
}

/* void initLighting()
	This function initializes all the lights in the scene
*/
void initLighting() {
	float inkyBlackness[4] = {0.1, 0.1, 0.1, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, inkyBlackness);
	// very little ambient lighting

	glEnable(GL_LIGHTING);

	light = new CSE40166::SpotLight(0);

	GLfloat color[4] = {.8, .8, .8, 1.0};
	light->setDiffuse(color);
	light->setSpecular(color);
	light->setAmbient(color);
	// color
	
	lightpos = new CSE40166::Point(0.0, 25.0, 0.0);
	lightdir = new CSE40166::Vector(0.0, -1.0, 0.0);
	light->setPosition(lightpos);
	light->setDirection(lightdir);
	light->setCutoff(45);
//	light->setExponent(10);

	light->turnLightOn();
	// position
}

/* void initMaterials()
	This function initializes all of the materials used in the scene
*/
void initMaterials() {
	defaultwhite = new CSE40166::Material(CSE40166::CSE40166_MATERIAL_WHITE);
}

/* void drawSkydome()
	Displays a skydome as background - alternative to skybox
*/
void drawSkydome() {
	glPushMatrix();
		glRotatef(-90, 1, 0, 0);

		GLUquadricObj* quad = gluNewQuadric();
		gluQuadricNormals(quad, GL_SMOOTH);
		gluQuadricTexture(quad, GL_TRUE);
		gluQuadricOrientation(quad, GLU_INSIDE); 
		glBindTexture(GL_TEXTURE_2D, skydome);
		gluSphere(quad, skydomeradius, 100, 100);
		gluDeleteQuadric(quad);
	glPopMatrix();
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

/* void initSkydome(string skydomename)
	Initializes a skydome from the texture with the given name.
*/
void initSkydome(string skydomename) {
	skydome = loadTexture(skydomename);

	skydomeindex = glGenLists(1);
	glNewList(skydomeindex, GL_COMPILE);
		drawSkydome();
	glEndList();
	// creates displaylist to be used in display()
}

/* void initSkybox(string skyboxname)
	Initialzies a skybox from textures with the given name.
*/
void initSkybox(string skyboxname) {
 	skybox[0] = loadTexture((skyboxname + "/" + skyboxname + "_top.jpg").c_str());
 	skybox[1] = loadTexture((skyboxname + "/" + skyboxname + "_front.jpg").c_str());
 	skybox[2] = loadTexture((skyboxname + "/" + skyboxname + "_left.jpg").c_str());
 	skybox[3] = loadTexture((skyboxname + "/" + skyboxname + "_back.jpg").c_str());
 	skybox[4] = loadTexture((skyboxname + "/" + skyboxname + "_right.jpg").c_str());
 	skybox[5] = loadTexture((skyboxname + "/" + skyboxname + "_floor.jpg").c_str());

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
	key_Toggle_Fullscreen = bindings[6];
}

/*	void printLog(GLuint handle)
		prints the log for the program whos handle is passed
*/
void printLog(GLuint object) {
	GLint log_length = 0;
	if(glIsShader(object)) glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if(glIsProgram(object)) glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else {
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}
	
	char* log = (char*)malloc(log_length);
	
	if(glIsShader(object)) glGetShaderInfoLog(object, log_length, NULL, log);
	else if(glIsProgram(object)) glGetProgramInfoLog(object, log_length, NULL, log);
	
	fprintf(stderr, "%s", log);
	free(log);
}

/*	void setupShaders()
		Compiles and registers our Vertex and Fragment shaders
*/
int setupShaders(char* v, char* f) {
	char *vertexShaderString;
	char *fragmentShaderString;
	readTextFile(v, vertexShaderString);
	readTextFile(f, fragmentShaderString);
	// reads each text file into a string
	
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	// cretes shaders
	
	glShaderSource(vertShader, 1, (const char**)&vertexShaderString, NULL);
	printLog(vertShader);
	glShaderSource(fragShader, 1, (const char**)&fragmentShaderString, NULL);
	printLog(fragShader);
	// sends each program to the GPU
	
	delete[] vertexShaderString;
	delete[] fragmentShaderString;
	// frees up memory
	
	glCompileShader(vertShader);
	printLog(vertShader);
	glCompileShader(fragShader);
	printLog(fragShader);
	// compiles shaders on the GPU
	
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	printLog(vertShader);
	glAttachShader(program, fragShader);
	printLog(fragShader);
	// creates a program and attaches the shaders
	
	glLinkProgram(program);
	printLog(program);
	// link all programs together on GPU
	
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if(!link_ok) fprintf(stderr, "glLinkProgram Error ");
	
	return program;
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

void configOVR() {
	pManager = DeviceManager::Create();
	pHMD = pManager->EnumerateDevices<HMDDevice>().CreateDevice();
	pSensor = pManager->EnumerateDevices<SensorDevice>().CreateDevice();

	if(pHMD) {
        pSensor = pHMD->GetSensor();
		pHMD->GetDeviceInfo(&hmdInfo);
		cout << "Connected to " << hmdInfo.DisplayDeviceName << endl;

        // Retrieve relevant profile settings. 
//        Profile* pUserProfile;
//		Player ThePlayer;
//		pUserProfile = pHMD->GetProfile();
//        if (pUserProfile)
//        {
//            ThePlayer.UserEyeHeight = pUserProfile->GetEyeHeight();
//            ThePlayer.EyePos.y = ThePlayer.UserEyeHeight;
//        }
	}
	else cout << "Could not connect to device." << endl;
	// connect to device

    sconfig.SetFullViewport(Viewport(0, 0, width, height));
    sconfig.SetStereoMode(Stereo_LeftRight_Multipass);
	sconfig.SetHMDInfo(hmdInfo);
	// init stereo mode
}

int main(int argc, char* argv[]) {
	srand(time(NULL));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	alutInit(&argc, argv);
	glutCreateWindow("Demo");
	glutSetCursor(GLUT_CURSOR_NONE); 
	if(fullscreen) glutFullScreen();
	
/*#ifdef __APPLE__
	CGPoint warpPoint= CGPointMake(width/2, height/2);
	CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
	CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
	glutWarpPointer(width/2, height/2);
#endif*/
	// create window and center mouse
	
	CSE40166::CSE40166Init(true, true); // using GLUT and ALUT

	glewExperimental = GL_TRUE; // has to be the newer version of GLEW
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
	configOVR(); // connect to the Oculus Rift


	// We need to attach sensor to SensorFusion object for it to receive
    // body frame messages and update orientation. SFusion.GetOrientation()
    // is used in OnIdle() to orient the view.
    if (pSensor) SFusion->AttachToSensor(pSensor);
   // SFusion->SetDelegateMessageHandler(this);
	//SFusion->SetPredictionEnabled(true); 

	glEnable(GL_DEPTH_TEST);

//	glEnable(GL_SMOOTH);
//	glHint(GL_POLYGON_SMOOTH, GL_NICEST);
//	glEnable(GL_POLYGON_SMOOTH);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_MULTISAMPLE_ARB);	

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	body = new CSE40166::Object();
	bodyPos = body->getLocation();
	bodyPos->setX(0.0);
	bodyPos->setY(2.0);
	bodyPos->setZ(0.0);
	head = new CSE40166::Object();
	headPos = head->getLocation();
	headPos->setX(0.0);
	headPos->setY(2.0);
	headPos->setZ(0.0);
	// load texture for object
	// create objects
	
	topdowncam = new CSE40166::Camera(CSE40166::OTHER);
	topdowncam->setEye(new CSE40166::Point(.1, 75.0, 0.0));
	topdowncam->setLookAt(new CSE40166::Point(0, 0, 0));
	topdowncam->setUp(new CSE40166::Vector(0, 1, 0));
	topdowncam->look();

	firstpersoncam = new CSE40166::Camera(CSE40166::OTHER);
	firstpersoncam->setEye(new CSE40166::Point(0, 0, 0));
	firstpersoncam->setLookAt(new CSE40166::Point(-1, 0, 0));
	firstpersoncam->setUp(new CSE40166::Vector(0, 1, 0));
	firstpersoncam->look();
	// create cameras
	
//	initSkybox("space");
	initSkydome("sky.jpg");
	initLighting();
	initMaterials();
	initSounds();
	// other parts of scene

	Voronoi* v = new Voronoi(100);

	texture = loadTexture("grass.jpg");
	Terrain* t = new Terrain(texture);
	groundList = glGenLists(1);
	glNewList(groundList, GL_COMPILE);
		t->draw();
//		t->drawNormals();
	glEndList();
	// initializes a (hopefully) randomly generated terrain
	
	passthroughShader = setupShaders("BasicShader.v.glsl", "BasicShader.f.glsl");
	barrelShader = setupShaders("BarrelShader.v.glsl", "BarrelShader.f.glsl");
	simpleShader = setupShaders("SimpleShader.v.glsl", "SimpleShader.f.glsl");
	// setup shaders

	glutKeyboardFunc(key_press);
	glutKeyboardUpFunc(key_release);
	glutDisplayFunc(displayMulti);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse_click);
	glutPassiveMotionFunc(mouse_motion);
	glutTimerFunc(1000.0/MAX_FPS, timer, 0);
	// glut callbacks

#ifdef USE_FRAMEBUFFER
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// framebuffer
	
	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// texture to render to

	glGenRenderbuffers(1, &depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
	// depth render buffer

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
	// attach framebuffer to texture

	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Error creating framebuffer.\n");
		return 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// reset to regular drawing
#endif

//	glutFullScreen();
#ifdef __APPLE__
	CGPoint warpPoint= CGPointMake(width/2, height/2);
	CGWarpMouseCursorPosition(CGPointMake(width/2, height/2));
	CGAssociateMouseAndMouseCursorPosition(true);
#endif
#ifdef __linux__
	glutWarpPointer(width/2, height/2);
#endif
	glutMainLoop();
	
	return 0;
}

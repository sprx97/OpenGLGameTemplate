#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include <algorithm>
#include "globals.h"
#include "Voronoi.h"

using namespace std;

bool sortPoints(_Point2D p1, _Point2D p2) { return p1.x < p2.x; }
Voronoi::Voronoi(int numpoints) {
	for(int n = 0; n < numpoints; n++) {
		_Point2D pt(((float)rand()/RAND_MAX) * mapwidth - (mapwidth/2.0), ((float)rand()/RAND_MAX) * mapheight - (mapheight/2.0));
		sites.push_back(pt);
	}
	sort(sites.begin(), sites.end(), sortPoints);
}

/*
void drawPolygonMap() {
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glDisable(GL_COLOR_MATERIAL);
	for(int n = 0; n < numpoints; n++) {
//		cout << randPoints[n][0] << " " << randPoints[n][1] << endl;
		glTranslatef(randPoints[n][0], 5, randPoints[n][1]);
		GLUquadricObj* pt = gluNewQuadric();
		gluSphere(pt, .25, 5, 5);
		gluDeleteQuadric(pt);
		glTranslatef(-randPoints[n][0], -5, -randPoints[n][1]);
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
}
*/

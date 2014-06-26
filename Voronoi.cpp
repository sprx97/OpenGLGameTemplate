#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include <iostream>
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

	for(int n = 0; n < sites.size(); n++) {
		events.push_back(sites[n]);
	}
	sweepline = -mapwidth/2.0;
}

void Voronoi::step() {
	if(events.size() == 0) return;
	sweepline = events[0].x;
	_Point2D next = events[0];
	events.pop_front();
	// moves sweepline and gets next event

	for(int n = 0; n < beachline.size(); n++) {
		beachline[n].recalculate(beachline[n].getFocus(), sweepline);
//		cout << beachline[n].a << " " << beachline[n].b << " " << beachline[n].c << endl;
	} // resizes all current parabolae
	cout << endl;

//	beachline.push_back(_Parabola(_Point2D(13.315, .833), -3.35));
	float offset = .001;
	beachline.push_back(_Parabola(_Point2D(next.x - offset, next.z), sweepline+offset));
}

void Voronoi::draw() {
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glDisable(GL_COLOR_MATERIAL);
	for(int n = 0; n < sites.size(); n++) {
//		cout << sites[n].x << " " << sites[n].z << endl;
		glTranslatef(sites[n].x, 5, sites[n].z);
		GLUquadricObj* pt = gluNewQuadric();
		gluSphere(pt, .25, 5, 5);
		gluDeleteQuadric(pt);
		glTranslatef(-sites[n].x, -5, -sites[n].z);
	} // draws the sites

	glColor4f(0.0, 1.0, 0.0, 1.0);
	glBegin(GL_LINES);
		for(int z = -mapwidth/2.0; z < mapwidth/2.0; z++) {
			glVertex3f(sweepline, 5, z);
			glVertex3f(sweepline, 5, z+1);
		}
	glEnd();

	for(int n = 0; n < beachline.size(); n++) {
		if(n == beachline.size()-1) continue;
		beachline[n].draw();
	}
	glEnable(GL_COLOR_MATERIAL);

}

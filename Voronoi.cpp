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
	beachline = NULL;
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

_Point2D Voronoi::circumcenter(_Point2D p1, _Point2D p2, _Point2D p3) {
	_Point2D point1((p1.x+p2.x)/2.0, (p1.z+p2.z)/2.0);
	_Point2D point2((p1.x+p3.x)/2.0, (p1.z+p3.z)/2.0);
	float slope1 = -1/((p2.z-p1.z)/(p2.x-p1.x));
	float slope2 = -1/((p3.z-p1.z)/(p3.x-p1.x));
	// point-slope form of two lines - their intersection is the circumcenter

	float m1 = slope1;
	float m2 = slope2;
	float b1 = point1.z - m1*point1.x;
	float b2 = point2.z - m2*point2.x;
	// standard form

	float x = -(b2-b1)/(m2-m1);
	float y = -(m1*b2 - b1*m2)/(m2-m1);
	return _Point2D(x, y);
}

void Voronoi::step() {
	if(events.size() == 0) return;
	sweepline = events[0].x;
	_Point2D next = events[0];
	events.pop_front();
	// moves sweepline and gets next event

	VoronoiArc* newarc = new VoronoiArc(_Point2D(next.x, next.z), sweepline);
	if(beachline == NULL) beachline = newarc;
	else {
		beachline->recalculate(beachline->getFocus(), sweepline);
		beachline->add(newarc);
	}
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
	// sweepline

	if(beachline != NULL) beachline->draw(-mapwidth/2.0, mapwidth/2.0);

//	glColor4f(0.0, 1.0, 1.0, 1.0);
//	for(int n = 1; n < beachline.size(); n++) {
//		vector<_Point2D> roots = beachline[n].getIntersection(beachline[n-1]);
//		roots[0].draw();
//		roots[1].draw();		
//	} // intersections

	glEnable(GL_COLOR_MATERIAL);

}

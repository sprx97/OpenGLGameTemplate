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

	float offset = .01;
	if(beachline.size() == 0) {
		beachline.push_back(_Parabola(_Point2D(next.x - offset, next.z), sweepline+offset));
	}
	else {
		int pushed = -1;
		for(int n = 0; n < beachline.size(); n++) {
			beachline[n].recalculate(beachline[n].getFocus(), sweepline);
			if(pushed == -1 && next.z < beachline[n].getFocus().z) {
				beachline.insert(beachline.begin()+n, _Parabola(_Point2D(next.x - offset, next.z), sweepline+offset));
				pushed = n;
			}
		} // resizes all current parabolae
		if(pushed == -1) {
			beachline.push_back(_Parabola(_Point2D(next.x - offset, next.z), sweepline+offset));
			pushed = beachline.size()-1;
		} // puts at the end

		for(int n = 1; n < beachline.size(); n++) {
			vector<_Point2D> roots = beachline[n].getIntersection(beachline[n-1]);
			if(beachline[n-1].getSlope(roots[0].z) < beachline[n].getSlope(roots[0].z)) {
//				cout << n << ":\t(" << roots[0].x << ", " << roots[0].z << ")" << endl;
//				cout << "\t" << beachline[n-1].getSlope(roots[0].x) << " " << beachline[n].getSlope(roots[0].x) << endl;

				beachline[n-1].end = roots[0].z;
				beachline[n].start = roots[0].z;
			}			
			if(beachline[n-1].getSlope(roots[1].z) < beachline[n].getSlope(roots[1].z)) {
//				cout << "\t(" << roots[1].x << ", " << roots[1].z << ")" << endl;
//				cout << "\t" << beachline[n-1].getSlope(roots[1].x) << " " << beachline[n].getSlope(roots[1].x) << endl;

				beachline[n-1].end = roots[1].z;
				beachline[n].start = roots[1].z;
			}
		}





//		cout << endl;

//			cout << beachline[beachline.size()-1].getSlope(roots[0].x) << " " << beachline[beachline.size()-2].getSlope(roots[0].x) << endl;

//			beachline[beachline.size()-1].start = intersect.x;
//			beachline[beachline.size()-2].end = intersect.x;
	} // creates beachline in order of x position
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
		beachline[n].draw();
	} // parabolas

	for(int n = 1; n < beachline.size(); n++) {
		vector<_Point2D> roots = beachline[n].getIntersection(beachline[n-1]);
		glColor4f(0.0, 1.0, 1.0, 1.0);
		roots[0].draw();
		glColor4f(1.0, 1.0, 0.0, 1.0);
		roots[1].draw();		
	} // intersections

	glEnable(GL_COLOR_MATERIAL);

}

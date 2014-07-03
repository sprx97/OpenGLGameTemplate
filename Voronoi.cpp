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

vector<_Parabola*> allarcs;
/* UGLY GLOBAL*/

bool sortPoints(_Point2D p1, _Point2D p2) { return p1.x < p2.x; }
Voronoi::Voronoi(int numpoints) {
//	beachline = NULL;
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

float distForm(_Point2D p1, _Point2D p2) {
	return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.z-p2.z)*(p1.z-p2.z));
}
_Circle Voronoi::circle(_Point2D p1, _Point2D p2, _Point2D p3) {
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
	_Point2D center(x, y);

	return _Circle(center, distForm(p1, center));
}

void Voronoi::step() {
	circle(sites[0], sites[1], sites[2]);

	if(events.size() == 0) return;
	sweepline = events[0].x;
	_Point2D next = events[0];
	events.pop_front();
	// moves sweepline and gets next event

	for(int n = 0; n < allarcs.size(); n++) allarcs[n]->recalculate(allarcs[n]->getFocus(), sweepline);

	_Parabola* newarc = new _Parabola(_Point2D(next.x, next.z), sweepline);
	if(allarcs.size() == 0) {
		newarc->start = -mapwidth/2.0;
		newarc->end = mapwidth/2.0;
		allarcs.push_back(newarc);
	}
	else {
		bool broken = false;
		for(int n = 0; n < allarcs.size(); n++) {
			if(newarc->getFocus().z < allarcs[n]->getFocus().z) {
				allarcs.insert(allarcs.begin()+n, newarc);
				broken = true;
				break;
			}		
		}
		if(!broken) allarcs.push_back(newarc);
	}

	setStartEnd();
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
	for(int n = 0; n < circleEvents.size(); n++) {
		glTranslatef(circleEvents[n].x, 5, circleEvents[n].z);
		GLUquadricObj* pt = gluNewQuadric();
		gluSphere(pt, .25, 5, 5);
		gluDeleteQuadric(pt);
		glTranslatef(-circleEvents[n].x, -5, -circleEvents[n].z);
	} // draws the circle events

	glColor4f(0.0, 0.0, 1.0, 1.0);
	glBegin(GL_LINES);
		for(int z = -mapwidth/2.0; z < mapwidth/2.0; z++) {
			glVertex3f(sweepline, 5, z);
			glVertex3f(sweepline, 5, z+1);
		}
	glEnd();
	// sweepline

	for(int n = 0; n < allarcs.size(); n++) allarcs[n]->draw();

	glEnable(GL_COLOR_MATERIAL);
}

void Voronoi::setStartEnd() {
	vector<_Parabola*> newallarcs;

	if(allarcs.size() == 0) return;

	_Parabola* myarc = allarcs[0];
	for(int n = 1; n < allarcs.size(); n++) {
		if(allarcs[n]->isInfinite()) continue;
		else if(myarc->isInfinite()) myarc = allarcs[n];
		else if(myarc->getVal(-mapwidth/2.0) < allarcs[n]->getVal(-mapwidth/2.0)) myarc = allarcs[n];
	} // start at far left - highest arc

	int nextArcIndex = -1;
	float last_point = -mapwidth/2.0; // start
	float max_point = mapwidth/2.0;
	myarc->start = last_point;
	while(last_point < max_point) {
		float intersect = max_point;
		for(int n = 0; n < allarcs.size(); n++) {
			if(allarcs[n] == myarc) continue;
			vector<_Point2D> roots = myarc->getIntersection(*(allarcs[n]));
			for(int r = 0; r < roots.size(); r++) {
				if(roots[r].x > last_point &&  roots[r].x < intersect && !allarcs[n]->isInfinite()) {
					intersect = roots[r].x;
					nextArcIndex = n;
				}
			}
		} // finds the next intersection along the arc

		if(nextArcIndex == -1) {
			if(myarc->isInfinite()) break;
			myarc->end = max_point;
			_Parabola* cpy = (_Parabola*)malloc(sizeof(_Parabola));
			memcpy(cpy, myarc, sizeof(_Parabola));
			newallarcs.push_back(cpy);
			break;
		} // safety valve

		myarc->end = intersect;
		_Parabola* cpy = (_Parabola*)malloc(sizeof(_Parabola));
		memcpy(cpy, myarc, sizeof(_Parabola));
		newallarcs.push_back(cpy);
		myarc = allarcs[nextArcIndex];
		myarc->start = intersect;
		last_point = intersect;
	} // jumps arcs until it reaches the end

	for(int n = 0; n < allarcs.size(); n++) {
		if(allarcs[n]->isInfinite()) {
			_Parabola* cpy = (_Parabola*)malloc(sizeof(_Parabola));
			memcpy(cpy, allarcs[n], sizeof(_Parabola));	
			
			if(newallarcs.size() == 0) {
				newallarcs.push_back(cpy);
			}
			else {
				bool broken = false;
				int intersect = -1;
				for(int n = 0; n < newallarcs.size(); n++) {
					if(cpy->getFocus().z < newallarcs[n]->getFocus().z) {
						newallarcs.insert(newallarcs.begin()+n, cpy);
						intersect = n;
						broken = true;
						break;
					}		
				}
				if(!broken) {
					intersect = newallarcs.size()-1;
					newallarcs.push_back(cpy);
				}

				int lowest = 0;
				for(int n = 1; n < newallarcs.size(); n++) {
					if(newallarcs[lowest]->isInfinite()) lowest = n;
					if(newallarcs[n]->isInfinite()) continue;
					if(cpy->getIntersection(*(newallarcs[lowest]))[0].z < cpy->getIntersection(*(newallarcs[n]))[0].z) {
						lowest = n;
					}
				}
				if(newallarcs[lowest]->isInfinite()) continue;
//				cout << "highest at " <<  cpy->getIntersection(*(newallarcs[lowest]))[0].x << " " << cpy->getIntersection(*(newallarcs[lowest]))[0].z << endl;

				float breakpoint = cpy->getIntersection(*(newallarcs[lowest]))[0].x;
				
				_Parabola* righthalf = (_Parabola*)malloc(sizeof(_Parabola));
				memcpy(righthalf, newallarcs[lowest], sizeof(_Parabola));
				newallarcs[lowest]->end = breakpoint;
				righthalf->start = breakpoint;
				newallarcs.insert(newallarcs.begin()+intersect+1, righthalf);
			}
			// need to split the intersecting parabola
		}
	} // draws infinite parabola(s)

	allarcs = newallarcs; // replaces
}

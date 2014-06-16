#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include <math.h>
#include <vector>
#include <deque>

using namespace std;

enum Orientation {
	HORIZONTAL,
	VERTICAL
};

struct _Point2D {
	float x, z;
	_Point2D(float x, float z) {
		this->x = x;
		this->z = z;
	}
};

struct _Parabola {
	float a, b, c;
	float start, end;
	Orientation orientation;

	_Parabola(float a, float b, float c, Orientation o) {
		this->a = a;
		this->b = b;
		this->c = c;
		orientation = o;
		if(o == VERTICAL) {
			start = -mapwidth/2.0;
			end = mapwidth/2.0;
		}
		else {
			start = -mapheight/2.0;
			end = mapheight/2.0;
		}
	}

	_Parabola(_Point2D focus, _Point2D vertex) {
		float h = vertex.x;
		float k = vertex.z;

		float p = focus.x - vertex.x;
		orientation = VERTICAL;
		if(p == 0) {
			p = focus.z - vertex.z;
			orientation = HORIZONTAL;
		}
		a = 1/(4*p);
		b = -2*a*h;
		c = a*h*h + k;
	}

	_Parabola(_Point2D focus, float directrix, Orientation o) {
		if(o == VERTICAL) _Parabola(focus, _Point2D(focus.x-directrix, focus.z));
		else _Parabola(focus, _Point2D(focus.x, focus.z-directrix));
	}
	
	_Point2D getVertex() {
		if(orientation == HORIZONTAL) {
			float h = -b/(2*a);
			float k = a*h*h + b*h + c;
			return _Point2D(h, k);
		}
		else {
			float k = -b/(2*a);
			float h = a*k*k + b*k + c;
			return _Point2D(h, k);
		}
	}

	_Point2D getFocus() {
		if(orientation == HORIZONTAL) {
			float h = -b/(2*a);
			float k = (a*h*h + b*h + c);
			float p = (1/(4*a));
			return _Point2D(h, k+p);
		}
		else {
			float k = -b/(2*a);
			float h = (a*k*k + b*k + c);
			float p = (1/(4*a));
			return _Point2D(h+p, k);
		}
	}

	float getDirectrix() {
		if(orientation == HORIZONTAL) {
			float h = -b/(2*a);
			float k = (a*h*h + b*h + c);
			float p = (1/(4*a));
			return (k - p);
		}
		else {
			float k = -b/(2*a);
			float h = (a*k*k + b*k + c);
			float p = (1/(4*a));
			return (h-p);
		}
	}

	float getVal(float x) {
		return (a*x*x + b*x + c);
	}

	void draw() {
		glColor4f(0.0, 0.0, 1.0, 1.0);
		glLineWidth(2.0);
		if(orientation == HORIZONTAL) {
			glBegin(GL_LINES);
				for(int x = start; x < end; x++) {
					float y = getVal(x);

					float x2 = x + 1;
					float y2 = getVal(x2);

					glVertex3f(x, 5, y);
					glVertex3f(x2, 5, y2);
				} // parabola
				for(int x = -mapwidth/2.0; x < mapwidth/2.0; x++) {
					glVertex3f(x, 5, getDirectrix());
					glVertex3f(x+1, 5, getDirectrix());
				} // draws directrix
			glEnd();

			glTranslatef(getFocus().x, 5, getFocus().z);
			GLUquadricObj* f = gluNewQuadric();
			gluSphere(f, .25, 5, 5);
			gluDeleteQuadric(f);			
			glTranslatef(-getFocus().x, -5, -getFocus().z);
			// draws focus
		} 
		else {
			glBegin(GL_LINES);
				for(int x = start; x < end; x++) {
					float y = getVal(x);

					float x2 = x+1;
					float y2 = getVal(x2);

					glVertex3f(y, 5, x);
					glVertex3f(y2, 5, x2);
				} // draws parabola
				for(int x = -mapheight/2.0; x < mapheight/2.0; x++) {
					glVertex3f(getDirectrix(), 5, x);
					glVertex3f(getDirectrix(), 5, x+1);
				} // draws directrix
			glEnd();
	
			glTranslatef(getFocus().x, 5, getFocus().z);
			GLUquadricObj* f = gluNewQuadric();
			gluSphere(f, .25, 5, 5);
			gluDeleteQuadric(f);			
			glTranslatef(-getFocus().x, -5, -getFocus().z);
			// draws focus
		}
	}
};

struct _Edge {
	_Point2D start, end;
};

class Voronoi {
	public:
		Voronoi(int numpoints);
		void draw();
		void step();
	private:
		deque<_Point2D> events;
		vector<_Point2D> sites;
		vector<_Edge> edges;
		vector<_Parabola> beachline;
		float sweepline;
};

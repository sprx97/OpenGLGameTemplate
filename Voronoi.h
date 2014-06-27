#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include <iostream>
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

	bool equals(_Point2D other) {
		return ((this->x == other.x) && (this->z == other.z));
	}

	void draw() {
		glDisable(GL_COLOR_MATERIAL);
		glBegin(GL_LINES);
			glVertex3f(x-.25, 5, z-.25);
			glVertex3f(x+.25, 5, z+.25);

			glVertex3f(x-.25, 5, z+.25);
			glVertex3f(x+.25, 5, z-.25);
		glEnd();
		glEnable(GL_COLOR_MATERIAL);
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

	void construct(_Point2D focus, _Point2D vertex) {
		float k = vertex.x;
		float h = vertex.z;

		float p = focus.x - vertex.x;
		orientation = VERTICAL;
		if(p == 0) {
			p = focus.z - vertex.z;
			orientation = HORIZONTAL;
			float temp = h;
			h = k;
			k = temp;
		}
		a = 1/(4*p);
		b = -2*a*h;
		c = a*h*h + k;
//		cout << a << " " << b << " " << c << endl;		
	}

	_Parabola(_Point2D focus, _Point2D vertex) {
		construct(focus, vertex);
	}

	_Parabola(_Point2D focus, float directrix, Orientation o = VERTICAL) {
		orientation = o;
		if(o == VERTICAL) {
			construct(focus, _Point2D((focus.x+directrix)/2.0, focus.z));
			start = -mapwidth/2.0;
			end = mapwidth/2.0;
		}
		else {
			construct(focus, _Point2D(focus.x, (focus.z+directrix)/2.0));
			start = -mapheight/2.0;
			end = mapheight/2.0;
		}
	}

	void recalculate(_Point2D focus, float directrix) {
		if(orientation == VERTICAL) 
			construct(focus, _Point2D((focus.x+directrix)/2.0, focus.z));
		else
			construct(focus, _Point2D(focus.x, (focus.z+directrix)/2.0));
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

	float getSlope(float x) {
		return (2*a*x + b);
	}

	vector<_Point2D> getIntersection(_Parabola other) {
		float A = other.a - this->a;
		float B = other.b - this->b;
		float C = other.c - this->c;

		float x1 = (-B + sqrt(B*B - 4*A*C))/(2*A);
		float y1 = getVal(x1);

		float x2 = (-B - sqrt(B*B - 4*A*C))/(2*A);
		float y2 = getVal(x2);

		vector<_Point2D> roots;
		roots.push_back(_Point2D(y1, x1));
		roots.push_back(_Point2D(y2, x2));

		return roots; // return appropriate root
	}

	void draw() {
//		cout << a << " " << b << " " << c << endl;

		glColor4f(0.0, 0.0, 1.0, 1.0);
		glLineWidth(2.0);
		if(orientation == HORIZONTAL) {
			glBegin(GL_LINES);
				for(float x = start; x < end; x += .01) {
					float y = getVal(x);

					float x2 = x + .01;
					float y2 = getVal(x2);

					glVertex3f(x, 5, y);
					glVertex3f(x2, 5, y2);
				} // parabola
/*				for(int x = -mapwidth/2.0; x < mapwidth/2.0; x++) {
					glVertex3f(x, 5, getDirectrix());
					glVertex3f(x+1, 5, getDirectrix());
				} // draws directrix
*/
			glEnd();

/*			glTranslatef(getFocus().x, 5, getFocus().z);
			GLUquadricObj* f = gluNewQuadric();
			gluSphere(f, .25, 5, 5);
			gluDeleteQuadric(f);			
			glTranslatef(-getFocus().x, -5, -getFocus().z);
*/
			// draws focus
		} 
		else {
			glBegin(GL_LINES);
				for(float x = start; x < end; x += .01) {
					float y = getVal(x);

					float x2 = x + .01;
					float y2 = getVal(x2);

					glVertex3f(y, 5, x);
					glVertex3f(y2, 5, x2);
				} // draws parabola
/*				for(int x = -mapheight/2.0; x < mapheight/2.0; x++) {
					glVertex3f(getDirectrix(), 5, x);
					glVertex3f(getDirectrix(), 5, x+1);
				} // draws directrix
*/
			glEnd();
	
/*			glTranslatef(getFocus().x, 5, getFocus().z);
			GLUquadricObj* f = gluNewQuadric();
			gluSphere(f, .25, 5, 5);
			gluDeleteQuadric(f);			
			glTranslatef(-getFocus().x, -5, -getFocus().z);
			// draws focus
*/
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

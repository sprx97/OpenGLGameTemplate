#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include <math.h>
#include <vector>

using namespace std;

struct _Point2D {
	float x, z;
	_Point2D(float x, float z) {
		this->x = x;
		this->z = z;
	}
};

struct _Parabola {
	float a, b, c;

	_Parabola(float a, float b, float c) {
		this->a = a;
		this->b = b;
		this->c = c;
	}

	_Parabola(_Point2D focus, float directrix) {
		// calculate a, b, and c
	}

	_Parabola(_Point2D focus, _Point2D vertex) {
		// calculate a, b, and c
	}

	_Point2D getVertex() {
		float h = -b/(2*a);
		float k = a*h*h + b*h + c;

		return _Point2D(h, k);
	}

	_Point2D getFocus() {
		float h = -b/(2*a);
		float k = (a*h*h + b*h + c);
		float p = (1/(4*a));
	
		return _Point2D(h, k + p);
	}

	float getDirectrix() {
		float h = -b/(2*a);
		float k = (a*h*h + b*h + c);
		float p = (1/(4*a));
	
		return (k - p);
	}

	float getY(float x) {
		return (a*x*x + b*x + c);
	}

	void draw() {
		glColor4f(0.0, 0.0, 1.0, 1.0);
		glLineWidth(2.0);
		glBegin(GL_LINES);
			for(int x = -mapwidth/2.0; x < mapwidth/2.0; x++) {
				float y = getY(x);

				float x2 = x + 1;
				float y2 = getY(x2);

				glVertex3f(x, 5, y);
				glVertex3f(x2, 5, y2);
			} // draws parabola

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
};

class Voronoi {
	public:
		Voronoi(int numpoints);
		void draw();
	private:
		vector<_Point2D> sites;
};

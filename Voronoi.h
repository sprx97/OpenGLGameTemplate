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

	_Parabola(_Point2D focus, float directrix, int orientation) {
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

	void draw() {

	}
};

class Voronoi {
	public:
		Voronoi(int numpoints);
		void draw();
	private:
		vector<_Point2D> sites;
};

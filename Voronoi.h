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

class Voronoi {
	public:
		Voronoi(int numpoints);
	private:
		vector<_Point2D> sites;
};

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
	
	_Point2D() {
		x = 0.0;
		z = 0.0;
	}

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
	_Point2D vertex, focus;
	float directrix;

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

		vertex = getVertex();
		focus = getFocus();
		directrix = getDirectrix();
	}

	void construct(_Point2D focus, _Point2D vertex) {
		if(focus.equals(vertex)) {
			this->focus = focus;
			this->vertex = vertex;
			directrix = focus.x;	
			return;		
		}

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

		this->vertex = vertex;
		this->focus = focus;
		this->directrix = getDirectrix();
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
		return vertex;

//		if(orientation == HORIZONTAL) {
//			float h = -b/(2*a);
//			float k = a*h*h + b*h + c;
//			return _Point2D(h, k);
//		}
//		else {
//			float k = -b/(2*a);
//			float h = a*k*k + b*k + c;
//			return _Point2D(h, k);
//		}
	}

	_Point2D getFocus() {
		return focus;

//		if(orientation == HORIZONTAL) {
//			float h = -b/(2*a);
//			float k = (a*h*h + b*h + c);
//			float p = (1/(4*a));
//			return _Point2D(h, k+p);
//		}
//		else {
//			float k = -b/(2*a);
//			float h = (a*k*k + b*k + c);
//			float p = (1/(4*a));
//			return _Point2D(h+p, k);
//		}
	}

	float getDirectrix() {
		if(focus.equals(vertex)) return focus.x;

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
		if(vertex.equals(focus)) return directrix;
		return (a*x*x + b*x + c);
	}

	float getSlope(float x) {
		if(vertex.equals(focus)) return 999999;
		return (2*a*x + b);
	}

	vector<_Point2D> getIntersection(_Parabola other) {
		vector<_Point2D> infroots;
		if(vertex.equals(focus)) {
//			cout << focus.z << " " << other.getVal(focus.z) << endl;
			infroots.push_back(_Point2D(focus.z, other.getVal(focus.z)));
			infroots.push_back(_Point2D(focus.z, other.getVal(focus.z)));
		}
		if(other.vertex.equals(other.focus)) {
//			cout << other.focus.z << " " << getVal(other.focus.z) << endl;
			infroots.push_back(_Point2D(other.focus.z, getVal(other.focus.z)));
			infroots.push_back(_Point2D(other.focus.z, getVal(other.focus.z)));
		}
		if(infroots.size() > 2) {
			cout << "Trying to find the intersection of two infinite parabolas" << endl;
			exit(0);
		}
		else if (infroots.size() != 0) return infroots;

		float A = other.a - this->a;
		float B = other.b - this->b;
		float C = other.c - this->c;

		float x1 = (-B + sqrt(B*B - 4*A*C))/(2*A);
		float y1 = getVal(x1);

		float x2 = (-B - sqrt(B*B - 4*A*C))/(2*A);
		float y2 = getVal(x2);

		vector<_Point2D> roots;
		roots.push_back(_Point2D(x1, y1));
		roots.push_back(_Point2D(x2, y2));

		if(roots[0].x > roots[1].x) {
			_Point2D temp = roots[0];
			roots[0] = roots[1];
			roots[1] = temp;
		} // orders roots from left to right

		return roots; // return appropriate root
	}

	void draw() {
/*		cout << a << " " << b << " " << c << endl;
		cout << focus.x << " " << focus.z << endl;
		cout << vertex.x << " " << vertex.z << endl;
		cout << directrix << endl;
		cout << focus.equals(vertex) << endl << endl;
*/
		glColor4f(0.0, 0.0, 1.0, 1.0);
		glLineWidth(2.0);
		if(focus.equals(vertex)) {
			glBegin(GL_LINES);
			if(orientation == VERTICAL) {
//				float connect = parent->getVal(vertex.z);
				for(float x = vertex.x-25; x < vertex.x; x += .01) {
					glVertex3f(x, 5, vertex.z);
					glVertex3f(x+.01, 5, vertex.z);
				}
			}
			else {
//				float connect = parent->getVal(vertex.x);
				for(float z = vertex.z-25; z < vertex.z; z += .01) {
					glVertex3f(vertex.x, 5, z);
					glVertex3f(vertex.x, 5, z+.01);
				}
			}
			glEnd();
			return;	
		} // straight line placeholder if infinite parabola

		if(orientation == HORIZONTAL) {
			glBegin(GL_LINES);
				for(float x = start; x < end; x += .01) {
					float y = getVal(x);

					float x2 = x + .01;
					float y2 = getVal(x2);

					glVertex3f(x, 5, y);
					glVertex3f(x2, 5, y2);
				} // parabola
			glEnd();
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
			glEnd();
		}
	}
};

struct VoronoiArc : public _Parabola {
	vector<VoronoiArc*> children;
	VoronoiArc* parent;

	VoronoiArc(float a, float b, float c, Orientation o) : _Parabola(a, b, c, o) {}
	VoronoiArc(_Point2D focus, _Point2D vertex) : _Parabola(focus, vertex) {}
	VoronoiArc(_Point2D focus, float directrix, Orientation o = VERTICAL) : _Parabola(focus, directrix, o) {}
	// same constructors as a parabola

	void draw(float s, float e) {
		start = end = s;
		for(int n = 0; n < children.size(); n++) {
			vector<_Point2D> roots = getIntersection(*(children[n]));

//			cout << roots[0].x << "\t" << roots[0].z << endl;
//			cout << roots[1].x << "\t" << roots[1].z << endl;
//			cout << endl;

			if(roots[0].x > end) end = roots[0].x; // stop where new parabola is
			_Parabola::draw();
			start = roots[1].x; // restart where new one reconnects
			if(start > e) start = e;
			children[n]->draw(end, start);

			// roots[0] will always start the parabola and roots[1]
			// will always end it. Because the children are "tighter"
		}
		end = e;
		_Parabola::draw(); // draw last segment

//		_Parabola::draw();
//		for(int n = 0; n < children.size(); n++) children[n]->draw(s, e);
		// draw self only in places not covered by children
	}

	void recalculate(_Point2D focus, float sweepline) {
		_Parabola::recalculate(focus, sweepline);
		for(int n = 0; n < children.size(); n++) children[n]->recalculate(children[n]->getFocus(), sweepline);
	}

	void add(VoronoiArc* child) {		
		int highest = -1; // default if this node is the highest collision
		float intersect = getIntersection(*child)[0].z;

		for(int n = 0; n < children.size(); n++) {
			if(children[n]->getIntersection(*child)[0].z > intersect) {
				intersect = children[n]->getIntersection(*child)[0].z;
				highest = n;
			}
		}

		if(highest == -1) {
			if(children.size() == 0) {
				children.push_back(child);
				child->parent = this;
			}
			else {
				bool broken = false;
				for(int n = 0; n < children.size(); n++) {
					if(child->getVertex().z < children[n]->getVertex().z) {
						children.insert(children.begin()+n, child);
						child->parent = this;
						broken = true;
						break;
					}
				}
				if(!broken) {
					children.push_back(child);
					child->parent = this;
				}
			}
		}
		else children[highest]->add(child);
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
		_Point2D circumcenter(_Point2D p1, _Point2D p2, _Point2D p3);
	private:
		deque<_Point2D> events;
		vector<_Point2D> sites;
		vector<_Edge> edges;
		VoronoiArc* beachline; // root
		float sweepline;
};

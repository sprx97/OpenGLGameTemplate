#ifndef _VECTOR_H_
#define _VECTOR_H_ 1

#include "PointBase.h"

namespace CSE40166 {
	
	class Vector : public PointBase {
	public:

		// CONSTRUCTORS / DESTRUCTORS
		Vector();

		Vector( int a, int b, int c );
		Vector( double a, double b, double c );

		// MISCELLANEOUS
		double magSq();
		double mag();
		void normalize();
		double at(int i);

		Vector& operator+=(Vector rhs);
		Vector& operator-=(Vector rhs);
		Vector& operator*=(float rhs);
		Vector& operator/=(float rhs);

		/* call glNormal3f( a, b, c ) */
		void glNormal();
	};

	// RELATED OPERATORS

	Vector operator*(Vector a, Vector b);
	Vector operator*(Vector a, float f);
	Vector operator/(Vector a, float f);
	Vector operator/(float f, Vector p);
	Vector operator*(float f, Vector a);
	Vector operator+(Vector a, Vector b);
	Vector operator-(Vector a, Vector b);
	bool operator==(Vector a, Vector b);
	bool operator!=(Vector a, Vector b);

	Vector cross(Vector a, Vector b);
	double dot(Vector a, Vector b);
	
}

#endif

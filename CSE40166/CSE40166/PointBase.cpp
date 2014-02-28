#include "PointBase.h"
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>

namespace CSE40166 {

	double PointBase::getX() { return x; }

	void PointBase::setX( double x ) { this->x = x; }

	double PointBase::getY() { return y; }

	void PointBase::setY( double y ) { this->y = y; }

	double PointBase::getZ() { return z; }

	void PointBase::setZ( double z ) { this->z = z; }

	double PointBase::getW() { return w; }

	double* PointBase::asArray() {
		double* _vec = (double*)malloc( 4 * sizeof(double) );
		_vec[0] = x;
		_vec[1] = y;
		_vec[2] = z;
		_vec[3] = w;
		return _vec;
	}
	
	/* deprecated */
	double* PointBase::asVector() {
		return asArray();
	}

	char* PointBase::toString() {
		char* buf = (char*)malloc( sizeof(char*) * 255 );
		sprintf( buf, "(%f, %f, %f)", x, y, z );
		return buf;
	}

}

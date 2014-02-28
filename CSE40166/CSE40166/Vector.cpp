#include "Vector.h"
#include <math.h>

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

namespace CSE40166 {
	
	Vector::Vector() { x = y = z = 0; }

	Vector::Vector( int a, int b, int c ) { 
		x = a;
		y = b;
		z = c;
	}

	Vector::Vector( double a, double b, double c ) { 
		x = a;
		y = b;
		z = c;
	}

	// OPERATOR OVERLOADS

	Vector operator*(Vector a, Vector b) {
		return Vector( a.getX()*b.getX(), a.getY()*b.getY(), a.getZ()*b.getZ() );
	}

	Vector operator*(Vector a, float f) {
		return Vector(a.getX()*f,a.getY()*f,a.getZ()*f);
	}

	Vector operator/(Vector a, float f) {
		return Vector(a.getX()/f,a.getY()/f,a.getZ()/f);
	}

	Vector operator/(float f, Vector a) {
		return Vector(a.getX()/f,a.getY()/f,a.getZ()/f);
	}

	Vector operator*(float f, Vector a) {
		return Vector(a.getX()*f,a.getY()*f,a.getZ()*f);
	}

	Vector operator+(Vector a, Vector b) {
		return Vector(a.getX()+b.getX(), a.getY()+b.getY(), a.getZ()+b.getZ());
	}

	Vector operator-(Vector a, Vector b) {
		return Vector(a.getX()-b.getX(), a.getY()-b.getY(), a.getZ()-b.getZ());
	}


	Vector& Vector::operator+=(Vector rhs) {
		this->setX( this->getX() + rhs.getX() );
		this->setY( this->getY() + rhs.getY() );
		this->setZ( this->getZ() + rhs.getZ() );
		return *this;
	}


	Vector& Vector::operator-=(Vector rhs) {
		this->setX( this->getX() - rhs.getX() );
		this->setY( this->getY() - rhs.getY() );
		this->setZ( this->getZ() - rhs.getZ() );
		return *this;
	}

	Vector& Vector::operator*=(float rhs) {
		this->setX( this->getX() * rhs );
		this->setY( this->getY() * rhs );
		this->setZ( this->getZ() * rhs );
		return *this;
	}

	Vector& Vector::operator/=(float rhs) {
		this->setX( this->getX() / rhs );
		this->setY( this->getY() / rhs );
		this->setZ( this->getZ() / rhs );
		return *this;
	}

	bool operator==(Vector a, Vector b) {
		return ( a.getX() == b.getX() && a.getY() == b.getY() && a.getZ() == b.getZ() );
	}

	bool operator!=(Vector a, Vector b) {
		return !( a == b );
	}

	Vector cross(Vector a, Vector b) {
		Vector c;
		c.setX( a.getY()*b.getZ() - a.getZ()*b.getY() );
		c.setY( b.getX()*a.getZ() - a.getX()*b.getZ() );
		c.setZ( a.getX()*b.getY() - a.getY()*b.getX() );
		return c;
	}

	double dot(Vector a, Vector b) {
		return a.getX()*b.getX() + a.getY()*b.getX() + a.getZ()*b.getZ();
	}


	// MEMBER FUNCTIONS

	double Vector::magSq() {
		return x*x + y*y + z*z;
	}

	double Vector::mag() {
		double t = magSq();
		if(t <= 0.0)
			return 0;
		return sqrt(t);
	}

	void Vector::normalize() {
		double m = mag();
		x /= m;
		y /= m;
		z /= m;
	}

	double Vector::at(int i) {
		if(i == 0)  return x;
		if(i == 1)  return y;
		if(i == 2)  return z;
		return -1;
	}

	void Vector::glNormal() {
		glNormal3f( x, y, z );
	};

}

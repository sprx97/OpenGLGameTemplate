#ifndef _TERRAIN_H
#define _TERRAIN_H

#include "globals.h"
#include <math.h>

using namespace std;

struct _Vector {
	float x, y, z;

	_Vector() {
		this->x = 0.0;
		this->y = 0.0;
		this->z = 0.0;
	}

	_Vector(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	_Vector normalize() {
		float mag = x*x + y*y + z*z;
		if(mag <= 0) mag = 0;
		else mag = sqrt(mag);
		// calculate magnitude

		x /= mag;
		y /= mag;
		z /= mag;
		// normalize
	}

	_Vector operator += (_Vector other) {
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;

		return *this;
	}
};

_Vector cross(_Vector a, _Vector b);

struct _Point {
	float x, y, z;

	_Point(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	_Vector operator - (_Point other) {
		return _Vector(x-other.x, y-other.y, z-other.z);
	}
};

class Terrain { // Extend object
	public:
		Terrain(GLuint tex, float f = 5, float p = .125, float o = 3, float a = 3);
		float cubicPolate(float v0, float v1, float v2, float v3, float frac);
		float interpolate(int x, int z);
		void generateNoise(int s1, int s2);
		float smoothNoise(double x, double z);
		void calculateNormals();		
		void draw();
		void drawNormals();

	private:
		double heightmap[(int)(mapwidth/delta)+1][(int)(mapheight/delta)+1];
		double randomNoise[(int)(mapwidth/delta)+1][(int)(mapheight/delta)+1];
		_Vector normals[(int)(mapwidth/delta)+1][(int)(mapheight/delta)+1];

		int frequency;
		float persistance;
		float octaves;
		float amplitude;
		GLuint texture;
};

#endif

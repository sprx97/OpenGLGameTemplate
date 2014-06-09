#ifndef _TERRAIN_H
#define _TERRAIN_H

#include <CSE40166/CSE40166.h>
#include "globals.h"

using namespace std;

class Terrain { // Extend object
	public:
		Terrain(float f, float p, float o, float a, GLuint tex);
		float cubicPolate(float v0, float v1, float v2, float v3, float frac);
		float interpolate(int x, int z);
		void generateNoise(int s1, int s2);
		float smoothNoise(double x, double z);
		void calculateNormals();		
		void draw();

	private:
		double heightmap[(int)(mapwidth/delta)+1][(int)(mapheight/delta)+1];
		double randomNoise[(int)(mapwidth/delta)+1][(int)(mapheight/delta)+1];
		CSE40166::Vector normals[(int)(mapwidth/delta)+1][(int)(mapheight/delta)+1];

		int frequency;
		float persistance;
		float octaves;
		float amplitude;
		GLuint texture;
};

#endif

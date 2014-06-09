#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "Terrain.h"
#include <algorithm>
#include <math.h>

using namespace std;

_Vector cross(_Vector a, _Vector b) {
	return _Vector(a.y*b.z-a.z*b.y, b.x*a.z-a.x*b.z, a.x*b.y-a.y*b.x);
}

/* Helper function for bicubic interpolation*/
float Terrain::cubicPolate(float v0, float v1, float v2, float v3, float frac) {
	float A = (v3-v2)-(v0-v1);
	float B = (v0-v1)-A;
	float C = v2-v0;
	float D = v1;

	return A*pow(frac, 3) + B*pow(frac, 2) + C*frac + D;
} 

/* float interpolate(float x, float z)
	finds the y value of a point on the grid by interpolating its nearest neighbors
*/
float Terrain::interpolate(int x, int z) {
	float xfrac = (x%frequency) / (float)frequency;
	float zfrac = (z%frequency) / (float)frequency;

	int xL = x - x%frequency;
	int zL = z - z%frequency;

	float ndata[4][4];
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			int xval = xL + frequency*(i-1);
			int zval = zL + frequency*(j-1);
			xval = max(0, xval);
			zval = max(0, zval);
			xval = min((double)xval, mapwidth/delta);
			zval = min((double)zval, mapheight/delta);
			ndata[i][j] = randomNoise[xval][zval];
		}
	} // gets the 4x4 grid of points surrounding

	float x1 = cubicPolate(ndata[0][0], ndata[1][0], ndata[2][0], ndata[3][0], xfrac);
	float x2 = cubicPolate(ndata[0][1], ndata[1][1], ndata[2][1], ndata[3][1], xfrac);
	float x3 = cubicPolate(ndata[0][2], ndata[1][2], ndata[2][2], ndata[3][2], xfrac);
	float x4 = cubicPolate(ndata[0][3], ndata[1][3], ndata[2][3], ndata[3][3], xfrac);

	float yval = cubicPolate(x1, x2, x3, x4, zfrac);

	return yval;
}

/* void generateNoise()
	fills the randomNoise array with random double values from 0 to 1
*/
void Terrain::generateNoise(int s1, int s2) {
#ifdef SEED_DEBUG
	srand(SEED_DEBUG);
#endif
	for(int x = 0; x <= mapwidth/delta; x += frequency) {
		for(int z = 0; z <= mapheight/delta; z += frequency) {
			randomNoise[x][z] = ((rand() % s1) / (float)s2);
		}
	} // generates array of random heights

	for(int x = 0; x <= mapwidth/delta; x++) {
		for(int z = 0; z <= mapheight/delta; z++) {
			if(x%frequency == 0 && z%frequency == 0) continue;
			randomNoise[x][z] = interpolate(x, z);
		}
	} // fills in other heights by interpolating
}

/* float smoothNoise(double x, double y)
	returns the noise function at a point by interpolating between noise indices
*/
float Terrain::smoothNoise(double x, double z) {
	double xfrac = x-(int)x;
	double zfrac = z-(int)z;
	// fractional part of x and z

	int x1 = (int)((int)x + mapwidth/delta) % ((int)(mapwidth/delta));
	int z1 = (int)((int)z + mapheight/delta) % ((int)(mapheight/delta));
	// wrap around
	
	int x2 = (int)(x1 + mapwidth/delta - 1) % ((int)(mapwidth/delta));
	int z2 = (int)(z1 + mapheight/delta - 1) % ((int)(mapheight/delta));
	// neighbor values

	double value = 0.0;
	value += xfrac * zfrac * randomNoise[x1][z1];
	value += xfrac * (1-zfrac) * randomNoise[x1][z2];
	value += (1-xfrac) * zfrac * randomNoise[x2][z1];
	value += (1-xfrac) * (1-zfrac) * randomNoise[x2][z2];

	return value;	
}
/* void calculateNormals()
	Calculates the normal vectors of all the points.
*/
void Terrain::calculateNormals() {
	for(double z = 0.0; z <= mapheight; z += delta) {
		for(double x = 0.0; x <= mapwidth; x += delta) {
			_Point* myPoint = new _Point(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)], z-mapheight/2.0);
			_Point* adjPoints[6] = {NULL};
			if(x > 0.0 && z > 0.0) // up and left
				adjPoints[0] = new _Point(x-delta-mapwidth/2.0, heightmap[(int)(x/delta)-1][(int)(z/delta)-1], z-delta-mapheight/2.0);
			if(z > 0.0) // up
				adjPoints[1] = new _Point(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)-1], z-delta-mapheight/2.0);
			if(x < mapwidth-delta) // right
				adjPoints[2] = new _Point(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)], z-mapheight/2.0);
			if(x < mapwidth-delta && z < mapheight-delta) // down right
				adjPoints[3] = new _Point(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)+1], z+delta-mapheight/2.0);
			if(z < mapheight-delta) // down
				adjPoints[4] = new _Point(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)+1], z+delta-mapheight/2.0);
			if(x > 0.0) // left
				adjPoints[5] = new _Point(x-delta-mapwidth/2.0, heightmap[(int)(x/delta)-1][(int)(z/delta)], z-mapheight/2.0);
			// all adjacent points

			normals[(int)(x/delta)][(int)(z/delta)];
			if(adjPoints[0] != NULL && adjPoints[1] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[1])-*myPoint, *(adjPoints[0])-*myPoint);
			if(adjPoints[1] != NULL && adjPoints[2] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[2])-*myPoint, *(adjPoints[1])-*myPoint);
			if(adjPoints[2] != NULL && adjPoints[3] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[3])-*myPoint, *(adjPoints[2])-*myPoint);
			if(adjPoints[3] != NULL && adjPoints[4] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[4])-*myPoint, *(adjPoints[3])-*myPoint);
			if(adjPoints[4] != NULL && adjPoints[5] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[5])-*myPoint, *(adjPoints[4])-*myPoint);
			if(adjPoints[5] != NULL && adjPoints[0] != NULL)
				normals[(int)(x/delta)][(int)(z/delta)] += cross(*(adjPoints[0])-*myPoint, *(adjPoints[5])-*myPoint);
			normals[(int)(x/delta)][(int)(z/delta)].normalize();
		}
	} // calculates normals
}

/* Constructor: Terrain
	Creates a new terrain from the given parameters
*/
Terrain::Terrain(GLuint tex, float f, float p, float o, float a) {
	frequency = f;
	persistance = p;
	octaves = o;
	amplitude = a;
	texture = tex;

	for(int x = 0; x <= mapwidth/delta; x++) {
		for(int z = 0; z <= mapheight/delta; z++) {
			heightmap[x][z] = 0.0;
		}
	} // sets initial heights to be flat

	generateNoise(32768, 32768);
	for(int x = 0; x <= mapwidth/delta; x++) {
		for(int z = 0; z <= mapheight/delta; z++) {
			float startfreq = pow(2, octaves);
			int i = 0;
			for(float freq = startfreq; freq >= 1; freq /= 2) {
				heightmap[x][z] += smoothNoise(x/freq, z/freq) * amplitude * pow(persistance, i);
				i++;
			}
		}
	} // add different zoom layers of smoothed maps

	float max = heightmap[0][0];
	float avg = 0.0;
	for(int x = 0; x <= mapwidth/delta; x++) {
		for(int z = 0; z <= mapheight/delta; z++) {
			if(heightmap[x][z] > max) max = heightmap[x][z];
			avg += heightmap[x][z];
		}
	}
	avg /= ((mapwidth/delta)*(mapheight/delta));
	// finds max/avg height

	for(int x = 0; x <= mapwidth/delta; x++) {
		for(int z = 0; z <= mapheight/delta; z++) {
			heightmap[x][z] -= avg;
		}
	} // sets terrain somewhere visible

	calculateNormals();
}

/* void Terrain::draw()
	Renders this terrain to the screen
*/
void Terrain::draw() {
	glColor4f(1, 1, 1, 1); // white
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	float white[4] = {1.0, 1.0, 1.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	float emiss[4] = {0.0, 0.0, 0.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emiss);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
	// Plain white

	glEnable(GL_TEXTURE_2D);
	for(double z = 0.0; z < mapheight-delta; z += delta) {
		glBegin(GL_TRIANGLE_STRIP);
			for(double x = 0.0; x < mapwidth-delta; x += delta*2) {
				glNormal3f(normals[(int)(x/mapwidth)][(int)((z+delta)/mapheight)].x,
						   normals[(int)(x/mapwidth)][(int)((z+delta)/mapheight)].y,
						   normals[(int)(x/mapwidth)][(int)((z+delta)/mapheight)].z);
				glTexCoord2f(tilefactor*x/mapwidth, tilefactor*(z+delta)/mapheight);
				glVertex3f(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)+1], z+delta-mapheight/2.0);

				glNormal3f(normals[(int)(x/mapwidth)][(int)(z/mapheight)].x,
						   normals[(int)(x/mapwidth)][(int)(z/mapheight)].y,
						   normals[(int)(x/mapwidth)][(int)(z/mapheight)].z);
				glTexCoord2f(tilefactor*x/mapwidth, tilefactor*z/mapheight);
				glVertex3f(x-mapwidth/2.0, heightmap[(int)(x/delta)][(int)(z/delta)], z-mapheight/2.0);

				glNormal3f(normals[(int)((x+delta)/mapwidth)][(int)((z+delta)/mapheight)].x,
						   normals[(int)((x+delta)/mapwidth)][(int)((z+delta)/mapheight)].y,
						   normals[(int)((x+delta)/mapwidth)][(int)((z+delta)/mapheight)].z);
				glTexCoord2f(tilefactor*(x+delta)/mapwidth, tilefactor*(z+delta)/mapheight);
				glVertex3f(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)+1], z+delta-mapwidth/2.0);
				
				glNormal3f(normals[(int)((x+delta)/mapwidth)][(int)(z/mapheight)].x,
						   normals[(int)((x+delta)/mapwidth)][(int)(z/mapheight)].y,
						   normals[(int)((x+delta)/mapwidth)][(int)(z/mapheight)].z);
				glTexCoord2f(tilefactor*(x+delta)/mapwidth, tilefactor*z/mapheight);
				glVertex3f(x+delta-mapwidth/2.0, heightmap[(int)(x/delta)+1][(int)(z/delta)], z-mapheight/2.0);
			}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}

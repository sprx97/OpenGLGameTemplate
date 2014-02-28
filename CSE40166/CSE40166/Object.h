#ifndef _OPENGL_OBJECT_H_
#define _OPENGL_OBJECT_H_

#include "Face.h"
#include "Material.h"
#include "Point.h"

#include <map>
#include <string>
#include <vector>
using namespace std;

namespace CSE40166 {
	
	class Object {
	public:
		Object();
		Object( string filename );
		~Object();
		
		bool loadObjectFile( string filename );
		bool loadObjectFile( string filename, bool INFO );
		bool loadObjectFile( string filename, bool INFO, bool ERRORS );
		
		bool draw();
		
		Point* getLocation();

		vector< Face* > *getFaces();
		
	private:
		string _objFile;
		string _mtlFile;
		GLuint _objectDisplayList;
		
		bool objHasVertexTexCoords;
		bool objHasVertexNormals;
		
		void init();
		
		bool loadObject( bool INFO = false, bool ERRORS = false );
		bool loadMaterial( bool INFO = false, bool ERRORS = false );
		
		vector< GLfloat > vertices;
		vector< GLfloat > vertexNormals;
		vector< GLfloat > vertexTexCoords;
		
		map< string, Material* >* _materials;
		map< string, GLuint >* _textureHandles;

		Point* _location;
		
		vector<string> tokenizeString(string input, string delimiters);
		unsigned char* createTransparentTexture( unsigned char *imageData, unsigned char *imageMask, int texWidth, int texHeight, int texChannels, int maskChannels );
		unsigned char* loadPPM( char* filename, int &texWidth, int &texHeight, int &texChannels, bool &success );
		unsigned char* loadBMP( char* filename, int &texWidth, int &texHeight, int &texChannels, bool &success );
	};

}

#endif

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include <SOIL/soil.h>

#include "Object.h"
#include "Point.h"
#include "Vector.h"

#include <fstream>
#include <iostream>
using namespace std;

namespace CSE40166 {

	//
	//  vector<string> tokenizeString(string input, string delimiters)
	//
	//      This is a helper function to break a single string into std::vector
	//  of strings, based on a given set of delimiter characters.
	//
	vector<string> Object::tokenizeString(string input, string delimiters) {
		if(input.size() == 0)
			return vector<string>();
		
		vector<string> retVec = vector<string>();
		size_t oldR = 0, r = 0; 
		
		//strip all delimiter characters from the front and end of the input string.
		string strippedInput;
		int lowerValidIndex = 0, upperValidIndex = input.size() - 1; 
		while((unsigned int)lowerValidIndex < input.size() && delimiters.find_first_of(input.at(lowerValidIndex), 0) != string::npos)
			lowerValidIndex++;
		
		while(upperValidIndex >= 0 && delimiters.find_first_of(input.at(upperValidIndex), 0) != string::npos)
			upperValidIndex--;
		
		//if the lowest valid index is higher than the highest valid index, they're all delimiters! return nothing.
		if((unsigned int)lowerValidIndex >= input.size() || upperValidIndex < 0 || lowerValidIndex > upperValidIndex)
			return vector<string>();
		
		//remove the delimiters from the beginning and end of the string, if any.
		strippedInput = input.substr(lowerValidIndex, upperValidIndex-lowerValidIndex+1);
		
		//search for each instance of a delimiter character, and create a new token spanning
		//from the last valid character up to the delimiter character.
		while((r = strippedInput.find_first_of(delimiters, oldR)) != string::npos)
		{    
			if(oldR != r)           //but watch out for multiple consecutive delimiters!
				retVec.push_back(strippedInput.substr(oldR, r-oldR));
			oldR = r+1; 
		}    
		if(r != 0)
			retVec.push_back(strippedInput.substr(oldR, r-oldR));
		
		return retVec;
	}

	Object::Object() {
		init();
	}

	Object::Object( string filename ) : _objFile( filename ) {
		init();
		loadObject();
	}

	Object::~Object() {
		delete _materials;
		delete _textureHandles;
	}

	bool Object::loadObjectFile( string filename ) {
		return loadObjectFile( filename, true, true );
	}

	bool Object::loadObjectFile( string filename, bool INFO ) {
		return loadObjectFile( filename, INFO, true );
	}

	bool Object::loadObjectFile( string filename, bool INFO, bool ERRORS ) {
		bool result = true;
		_objFile = filename;
		
		result = loadObject( INFO, ERRORS );
		
		return result;
	}

	bool Object::draw() {
		bool result = true;
		
		glPushMatrix(); {
			glCallList( _objectDisplayList );
		}; glPopMatrix();
		
		return result;
	}

	Point* Object::getLocation() { 
		return _location;
	}

	vector< Face* >* Object::getFaces() {
		vector< Face* > *faces = new vector< Face* >();
		
		delete _materials;
		delete _textureHandles;
		
		_materials = new map< string, Material* >();
		_textureHandles = new map< string, GLuint >();
		
		Material *materialForFace = NULL;
		GLuint textureForFace = 0;
		bool smoothForFace = true;
		
		ifstream in( _objFile.c_str() );
		string line;
		while( getline( in, line ) ) {
			line.erase( line.find_last_not_of( " \n\r\t" ) + 1 );
			
			vector< string > tokens = tokenizeString( line, " " );
			if( tokens.size() < 1 ) continue;
			
			//the line should have a single character that lets us know if it's a...
			if( !tokens[0].compare( "#" ) ) {								// comment ignore
			} else if( !tokens[0].compare( "o" ) ) {						// object name ignore
			} else if( !tokens[0].compare( "g" ) ) {						// polygon group name ignore
			} else if( !tokens[0].compare( "mtllib" ) ) {					// material library
				loadMaterial();
			} else if( !tokens[0].compare( "usemtl" ) ) {					// use material library
				map< string, Material* >::iterator materialIter = _materials->find( tokens[1] );
				if( materialIter != _materials->end() ) {
					materialForFace = materialIter->second;
				} else {
					materialForFace = NULL;
				}
				
				map< string, GLuint >::iterator textureIter = _textureHandles->find( tokens[1] );
				if( textureIter != _textureHandles->end() ) {
					textureForFace = textureIter->second;
				} else {
					textureForFace = 0;
				}
			} else if( !tokens[0].compare( "s" ) ) {						// smooth shading
				if( !tokens[1].compare( "off" ) ) {
					smoothForFace = false;
				} else {
					smoothForFace = true;
				}
			} else if( !tokens[0].compare( "v" ) ) {						//vertex
			} else if( !tokens.at(0).compare( "vn" ) ) {                    //vertex normal
			} else if( !tokens.at(0).compare( "vt" ) ) {                    //vertex tex coord
			} else if( !tokens.at(0).compare( "f" ) ) {                     //face!
				
				//now, faces can be either quads or triangles (or maybe more?)
				//split the string on spaces to get the number of verts+attrs.
				vector<string> faceTokens = tokenizeString(line, " ");
				
				//some local variables to hold the vertex+attribute indices we read in.
				//we do it this way because we'll have to split quads into triangles ourselves.
				vector<int> v, vn, vt;
				
				bool faceHasVertexTexCoords = false, faceHasVertexNormals = false;
				
				for(long unsigned int i = 1; i < faceTokens.size(); i++) {
					//need to use both the tokens and number of slashes to determine what info is there.
					vector<string> groupTokens = tokenizeString(faceTokens[i], "/");
					int numSlashes = 0;
					for(long unsigned int j = 0; j < faceTokens[i].length(); j++) { if(faceTokens[i][j] == '/') numSlashes++; }
					
					int vert = atoi(groupTokens[0].c_str());
					if( vert < 0 )
						vert = (vertices.size() / 3) + vert + 1;
					
					//regardless, we always get a vertex index.
					v.push_back( vert - 1 );
					
					//based on combination of number of tokens and slashes, we can determine what we have.
					if(groupTokens.size() == 2 && numSlashes == 1) {
						int vtI = atoi(groupTokens[1].c_str());	
						if( vtI < 0 )
							vtI = (vertexTexCoords.size() / 2) + vtI + 1;
						
						vtI--;
						vt.push_back( vtI ); 
						objHasVertexTexCoords = true; 
						faceHasVertexTexCoords = true;
					} else if(groupTokens.size() == 2 && numSlashes == 2) {
						int vnI = atoi(groupTokens[1].c_str());
						if( vnI < 0 )
							vnI = vertexNormals.size() + vnI + 1;
						
						vnI--;
						vn.push_back( vnI ); 
						objHasVertexNormals = true; 
						faceHasVertexNormals = true;
					} else if(groupTokens.size() == 3) {
						int vtI = atoi(groupTokens[1].c_str());
						if( vtI < 0 )
							vtI = vertexTexCoords.size() + vtI + 1;
						
						vtI--;
						vt.push_back( vtI ); 
						objHasVertexTexCoords = true; 
						faceHasVertexTexCoords = true;
						
						int vnI = atoi(groupTokens[2].c_str());
						if( vnI < 0 )
							vnI = vertexNormals.size() + vnI + 1;
						
						vnI--;
						vn.push_back( vnI ); 
						objHasVertexNormals = true; 
						faceHasVertexNormals = true;					
					} else if(groupTokens.size() != 1) {
					}
				}    
				
				//now the local variables have been filled up; push them onto our global 
				//variables appropriately.
				
				for(long unsigned int i = 1; i < v.size()-1; i++) {
					Face *f = new Face();
					f->setMaterial( materialForFace );
					f->setTextureHandle( textureForFace );
					f->setSmooth( smoothForFace );
										
					if( faceHasVertexNormals ) {
						f->setPNormal( Vector( vertexNormals.at( vn[0]*3+0 ), vertexNormals.at( vn[0]*3+1 ), vertexNormals.at( vn[0]*3+2 ) ) ); 
					} else {
						Point v1 = Point( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
						Point v2 = Point( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
						Point v3 = Point( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );
						Vector normal = cross( v2-v1, v3-v1 );
						normal.normalize();
						
						f->setPNormal( normal );
					}
					if( faceHasVertexTexCoords ) {
						f->setPTexCoord( Point( vertexTexCoords.at( vt[0]*2+0 ), vertexTexCoords.at( vt[0]*2+1 ), 0.0f ) );
					}
					f->setP( Point( vertices.at( v[0]*3+0 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) ) );
					
					if( faceHasVertexNormals ) {
						f->setQNormal( Vector( vertexNormals.at( vn[i]*3+0 ), vertexNormals.at( vn[i]*3+1 ), vertexNormals.at( vn[i]*3+2 ) ) );
					} else {
						Point v1 = Point( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
						Point v2 = Point( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
						Point v3 = Point( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );
						Vector normal = cross( v3-v2, v1-v2 );
						normal.normalize();
						
						f->setQNormal( normal );
					}
					if( faceHasVertexTexCoords ) {
						f->setQTexCoord( Point( vertexTexCoords.at( vt[i]*2+0 ), vertexTexCoords.at( vt[i]*2+1 ), 0.0f ) );
					}
					f->setQ( Point( vertices.at( v[i]*3+0 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) ) );
					
					if( faceHasVertexNormals ) {
						f->setRNormal( Vector( vertexNormals.at( vn[i+1]*3+0 ), vertexNormals.at( vn[i+1]*3+1 ), vertexNormals.at( vn[i+1]*3+2 ) ) );
					} else {
						Point v1 = Point( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
						Point v2 = Point( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
						Point v3 = Point( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );
						Vector normal = cross( v1-v3, v2-v3 );
						normal.normalize();

						f->setRNormal( normal );
					}
					if( faceHasVertexTexCoords ) {
						f->setRTexCoord( Point( vertexTexCoords.at( vt[i+1]*2+0 ), vertexTexCoords.at( vt[i+1]*2+1 ), 0.0f ) );
					}
					f->setR( Point( vertices.at( v[i+1]*3+0 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) ) );	
					
					faces->push_back( f );
				} 
			}
		}
		in.close();
		
		return faces;
	}

	void Object::init() {
		objHasVertexTexCoords = false;
		objHasVertexNormals = false;
		
		_materials = new map< string, Material* >();
		_textureHandles = new map< string, GLuint >();
		
		_location = new Point(0, 0, 0);
	}

	bool Object::loadObject( bool INFO, bool ERRORS ) {
		bool result = true;
		
		int numFaces = 0;
		float minX = 999999, maxX = -999999, minY = 999999, maxY = -999999, minZ = 999999, maxZ = -999999;
		string line;
		
		Material *solidWhiteMaterial = new Material( CSE40166_MATERIAL_WHITE );
		
		_objectDisplayList = glGenLists(1);
		glNewList(_objectDisplayList, GL_COMPILE); {
			
			if (INFO ) cout << "[.obj]: about to read " << _objFile << endl;
			
			ifstream in( _objFile.c_str() );
			while( getline( in, line ) ) {
				line.erase( line.find_last_not_of( " \n\r\t" ) + 1 );
				
				vector< string > tokens = tokenizeString( line, " " );
				if( tokens.size() < 1 ) continue;
				
				//the line should have a single character that lets us know if it's a...
				if( !tokens[0].compare( "#" ) ) {								// comment ignore
				} else if( !tokens[0].compare( "o" ) ) {						// object name ignore
				} else if( !tokens[0].compare( "g" ) ) {						// polygon group name ignore
				} else if( !tokens[0].compare( "mtllib" ) ) {					// material library
					_mtlFile = tokens[1];
					if (INFO) cout << "[.obj]: about to read " << _mtlFile << endl;
					if( loadMaterial( INFO, ERRORS ) )
						if (INFO) cout << "[.obj]: finished reading " << _mtlFile << endl;
				} else if( !tokens[0].compare( "usemtl" ) ) {					// use material library
					map< string, Material* >::iterator materialIter = _materials->find( tokens[1] );
					if( materialIter != _materials->end() ) {
						setCurrentMaterial( materialIter->second );
					} else {

					}
					
					map< string, GLuint >::iterator textureIter = _textureHandles->find( tokens[1] );
					if( textureIter != _textureHandles->end() ) {
						glEnable( GL_TEXTURE_2D );
						glBindTexture( GL_TEXTURE_2D, textureIter->second );
					} else {
						glDisable( GL_TEXTURE_2D );
					}
				} else if( !tokens[0].compare( "s" ) ) {						// smooth shading
					if( !tokens[1].compare( "off" ) ) {
						glShadeModel( GL_FLAT );
					} else {
						glShadeModel( GL_SMOOTH );
					}
				} else if( !tokens[0].compare( "v" ) ) {						//vertex
					float x = atof( tokens[1].c_str() ),
					y = atof( tokens[2].c_str() ),
					z = atof( tokens[3].c_str() );
					
					if( x < minX ) minX = x;
					if( x > maxX ) maxX = x;
					if( y < minY ) minY = y;
					if( y > maxY ) maxY = y;
					if( z < minZ ) minZ = z;
					if( z > maxZ ) maxZ = z;
					
					vertices.push_back( x );
					vertices.push_back( y );
					vertices.push_back( z );
				} else if( !tokens.at(0).compare( "vn" ) ) {                    //vertex normal
					vertexNormals.push_back( atof( tokens[1].c_str() ) );
					vertexNormals.push_back( atof( tokens[2].c_str() ) );
					vertexNormals.push_back( atof( tokens[3].c_str() ) );
				} else if( !tokens.at(0).compare( "vt" ) ) {                    //vertex tex coord
					vertexTexCoords.push_back(atof(tokens[1].c_str()));
					vertexTexCoords.push_back(atof(tokens[2].c_str()));
				} else if( !tokens.at(0).compare( "f" ) ) {                     //face!
					
					//now, faces can be either quads or triangles (or maybe more?)
					//split the string on spaces to get the number of verts+attrs.
					vector<string> faceTokens = tokenizeString(line, " ");
					
					//some local variables to hold the vertex+attribute indices we read in.
					//we do it this way because we'll have to split quads into triangles ourselves.
					vector<int> v, vn, vt;
					
					bool faceHasVertexTexCoords = false, faceHasVertexNormals = false;
					
					for(long unsigned int i = 1; i < faceTokens.size(); i++) {
						//need to use both the tokens and number of slashes to determine what info is there.
						vector<string> groupTokens = tokenizeString(faceTokens[i], "/");
						int numSlashes = 0;
						for(long unsigned int j = 0; j < faceTokens[i].length(); j++) { if(faceTokens[i][j] == '/') numSlashes++; }
						
						int vert = atoi(groupTokens[0].c_str());
						if( vert < 0 )
							vert = (vertices.size() / 3) + vert + 1;
						
						//regardless, we always get a vertex index.
						v.push_back( vert - 1 );
						
						//based on combination of number of tokens and slashes, we can determine what we have.
						if(groupTokens.size() == 2 && numSlashes == 1) {
							int vtI = atoi(groupTokens[1].c_str());	
							if( vtI < 0 )
								vtI = (vertexTexCoords.size() / 2) + vtI + 1;
							
							vtI--;
							vt.push_back( vtI ); 
							objHasVertexTexCoords = true; 
							faceHasVertexTexCoords = true;
						} else if(groupTokens.size() == 2 && numSlashes == 2) {
							int vnI = atoi(groupTokens[1].c_str());
							if( vnI < 0 )
								vnI = vertexNormals.size() + vnI + 1;
							
							vnI--;
							vn.push_back( vnI ); 
							objHasVertexNormals = true; 
							faceHasVertexNormals = true;
						} else if(groupTokens.size() == 3) {
							int vtI = atoi(groupTokens[1].c_str());
							if( vtI < 0 )
								vtI = vertexTexCoords.size() + vtI + 1;
							
							vtI--;
							vt.push_back( vtI ); 
							objHasVertexTexCoords = true; 
							faceHasVertexTexCoords = true;
							
							int vnI = atoi(groupTokens[2].c_str());
							if( vnI < 0 )
								vnI = vertexNormals.size() + vnI + 1;

							vnI--;
							vn.push_back( vnI ); 
							objHasVertexNormals = true; 
							faceHasVertexNormals = true;					
						} else if(groupTokens.size() != 1) {
							if (ERRORS) fprintf(stderr, "[.obj]: [ERROR] Malformed OBJ file, %s.\n", _objFile.c_str());
							return false;
						}
					}    
					
					//now the local variables have been filled up; push them onto our global 
					//variables appropriately.
					
					glBegin(GL_TRIANGLES); {
						for(long unsigned int i = 1; i < v.size()-1; i++) {
							
							if( faceHasVertexNormals ) {
								glNormal3f( vertexNormals.at( vn[0]*3 ), vertexNormals.at( vn[0]*3+1 ), vertexNormals.at( vn[0]*3+2) );
							} else {
								Point v1 = Point( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
								Point v2 = Point( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
								Point v3 = Point( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );
								Vector normal = cross( v2-v1, v3-v1 );
								normal.normalize();
								glNormal3f( normal.getX(), normal.getY(), normal.getZ() );
							}
							if( faceHasVertexTexCoords )
								glTexCoord2f( vertexTexCoords.at( vt[0]*2 ), vertexTexCoords.at( vt[0]*2+1 ) );
							
							glVertex3f( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
							
							if( faceHasVertexNormals ) {
								glNormal3f( vertexNormals.at( vn[i]*3 ), vertexNormals.at( vn[i]*3+1 ), vertexNormals.at( vn[i]*3+2 ) );
							} else {
								Point v1 = Point( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
								Point v2 = Point( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
								Point v3 = Point( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );
								Vector normal = cross( v3-v2, v1-v2 );
								normal.normalize();
								glNormal3f( normal.getX(), normal.getY(), normal.getZ() );
							}
							if( faceHasVertexTexCoords )
								glTexCoord2f( vertexTexCoords.at( vt[i]*2 ), vertexTexCoords.at( vt[i]*2+1 ) );
							glVertex3f( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
							
							if( faceHasVertexNormals ) {
								glNormal3f( vertexNormals.at( vn[i+1]*3 ), vertexNormals.at( vn[i+1]*3+1 ), vertexNormals.at( vn[i+1]*3+2) );
							} else {
								Point v1 = Point( vertices.at( v[0]*3 ), vertices.at( v[0]*3+1 ), vertices.at( v[0]*3+2 ) );
								Point v2 = Point( vertices.at( v[i]*3 ), vertices.at( v[i]*3+1 ), vertices.at( v[i]*3+2 ) );
								Point v3 = Point( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );
								Vector normal = cross( v1-v3, v2-v3 );
								normal.normalize();
								glNormal3f( normal.getX(), normal.getY(), normal.getZ() );
							}
							if( faceHasVertexTexCoords )
								glTexCoord2f( vertexTexCoords.at( vt[i+1]*2 ), vertexTexCoords.at( vt[i+1]*2+1 ) );
							glVertex3f( vertices.at( v[i+1]*3 ), vertices.at( v[i+1]*3+1 ), vertices.at( v[i+1]*3+2 ) );				
						} 
					}; glEnd();
					
					numFaces++;
				} else {
					if (INFO) cout << "[.obj]: ignoring line: " << line << endl;
				}
			}
			in.close();
			setCurrentMaterial( solidWhiteMaterial );
			glDisable( GL_TEXTURE_2D );
		}; glEndList();
		
		if (INFO) {
			cout << "[.obj]: finished reading " << _objFile << endl;
			cout << "[.obj]: read " << vertices.size()/3 << " vertices, " << vertexNormals.size()/3 << " vertex normals, " << vertexTexCoords.size()/2 << " vertex tex coords, and " << numFaces << " polygons." << endl;
			cout << "[.obj]: " << (maxX - minX) << " units across in X, " << (maxY - minY) << " units across in Y, " << (maxZ - minZ) << " units across in Z." << endl;
		}
		
		return result;
	}

	bool Object::loadMaterial( bool INFO, bool ERRORS ) {
		bool result = true;
		
		string line;
		
		ifstream in( _mtlFile.c_str() );
		if( !in.is_open() ) {
			cerr << "[.mtl]: [ERROR] could not open material file: " << _mtlFile << endl;
			return false;
		}
		
		Material *currMaterial;
		string materialName;
		
		unsigned char *textureData = NULL;
		unsigned char *maskData = NULL;
		unsigned char *fullData;
		int texWidth, texHeight, textureChannels = 1, maskChannels = 1;
		GLuint textureHandle = 0;
		
		map< string, GLuint > imageHandles;
		
		while( getline( in, line ) ) {
			line.erase( line.find_last_not_of( " \n\r\t" ) + 1 );
			
			vector< string > tokens = tokenizeString( line, " /" );
			if( tokens.size() < 1 ) continue;
			
			//the line should have a single character that lets us know if it's a...
			if( !tokens[0].compare( "#" ) ) {							// comment
			} else if( !tokens[0].compare( "newmtl" ) ) {				//new material
				currMaterial = new Material();
				materialName = tokens[1];
				_materials->insert( pair<string, Material*>( materialName, currMaterial ) );

				textureHandle = 0;
				textureData = NULL;
				maskData = NULL;
				textureChannels = 1;
				maskChannels = 1;
			} else if( !tokens[0].compare( "Ka" ) ) {					// ambient component
				GLfloat ambient[4];
				ambient[0] = atof( tokens[1].c_str() );
				ambient[1] = atof( tokens[2].c_str() );
				ambient[2] = atof( tokens[3].c_str() );
				ambient[3] = 1;
				currMaterial->setAmbient( ambient );
			} else if( !tokens[0].compare( "Kd" ) ) {					// diffuse component
				GLfloat diffuse[4];
				diffuse[0] = atof( tokens[1].c_str() );
				diffuse[1] = atof( tokens[2].c_str() );
				diffuse[2] = atof( tokens[3].c_str() );
				diffuse[3] = 1;			
				currMaterial->setDiffuse( diffuse );
			} else if( !tokens[0].compare( "Ks" ) ) {					// specular component
				GLfloat specular[4];
				specular[0] = atof( tokens[1].c_str() );
				specular[1] = atof( tokens[2].c_str() );
				specular[2] = atof( tokens[3].c_str() );
				specular[3] = 1;						
				currMaterial->setSpecular( specular );
			} else if( !tokens[0].compare( "Ke" ) ) {					// emissive component
				GLfloat emissive[4];
				emissive[0] = atof( tokens[1].c_str() );
				emissive[1] = atof( tokens[2].c_str() );
				emissive[2] = atof( tokens[3].c_str() );
				currMaterial->setEmissive( emissive );
			} else if( !tokens[0].compare( "Ns" ) ) {					// shininess component
				currMaterial->setShininess( atof( tokens[1].c_str() ) );
			} else if( !tokens[0].compare( "Tr" ) 
					  || !tokens[0].compare( "d" ) ) {					// transparency component - Tr or d can be used depending on the format
				currMaterial->getAmbient()[3] = atof( tokens[1].c_str() );
				currMaterial->getDiffuse()[3] = atof( tokens[1].c_str() );
				currMaterial->getSpecular()[3] = atof( tokens[1].c_str() );
			} else if( !tokens[0].compare( "illum" ) ) {				// illumination type component
				currMaterial->setIllumination( atoi( tokens[1].c_str() ) );
			} else if( !tokens[0].compare( "map_Kd" ) ) {				// diffuse texture map
				if( imageHandles.find( tokens[1] ) != imageHandles.end() ) {
					_textureHandles->insert( pair< string, GLuint >( materialName, imageHandles.find( tokens[1] )->second ) );
				} else {
					if( tokens[1].find( ".bmp" ) != string::npos || tokens[1].find( ".BMP" ) != string::npos ) {
						bool success = false;
						textureData = loadBMP( (char*)tokens[1].c_str(), texWidth, texHeight, textureChannels, success );
						if( !success ) {
							textureData = SOIL_load_image( tokens[1].c_str(), &texWidth, &texHeight, &textureChannels, SOIL_LOAD_AUTO );
						} else {
							
						}
					} else if( tokens[1].find( ".ppm" ) != string::npos || tokens[1].find( ".PPM" ) != string::npos ) {
						bool success = false;
						textureData = loadPPM( (char*)tokens[1].c_str(), texWidth, texHeight, textureChannels, success );
						if( !success ) {
							textureData = SOIL_load_image( tokens[1].c_str(), &texWidth, &texHeight, &textureChannels, SOIL_LOAD_AUTO );
						} else {
							
						}
					} else {
						textureData = SOIL_load_image( tokens[1].c_str(), &texWidth, &texHeight, &textureChannels, SOIL_LOAD_AUTO );
					}

					if( !textureData ) {
						cout << "[.mtl]: [ERROR] file not found " << tokens[1] << endl;
					} else {
						if (INFO) cout << "[.mtl]: " << tokens[1] << " texture map of size " << texWidth << "x" 
										<< texHeight << " with " << textureChannels << " colors read in" << endl;
						
						if( maskData == NULL ) {
							if( textureHandle == 0 )
								glGenTextures( 1, &textureHandle );
							
							glBindTexture( GL_TEXTURE_2D, textureHandle );
							
							glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
							
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
							
							GLenum colorSpace = GL_RGB;
							if( textureChannels == 4 )
								colorSpace = GL_RGBA;
							gluBuild2DMipmaps( GL_TEXTURE_2D, colorSpace, texWidth, texHeight, colorSpace, GL_UNSIGNED_BYTE, textureData );		
				
							_textureHandles->insert( pair<string, GLuint>( materialName, textureHandle ) );
							imageHandles.insert( pair<string, GLuint>( tokens[1], textureHandle ) );
						} else {					
							fullData = createTransparentTexture( textureData, maskData, texWidth, texHeight, textureChannels, maskChannels );
							
							if( textureHandle == 0 )
								glGenTextures( 1, &textureHandle );
							
							glBindTexture( GL_TEXTURE_2D, textureHandle );
							
							glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
							
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
							
							gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, fullData );

							delete fullData;
				
							_textureHandles->insert( pair<string, GLuint>( materialName, textureHandle ) );
						}
					}
				}
			} else if( !tokens[0].compare( "map_d" ) ) {				// alpha texture map
				if( imageHandles.find( tokens[1] ) != imageHandles.end() ) {
					_textureHandles->insert( pair< string, GLuint >( materialName, imageHandles.find( tokens[1] )->second ) );
				} else {
					if( tokens[1].find( ".bmp" ) != string::npos || tokens[1].find( ".BMP" ) != string::npos ) {
						bool success = false;
						maskData = loadBMP( (char*)tokens[1].c_str(), texWidth, texHeight, maskChannels, success );
						if( !success ) {
							maskData = SOIL_load_image( tokens[1].c_str(), &texWidth, &texHeight, &maskChannels, SOIL_LOAD_AUTO );
						} else {
							
						}
					} else if( tokens[1].find( ".ppm" ) != string::npos || tokens[1].find( ".PPM" ) != string::npos ) {
						bool success = false;
						maskData = loadPPM( (char*)tokens[1].c_str(), texWidth, texHeight, maskChannels, success );
						if( !success ) {
							maskData = SOIL_load_image( tokens[1].c_str(), &texWidth, &texHeight, &maskChannels, SOIL_LOAD_AUTO );
						} else {
							
						}
					} else {
						maskData = SOIL_load_image( tokens[1].c_str(), &texWidth, &texHeight, &maskChannels, SOIL_LOAD_AUTO );
					}
					
					if( textureData != NULL ) {
						fullData = createTransparentTexture( textureData, maskData, texWidth, texHeight, textureChannels, maskChannels );
						
						if (INFO) cout << "[.mtl]: " << tokens[1] << " alpha texture map of size " << texWidth << "x" 
										<< texHeight << " with " << textureChannels << " colors read in" << endl;
						
						if( textureHandle == 0 )
							glGenTextures( 1, &textureHandle );
						
						glBindTexture( GL_TEXTURE_2D, textureHandle );
						
						glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
						
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						
						gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, fullData );
						
						delete fullData;
					}
				}
			} else if( !tokens[0].compare( "Ni" ) ) {					// optical density / index of refraction
				
			} else if( !tokens[0].compare( "Tf" ) ) {					// transmission filter
				
			} else {
				if (INFO) cout << "[.mtl]: ignoring line: " << line << endl;
			}
		}
		
		in.close();
		
		return result;
	}

	unsigned char* Object::createTransparentTexture( unsigned char *imageData, unsigned char *imageMask, int texWidth, int texHeight, int texChannels, int maskChannels ) {
		//combine the 'mask' array with the image data array into an RGBA array.
		unsigned char *fullData = new unsigned char[texWidth*texHeight*4];
		
		for(int j = 0; j < texHeight; j++) {
			for(int i = 0; i < texWidth; i++) {
				if( imageData ) {
					fullData[(j*texWidth+i)*4+0] = imageData[(j*texWidth+i)*3+0];	// R
					fullData[(j*texWidth+i)*4+1] = imageData[(j*texWidth+i)*3+1];	// G
					fullData[(j*texWidth+i)*4+2] = imageData[(j*texWidth+i)*3+2];	// B
				} else {
					fullData[(j*texWidth+i)*4+0] = 1;	// R
					fullData[(j*texWidth+i)*4+1] = 1;	// G
					fullData[(j*texWidth+i)*4+2] = 1;	// B
				}
				
				if( imageMask ) {
					fullData[(j*texWidth+i)*4+3] = imageMask[(j*texWidth+i)*maskChannels+0];	// A
				} else {
					fullData[(j*texWidth+i)*4+3] = 1;	// A
				}
			}
		}
		return fullData;
	}

	unsigned char* Object::loadPPM( char* filename, int &texWidth, int &texHeight, int &texChannels, bool &success ) {
		FILE *fp;
		
		// make sure the file is there.
		if ((fp = fopen(filename, "rb"))==NULL) {
			printf("File Not Found : %s\n",filename);
			return 0;
		}
		
		unsigned char *imageData;
		
		int temp, maxValue;
		fscanf(fp, "P%d", &temp);
		if(temp != 3) {
			fprintf(stderr, "Error: PPM file is not of correct format! (Must be P3, is P%d.)\n", temp);
			fclose(fp);
			return 0;
		}
		
		//got the file header right...
		fscanf(fp, "%d", &texWidth);
		fscanf(fp, "%d", &texHeight);
		fscanf(fp, "%d", &maxValue);
		
		//now that we know how big it is, allocate the buffer...
		imageData = new unsigned char[texWidth*texHeight*3];
		if(!imageData) {
			fprintf(stderr, "Error: couldn't allocate image memory. Dimensions: %d x %d.\n", texWidth, texHeight);
			fclose(fp);
			return 0;
		}
		
		//and read the data in.
		for(int j = 0; j < texHeight; j++) {
			for(int i = 0; i < texWidth; i++) {
				//read the data into integers (4 bytes) before casting them to unsigned characters
				//and storing them in the unsigned char array.
				int r, g, b;
				fscanf(fp, "%d", &r);
				fscanf(fp, "%d", &g);
				fscanf(fp, "%d", &b);
				
				imageData[(j*texWidth+i)*3+0] = (unsigned char)r;
				imageData[(j*texWidth+i)*3+1] = (unsigned char)g;
				imageData[(j*texWidth+i)*3+2] = (unsigned char)b;
			}
		}
		
		fclose(fp);
		
		texChannels = 3;
		success = true;
		return imageData;
	}

	unsigned char* Object::loadBMP( char* filename, int &texWidth, int &texHeight, int &texChannels, bool &success ) {
		FILE *file;
		unsigned long size;                 // size of the image in bytes.
		size_t i;							// standard counter.
		unsigned short int planes;          // number of planes in image (must be 1) 
		unsigned short int bpp;             // number of bits per pixel (must be 24)
		char temp;                          // used to convert bgr to rgb color.
		unsigned char *data;
		
		// make sure the file is there.
		if ((file = fopen(filename, "rb"))==NULL) {
			printf("File Not Found : %s\n",filename);
			return 0;
		}
		
		// seek through the bmp header, up to the width/height:
		fseek(file, 18, SEEK_CUR);
		
		// read the width
		if ((i = fread(&texWidth, 4, 1, file)) != 1) {
			printf("Error reading width from %s.\n", filename);
			return 0;
		}
		//printf("Width of %s: %lu\n", filename, image->sizeX);
		
		// read the height 
		if ((i = fread(&texHeight, 4, 1, file)) != 1) {
			printf("Error reading height from %s.\n", filename);
			return 0;
		}
		//printf("Height of %s: %lu\n", filename, image->sizeY);
		
		// calculate the size (assuming 24 bits or 3 bytes per pixel).
		size = texWidth * texHeight * 3;
		
		// read the planes
		if ((fread(&planes, 2, 1, file)) != 1) {
			printf("Error reading planes from %s.\n", filename);
			return 0;
		}
		if (planes != 1) {
			printf("Planes from %s is not 1: %u\n", filename, planes);
			return 0;
		}
		
		// read the bpp
		if ((i = fread(&bpp, 2, 1, file)) != 1) {
			printf("Error reading bpp from %s.\n", filename);
			return 0;
		}
		if (bpp != 24) {
			printf("Bpp from %s is not 24: %u\n", filename, bpp);
			return 0;
		}
		
		// seek past the rest of the bitmap header.
		fseek(file, 24, SEEK_CUR);
		
		// read the data. 
		data = (unsigned char *) malloc(size);
		if (data == NULL) {
			printf("Error allocating memory for color-corrected image data");
			return 0;	
		}
		
		if ((i = fread(data, size, 1, file)) != 1) {
			printf("Error reading image data from %s.\n", filename);
			return 0;
		}
		
		for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
			temp = data[i];
			data[i] = data[i+2];
			data[i+2] = temp;
		}

		texChannels = 3;
		success = true;
		return data;
	}

}
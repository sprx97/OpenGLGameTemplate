#ifndef _OPENGL_MATERIAL_H_
#define _OPENGL_MATERIAL_H_ 1

#ifdef __APPLE__
        #include <OpenGL/glu.h>
#else
        #include <GL/glu.h>
#endif

namespace CSE40166 {
	
	/* predefined materials */
	enum CSE40166_MaterialColor {
		CSE40166_MATERIAL_WHITE = 1,
		CSE40166_MATERIAL_BLACK = 2,
		CSE40166_MATERIAL_BRASS = 3,
		CSE40166_MATERIAL_REDPLASTIC = 4,
		CSE40166_MATERIAL_GREENPLASTIC = 5,
		CSE40166_MATERIAL_CYANRUBBER = 6
	};

	class Material {
	public:
		/* constructor */
		Material();
		/* use a predefined material color set */
		Material( CSE40166_MaterialColor preDefinedColor );
		
		/* get and set ambient component */
		GLfloat* getAmbient();
		void setAmbient( GLfloat amb[4] );
		
		/* get and set diffuse component */
		GLfloat* getDiffuse();
		void setDiffuse( GLfloat diff[4] );
		
		/* get and set specular component */
		GLfloat* getSpecular();
		void setSpecular( GLfloat spec[4] );
		
		/* get and set emissive componenet */
		GLfloat* getEmissive();
		void setEmissive( GLfloat emis[4] );
		
		/* get and set shininess value */
		GLfloat getShininess();
		void setShininess( GLfloat shin );
		
		/* get and set illumination type */
		/* used for compatibility with *.mtl files */
		GLint getIllumination();
		void setIllumination( GLint illum );
		
	private:
		GLfloat _ambient[4];
		GLfloat _diffuse[4];
		GLfloat _specular[4];
		GLfloat _emissive[4];
		GLfloat _shininess;
		GLint _illumination;
		
		void init( CSE40166_MaterialColor preDefinedColor );
	};

	/* set material to be the active material */
	void setCurrentMaterial( Material *material );

}

#endif

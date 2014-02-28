#ifndef _LIGHT_H_
#define _LIGHT_H_ 1

#ifdef __APPLE__
        #include <OpenGL/glu.h>
#else
        #include <GL/glu.h>
#endif

namespace CSE40166 {
	
	/* Light Types */
	enum LightType {
		POINT,
		DIRECTIONAL,
		SPOT
	};

	/* abstract light definition */
	/* cannot instantiate, must use PointLight DirectionalLight or SpotLight */
	class Light {
	public:
		/* virtual light contstructor */
		Light();
		
		/* get and set ambient component */
		GLfloat* getAmbient();
		void setAmbient( GLfloat amb[4] );
		
		/* get and set diffuse component */
		GLfloat* getDiffuse();
		void setDiffuse( GLfloat diff[4] );
		
		/* get and set specular component */
		GLfloat* getSpecular();
		void setSpecular( GLfloat spec[4] );
		
		/* get Light Type */
		LightType getLightType();
		
		/* get and set light number */
		/* must be between 0 and 8 */
		GLint getLightNumber();
		void setLightNumber( GLint lNum );
		
		/* get and set constant light attenuation factor */
		GLfloat getConstantAttenuation();
		void setConstantAttenuation( GLfloat cA );
		
		/* get and set linear light attenuation factor */
		GLfloat getLinearAttenuation();
		void setLinearAttenuation( GLfloat lA );
		
		/* get and set quadratic light attenuation factor */
		GLfloat getQuadraticAttenuation();
		void setQuadraticAttenuation( GLfloat qA );
		
		/* virtaul method to set all glLightf() parameters */
		virtual void shine() =0;
		
		/* turn light off or on */
		/* calls glDisable( GL_LIGHT# ) or glEnable( GL_LIGHT# ) */
		void turnLightOff();
		void turnLightOn();
		
		/* is this light currently on? */
		bool isLightOn();
		
	protected:
		GLfloat _ambient[4];
		GLfloat _diffuse[4];
		GLfloat _specular[4];

		GLint _lightNumber;
		
		GLfloat _cAtten;
		GLfloat _lAtten;
		GLfloat _qAtten;
		
		LightType _type;
		
		bool _on;
	};

}

#endif

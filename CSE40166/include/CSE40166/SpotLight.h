#ifndef _SPOTLIGHT_H_
#define _SPOTLIGHT_H_ 1

#include "Light.h"
#include "Point.h"
#include "Vector.h"

namespace CSE40166 {
	
	class SpotLight : public Light {
	public:
		/* constructor requires light number between 0 and 8 */
		SpotLight( GLint lightNum );
		
		/* get and set light position */
		Point* getPosition();
		void setPosition( Point pos );
		void setPosition( Point* pos );
		
		/* get and set light direction */
		Vector* getDirection();
		void setDirection( Vector dir );
		void setDirection( Vector* dir );
		
		/* get and set light cone cutoff */
		/* acceptable values [0,90] or 180 */
		GLfloat getCutoff();
		void setCutoff( GLfloat co );
		
		/* get and set light exponent */
		GLfloat getExponent();
		void setExponent( GLfloat exp );
		
		/* call glLightf() parameters */
		virtual void shine();
		
	private:
		Point* _position;
		Vector* _direction;
		
		GLfloat _cutoff;
		GLfloat _exponent;
	};

}

#endif

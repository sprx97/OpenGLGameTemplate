#ifndef _POINTLIGHT_H_
#define _POINTLIGHT_H_ 1

#include "Light.h"
#include "Point.h"

namespace CSE40166 {
	
	class PointLight : public Light {
	public:
		/* constructor requires light number between 0 and 8 */
		PointLight( GLint lightNum );

		/* get and set position */
		Point* getPosition();
		void setPosition( Point pos );
		void setPosition( Point* pos );
		
		/* call appropriate glLightf() parameters */
		virtual void shine();
		
	private:
		Point* _position;

	};

}

#endif

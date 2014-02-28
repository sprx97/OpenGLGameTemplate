#ifndef _DIRECTIONALLIGHT_H_
#define _DIRECTIONALLIGHT_H_ 1

#include "Light.h"
#include "Vector.h"

namespace CSE40166 {
	
	class DirectionalLight : public Light {
	public:
		/* constructor requires a light number between 0 and 8 */
		DirectionalLight( GLint lightNum );
		
		/* get and set the direction of the light */
		Vector* getDirection();
		void setDirection( Vector dir );
		void setDirection( Vector* dir );	// a pointer to the vector
		
		/* calls glLightfv() with appropriate values */
		virtual void shine();
		
	private:
		Vector* _direction;
	};

}

#endif

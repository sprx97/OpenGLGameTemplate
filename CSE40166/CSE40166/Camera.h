#ifndef _CAMERA_H_
#define _CAMERA_H_ 1

#include "Object.h"
#include "Point.h"
#include "Vector.h"

namespace CSE40166 {

	/* predefined camera types */
	enum CameraType {
		ARCBALLCAM,
		FREECAM,
		OTHER
	};

	class Camera {
	public:
		/* constructors and destructors */
		Camera();
		Camera( CameraType cType );
		~Camera();

		/* compute the arcball cam's position based on radius, theta, phi */
		void computeArcballPosition();
		/* compute the free cam's orientation based on theta, phi */
		void computeFreecamOrientation();

		/* call gluLookAt() with set parameters */
		void look();
		
		/* set the lookAt point to always be a given object */
		/* used for arcball cam */
		void followObject( Object* _obj );
		
		/* move free cam forward */
		void moveForward();
		/* move free cam backward */
		void moveBackward();

		/* get and set the eye point */
		Point* getEye();
		void setEye( Point* eye );	// a pointer to the eye point
		void setEye( Point eye );
		
		/* get and set the lookAt point */
		Point* getLookAt();
		void setLookAt( Point* lookAt );	// a pointer to the lookAt point
		void setLookAt( Point lookAt );
		
		/* get and set the up vector */
		Vector* getUp();
		void setUp( Vector* up );	// a pointer to the up vector
		void setUp( Vector up );

		/* get a pointer to the view vector */
		Vector* getView();
		
		/* get and set theta */
		float getTheta();
		void setTheta( float t );

		/* get and set phi */
		float getPhi();
		void setPhi( float p );
		
		/* get and set radius */
		float getRadius();
		void setRadius( float r );

	private:
		Point* _eye;
		Point* _lookAt;
		Vector* _up;
		Vector* _dir;
		
		float _theta, _phi, _radius;
		
		CameraType _type;
		
		Object* _objectToFollow;
	};
	
}

#endif

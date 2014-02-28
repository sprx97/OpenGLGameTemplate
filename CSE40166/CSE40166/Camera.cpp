#include "Camera.h"

#include <math.h>

#ifdef __APPLE__
	#include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif

namespace CSE40166 {

	Camera::Camera() {
		Camera( OTHER );
	}

	Camera::Camera( CameraType cType ) {
		_eye = new Point();
		_lookAt = new Point();
		_up = new Vector( 0, 1, 0 );
		_dir = new Vector();
		
		_theta = _phi = _radius = 0;
		
		_type = cType;
	}

	Camera::~Camera() {}

	void Camera::computeArcballPosition() {
		_eye->setX( _radius * sinf( _theta ) * sinf( _phi ) );
		_eye->setY( _radius * -cosf( _phi ) );
		_eye->setZ( _radius * -cosf( _theta ) * sinf( _phi ) );	
	}

	void Camera::computeFreecamOrientation() {
		_dir->setX(  sinf( _theta ) * sinf( _phi ) );
		_dir->setY( -cosf( _phi ) );
		_dir->setZ( -cosf( _theta ) * sinf( _phi ) );
		
		_dir->normalize();
	}

	void Camera::look() {
		if( _objectToFollow && _type == ARCBALLCAM ) {
			gluLookAt( _eye->getX() + _objectToFollow->getLocation()->getX(), _eye->getY() + _objectToFollow->getLocation()->getY(), _eye->getZ() + _objectToFollow->getLocation()->getZ(),
					  _objectToFollow->getLocation()->getX(), _objectToFollow->getLocation()->getY(), _objectToFollow->getLocation()->getZ(),
					  _up->getX(), _up->getY(), _up->getZ() );
		} else if( _type == FREECAM ) {
			gluLookAt( _eye->getX(), _eye->getY(), _eye->getZ(),
					  _eye->getX() + _dir->getX(), _eye->getY() + _dir->getY(), _eye->getZ() + _dir->getZ(),
					  _up->getX(), _up->getY(), _up->getZ() );
		} else {
			gluLookAt( _eye->getX(), _eye->getY(), _eye->getZ(),
					  _lookAt->getX(), _lookAt->getY(), _lookAt->getZ(),
					  _up->getX(), _up->getY(), _up->getZ() );
		}
	}

	void Camera::followObject( Object *obj ) {
		_objectToFollow = obj;
	}

	void Camera::moveForward() {
		(*_eye) += (*_dir) * 0.3;
	}

	void Camera::moveBackward() {
		(*_eye) -= (*_dir) * 0.3;
	}

	Point* Camera::getEye() { return _eye; }

	void Camera::setEye( Point* eye ) { this->_eye = eye; }

	void Camera::setEye( Point eye ) {
		_eye->setX( eye.getX() );
		_eye->setY( eye.getY() );
		_eye->setZ( eye.getZ() );
	}

	Point* Camera::getLookAt() { return _lookAt; }

	void Camera::setLookAt( Point* lookAt ) { this->_lookAt = lookAt; }

	void Camera::setLookAt( Point lookAt ) {
		_lookAt->setX( lookAt.getX() );
		_lookAt->setY( lookAt.getY() );
		_lookAt->setZ( lookAt.getZ() );
	}

	Vector* Camera::getUp() { return _up; }

	void Camera::setUp( Vector* up ) { this->_up = up; }

	void Camera::setUp( Vector up ) { 
		_up->setX( up.getX() );
		_up->setY( up.getY() );
		_up->setZ( up.getZ() );
	}
	
	Vector* Camera::getView() { 
		Vector* view = new Vector();
		
		if( _objectToFollow && _type == ARCBALLCAM ) {
			view->setX( _objectToFollow->getLocation()->getX() - (_eye->getX() + _objectToFollow->getLocation()->getX()) );
			view->setY( _objectToFollow->getLocation()->getY() - (_eye->getY() + _objectToFollow->getLocation()->getY()) );
			view->setZ( _objectToFollow->getLocation()->getZ() - (_eye->getZ() + _objectToFollow->getLocation()->getZ()) );
		} else if( _type == FREECAM ) {
			view->setX( _dir->getX() );
			view->setY( _dir->getY() );
			view->setZ( _dir->getZ() );
		} else {
			view->setX( _lookAt->getX() - _eye->getX() );
			view->setY( _lookAt->getY() - _eye->getY() );
			view->setZ( _lookAt->getZ() - _eye->getZ() );
		}
		
		return view;
	}

	float Camera::getTheta() { return _theta; }
	void Camera::setTheta( float t ) { this->_theta = t; }

	float Camera::getPhi() { return _phi; }
	void Camera::setPhi( float p ) { this->_phi = p; }

	float Camera::getRadius() { return _radius; }
	void Camera::setRadius( float r ) { this->_radius = r; }

}
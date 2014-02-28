#include "DirectionalLight.h"

namespace CSE40166 {

	DirectionalLight::DirectionalLight( GLint lightNum ) {
		_type = DIRECTIONAL;
		_lightNumber = lightNum;

		_direction = new Vector( 1, 1, 1 );
	}

	Vector* DirectionalLight::getDirection() { return _direction; }

	void DirectionalLight::setDirection( Vector* dir ) { this->_direction = dir; }

	void DirectionalLight::setDirection( Vector dir ) {
		_direction->setX( dir.getX() );
		_direction->setY( dir.getY() );
		_direction->setZ( dir.getZ() );
	}

	void DirectionalLight::shine() {
		glEnable( GL_LIGHT0 + this->getLightNumber() );

		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_DIFFUSE, this->getDiffuse() );
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_SPECULAR, this->getSpecular() );
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_AMBIENT, this->getAmbient() );

		GLfloat dir[4];
		this->getDirection()->normalize();
		dir[0] = this->getDirection()->asVector()[0];
		dir[1] = this->getDirection()->asVector()[1];
		dir[2] = this->getDirection()->asVector()[2];
		dir[3] = this->getDirection()->asVector()[3];
		
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_POSITION, dir );
		
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_CONSTANT_ATTENUATION, this->getConstantAttenuation() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_LINEAR_ATTENUATION, this->getLinearAttenuation() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_QUADRATIC_ATTENUATION, this->getQuadraticAttenuation() );
	}

}

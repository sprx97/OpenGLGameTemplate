#include "SpotLight.h"

namespace CSE40166 {
	
	SpotLight::SpotLight( GLint lightNum ) {
		_type = SPOT;
		_lightNumber = lightNum;

		_position = new Point( 0, 0, 0 );
		_direction = new Vector( 1, 1, 1 );
	}

	Point* SpotLight::getPosition() { return _position; }

	void SpotLight::setPosition( Point* pos ) { this->_position = pos; }

	void SpotLight::setPosition( Point pos ) {
		_position->setX( pos.getX() );
		_position->setY( pos.getY() );
		_position->setZ( pos.getZ() );
	}

	Vector* SpotLight::getDirection() { return _direction; }

	void SpotLight::setDirection( Vector* dir ) { this->_direction = dir; }

	void SpotLight::setDirection( Vector dir ) {
		_direction->setX( dir.getX() );
		_direction->setY( dir.getY() );
		_direction->setZ( dir.getZ() );
	}

	GLfloat SpotLight::getCutoff() { return _cutoff; }

	void SpotLight::setCutoff( GLfloat co ) { this->_cutoff = co; }

	GLfloat SpotLight::getExponent() { return _exponent; }

	void SpotLight::setExponent( GLfloat exp ) { this->_exponent = exp; }

	void SpotLight::shine() {
		glEnable( GL_LIGHT0 + this->getLightNumber() );

		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_DIFFUSE, this->getDiffuse() );
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_SPECULAR, this->getSpecular() );
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_AMBIENT, this->getAmbient() );
		
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_SPOT_CUTOFF, this->getCutoff() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_SPOT_EXPONENT, this->getExponent() );
		
		GLfloat pos[4];
		pos[0] = this->getPosition()->asVector()[0];
		pos[1] = this->getPosition()->asVector()[1];
		pos[2] = this->getPosition()->asVector()[2];
		pos[3] = this->getPosition()->asVector()[3];
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_POSITION, pos );
		
		GLfloat dir[4];
		this->getDirection()->normalize();
		dir[0] = this->getDirection()->asVector()[0];
		dir[1] = this->getDirection()->asVector()[1];
		dir[2] = this->getDirection()->asVector()[2];
		dir[3] = this->getDirection()->asVector()[3];
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_SPOT_DIRECTION, dir );
		
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_CONSTANT_ATTENUATION, this->getConstantAttenuation() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_LINEAR_ATTENUATION, this->getLinearAttenuation() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_QUADRATIC_ATTENUATION, this->getQuadraticAttenuation() );
	}

}
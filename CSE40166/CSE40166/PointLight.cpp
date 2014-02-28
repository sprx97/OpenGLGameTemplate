#include "PointLight.h"

namespace CSE40166 {
	
	PointLight::PointLight( GLint lightNum ) {
		_lightNumber = lightNum;
		_type = POINT;
		
		_position = new Point( 0, 0, 0 );
	}

	Point* PointLight::getPosition() { return _position; }
	void PointLight::setPosition( Point* pos ) { this->_position = pos; }

	void PointLight::setPosition( Point pos ) {
		_position->setX( pos.getX() );
		_position->setY( pos.getY() );
		_position->setZ( pos.getZ() );
	}

	void PointLight::shine() {
		glEnable( GL_LIGHT0 + this->getLightNumber() );
		
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_DIFFUSE, this->getDiffuse() );
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_SPECULAR, this->getSpecular() );
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_AMBIENT, this->getAmbient() );
		
		GLfloat pos[4];
		pos[0] = this->getPosition()->asVector()[0];
		pos[1] = this->getPosition()->asVector()[1];
		pos[2] = this->getPosition()->asVector()[2];
		pos[3] = this->getPosition()->asVector()[3];
		
		glLightfv( GL_LIGHT0 + this->getLightNumber(), GL_POSITION, pos );
		
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_CONSTANT_ATTENUATION, this->getConstantAttenuation() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_LINEAR_ATTENUATION, this->getLinearAttenuation() );
		glLightf( GL_LIGHT0 + this->getLightNumber(), GL_QUADRATIC_ATTENUATION, this->getQuadraticAttenuation() );
	}

}
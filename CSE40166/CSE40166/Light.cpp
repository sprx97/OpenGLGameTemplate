#include "Light.h"

namespace CSE40166 {
	
	Light::Light() {
		_on = false;
		_lightNumber = 0;
		
		_ambient[0] = 0.0;	_ambient[1] = 0.0;	_ambient[2] = 0.0;	_ambient[3] = 1.0;
		_diffuse[0] = 0.0;	_diffuse[1] = 0.0;	_diffuse[2] = 0.0;	_diffuse[3] = 1.0;
		_specular[0] = 0.0;	_specular[1] = 0.0;	_specular[2] = 0.0;	_specular[3] = 1.0;
		
		_cAtten = 1;
		_lAtten = 0;
		_qAtten = 0;
	}

	GLfloat* Light::getAmbient() { return _ambient; }
	void Light::setAmbient( GLfloat amb[4] ) { 
		this->_ambient[0] = amb[0]; 
		this->_ambient[1] = amb[1];
		this->_ambient[2] = amb[2];
		this->_ambient[3] = amb[3];
	}

	GLfloat* Light::getDiffuse() { return _diffuse; }
	void Light::setDiffuse( GLfloat diff[4] ) { 
		this->_diffuse[0] = diff[0]; 
		this->_diffuse[1] = diff[1]; 
		this->_diffuse[2] = diff[2]; 
		this->_diffuse[3] = diff[3]; 
	}

	GLfloat* Light::getSpecular() { return _specular; }
	void Light::setSpecular( GLfloat spec[4] ) { 
		this->_specular[0] = spec[0]; 
		this->_specular[1] = spec[1]; 
		this->_specular[2] = spec[2]; 
		this->_specular[3] = spec[3]; 
	}

	LightType Light::getLightType() { return _type; }

	GLint Light::getLightNumber() { return _lightNumber; }
	void Light::setLightNumber( GLint lNum ) { this->_lightNumber = lNum; }

	GLfloat Light::getConstantAttenuation() { return _cAtten; }
	void Light::setConstantAttenuation( GLfloat cA ) { this->_cAtten = cA; }

	GLfloat Light::getLinearAttenuation() { return _lAtten; }
	void Light::setLinearAttenuation( GLfloat lA ) { this->_lAtten = lA; }

	GLfloat Light::getQuadraticAttenuation() { return _qAtten; }
	void Light::setQuadraticAttenuation( GLfloat qA ) { this->_qAtten = qA; }

	void Light::turnLightOff() { glDisable( GL_LIGHT0 + _lightNumber );	_on = false; }
	void Light::turnLightOn() {  glEnable( GL_LIGHT0 + _lightNumber );  _on = true;  }

	bool Light::isLightOn() { return _on; }

}
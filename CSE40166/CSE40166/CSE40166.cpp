#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenAL/al.h>
#else
	#include <GL/gl.h>
	#include <AL/al.h>
#endif

#include "CSE40166.h"

#include <iostream>

namespace CSE40166 {

	void CSE40166Init( bool glutInit, bool alutInit ) {

		if( glutInit || alutInit )
			std::cout << "Using:" << std::endl;
	
		if( glutInit ) {
			std::cout << "      OpenGL Renderer: " << glGetString( GL_RENDERER ) << std::endl
			          << "      Vendor:          " << glGetString( GL_VENDOR ) << std::endl
	        		  << "      Version:         " << glGetString( GL_VERSION ) << std::endl
	    	      		  << "      Shading Version: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl
	        		  << std::endl;
		}

		if( alutInit ) {
			std::cout << "      OpenAL Renderer: " << alGetString( AL_RENDERER ) << std::endl
			          << "      Vendor:          " << alGetString( AL_VENDOR ) << std::endl
	        		  << "      Version:         " << alGetString( AL_VERSION ) << std::endl
	        		  << std::endl;
		}
	}

	void CSE40166Init() {
		CSE40166Init( true, false );
	}

}


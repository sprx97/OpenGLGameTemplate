CSE 40166 / 60166, Computer Graphics, Fall 2012
Jeffrey Paone

v1.4 12/12/2012

Code Example: CSE40166 Library

	The CSE40166 Library contains classes available for use for Points, Vectors,
Cameras, Lights, Materials, and Objects.  

Please be aware that the *.cpp files are very light on commenting (yes I know victim
of my own trap).  Most are self explainatory, the most complex is the Object class 
used for reading *.obj files.  The library is intended to be used as is, so compile 
the headers and install them.

Report any bugs to: jpaone@nd.edu

Compilation Instructions:
	Before compiling, FREEGLUT and SOIL need to be installed.  For installation
instructions consult the course website.

	Next, you must edit the Makefile and set the installation destination for
the CSE40166 library files.  Whatever folder you specify, a lib/ and include/
folder will be created inside there.  The compiled library file will be placed in
the lib/ folder and all the header files will be copied into the include/ folder.

	Next it is a three step process:

	1) make clean
		Will remove any pre-compiled *.o files
	2) make compile
		Will build each class and create the accompanying *.o file
	3) make install
		Will bundle the *.o files into a single library file and copy
		the files into the lib/ and include/ folders.

	The library is now ready to be used.

Usage: 
	Simply add an additional header to your code:

	#include <CSE40166/CSE40166.h>
	using namespace CSE40166;

	Each class is defined within the CSE40166 namespace so you may wish to
state which namespace to use (though you can access each class CSE40166::Camera
for example).  After calling glutCreateWindow(), call

	CSE40166Init( bool usingGLUT, bool usingALUT );

to set up the necessary headers and file information.  It will print some debug
information to the terminal telling which version of OpenGL you are using.

Notes:
	Report any bugs to: jpaone@nd.edu

Revision History:
v1.4	11/12/2012
    	- Per request, added toString() to PointBase class.
    	- Per request, added == and != operators to Point and Vector classes.

v1.3	11/09/2012
    	- Corrected bug when SOIL reads an object with a texture consisting of 4 
	    color channels.  Now properly reads in RGBA when registering texture.

v1.2	10/31/2012
	- Corrected bug when linking against multiple class and object files.
	    No longer gives multi-declaration error.
	- Added Camera::getView() per request which returns the view vector
	    for the camera.

v1.1	10/22/2012
	- Corrected bug in Object.cpp setting the Face properties correctly
	- Added OpenAL render info for CSE40166Init()

v1.0	10/17/2012
	 -Initial Release

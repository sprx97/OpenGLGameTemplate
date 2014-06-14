TARGET = main

CXX = g++
CFLAGS = -Wall -g -w -fno-rtti

ifeq ($(shell uname), Darwin)
	LIBS = -framework OpenGL -framework Cocoa -framework ApplicationServices -framework CoreFoundation -framework IOKit
else
	LIBS = -lX11 -ludev -lXrandr -lXinerama
	CFLAGS += -pthread
endif

# CSE40166 includes
CSE40166 = CSE40166
ifneq "$(wildcard $(CSE40166))" ""
	INCPATH += -I$(CSE40166)/include
	LIBPATH += -L$(CSE40166)/lib
	LIBS += -lCSE40166
endif


# FREEGLUT includes
ifeq ($(shell uname), Darwin)
	LIBS += -framework GLUT
else
	FREEGLUT = /linux/include/for/freeglut
	ifneq "$(wildcard $(FREEGLUT))" ""
		INCPATH += -I$(FREEGLUT)/include
		LIBPATH += -L$(FREEGLUT)/lib
	endif
	LIBS += -lglut
endif

# FREEALUT
ifeq ($(shell uname), Darwin)
	FREEALUT = /Library/freealut
else
	FREEALUT = /linux/include/for/freealut
endif
ifneq "$(wildcard $(FREEALUT))" ""
	INCPATH += -I$(FREEALUT)/include
	ifeq ($(shell uname), Darwin)
		LIBPATH += -L$(FREEALUT)/lib -Wl
	else
		LIBPATH += -L$(FREEALUT)/lib -Wl, --rpath -Wl, $(FREEALUT)/lib
	endif
endif
ifneq ($(shell uname), Darwin)
	LIBS += -lalut
endif

# OpenAL includes
ifeq ($(shell uname), Darwin)
	LIBS += -framework OpenAL
else
	OPENAL = /linux/include/for/OpenAL
	ifneq "$(wildcard $(OpenAL))" ""
		INCPATH += -I$(OPENAL)/include
		LIBPATH += -L$(OPENAL)/lib -Wl, --rpath -Wl,$(OPENAL)/lib
	endif
	LIBS += -lopenal
endif

# GLEW includes
ifeq ($(shell uname), Darwin)
	INCPATH += -I/usr/local/include
	LIBS += -lGLEW
else
	GLEW += /linux/include/for/GLEW
	ifneq "$(wildcard $(GLEW))" ""
		INCPATH += -I$(GLEW)/include
		LIBPATH += -L$(GLEW)/lib -Wl,--rpath -Wl,$(GLEW)/lib
	endif
	LIBS += -lGLEW -lGLU -lGL
endif

# SOIL includes
SOIL = SOIL
ifeq ($(shell uname), Darwin)
	LIBS += -framework CoreFoundation			
endif
INCPATH += -I$(SOIL)/include	
LIBPATH += -L$(SOIL)/lib
LIBS += -lSOIL

# OVR includes
LibOVR = LibOVR
ifeq ($(shell uname), Darwin)
LIBPATH += -L$(LibOVR)/Lib/MacOS/Release
else
LIBPATH += -L$(LibOVR)/Lib/Linux/Release/x86_64
endif
INCPATH += -I$(LibOVR)/Include
LIBS += -lovr

all: $(TARGET)

$(TARGET): main.o Terrain.o
	$(CXX) $(CFLAGS) $(INCPATH) -o $@ $^ $(LIBPATH) $(LIBS)

main.o: main.cpp globals.h
	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $<

Terrain.o: Terrain.cpp globals.h
	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $<

clean:
	rm -f *.o $(TARGET)

#depend:
#	rm -f Makefile.bak
#	mv Makefile Makefile.bak
#	sed '/^# DEPENDENCIES/,$$d' Makefile.bak > Makefile
#	echo '# DEPENDENCIES' >> Makefile
#	$(CXX) -MM *.cpp >> Makefile

#.c.o:
#	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $<

#.cc.o: 
#	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $^

#.cpp.o:
#	$(CXX) $(CFLAGS) $(INCPATH) -c -o $@ $^

#$(TARGET): main.o Terrain.o
#	$(CXX) $(CFLAGS) $(INCPATH) -o $@ $^ $(LIBPATH) $(LIBS)

# DEPENDENCIES
#main.o: main.cpp
#Terrain.o: Terrain.cpp

#ifndef _POINTBASE_H_
#define _POINTBASE_H_ 1

namespace CSE40166 {
	
	class PointBase {
	public:
		// GETTERS / SETTERS
		double getX();
		void setX( double x );
		double getY();
		void setY( double y );
		double getZ();
		void setZ( double z );
		
		double getW();

		/* return values as an array */
		double* asArray();
		/* deprecated due to poor naming!!! */
		double* asVector();
		
		/* return value as string in format "(x, y, z)" */
		char* toString();

	protected:
		// MEMBER VARIABLES
		double x,y,z,w;
	};

}

#endif

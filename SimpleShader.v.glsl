#version 120

varying vec2 oTexCoord;

void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	oTexCoord = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
} // should just  be a passthrough

// I think most of the Barrel Shader code should be in here
// Right now it's just a passthrough for a placeholder

// normal and lighting calculations are not done because 
// they already were done before rendering to the texture

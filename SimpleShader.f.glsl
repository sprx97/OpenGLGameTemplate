#version 120

uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;

uniform sampler2D texture;

varying vec2 oTexCoord;

void main(void) {
	vec4 texcolor = texture2D(texture, oTexCoord);
	gl_FragColor = vec4(vec3(texcolor), 1.0);
} 

// normal and lighting calculations are not done because 
// they already were done before rendering to the texture

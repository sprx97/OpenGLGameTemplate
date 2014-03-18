#version 120

uniform sampler2D texture;

void main(void) {
	vec4 texcolor = texture2D(texture, gl_TexCoord[0].st);
	gl_FragColor = vec4(vec3(texcolor), 1.0);
} 

// normal and lighting calculations are not done because 
// they already were done before rendering to the texture

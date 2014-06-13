#version 120

varying vec3 lightVector;
varying vec3 normalVector;

void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	// computer position and pass through texture coord

	normalVector = normalize( gl_NormalMatrix * gl_Normal );
	// computer normal vector

	lightVector = normalize(gl_LightSource[0].position.xyz - (gl_ModelViewMatrix * gl_Vertex).xyz);

	float dist = length(lightVector);
	float attenuation = 1.0 / (gl_LightSource[0].constantAttenuation + gl_LightSource[0].linearAttenuation * dist + gl_LightSource[0].quadraticAttenuation * dist * dist);
	lightVector = normalize(lightVector);
	// calculate light vector

}

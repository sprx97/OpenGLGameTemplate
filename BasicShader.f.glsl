uniform sampler2D texture;


varying vec3 normalVector;
varying vec3 lightVector;


void main(void) {
	float lamInt = max(0.0, dot(normalVector, lightVector));
	vec4 texcolor = texture2D(texture, gl_TexCoord[0].st);
	gl_FragColor = vec4(vec3(texcolor*gl_LightSource[0].diffuse*lamInt), 1.0);
}

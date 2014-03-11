uniform sampler2D texture;

void main(void) {
	vec4 texcolor = texture2D(texture, gl_TexCoord[0].st);
	gl_FragColor = texcolor;
}

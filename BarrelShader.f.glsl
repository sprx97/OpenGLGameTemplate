#version 120

//uniform vec2 LensCenter;
//uniform vec2 ScreenCenter;
//uniform vec2 Scale;
//uniform vec2 ScaleIn;
//uniform vec4 HmdWarpParam;

//uniform sampler2D texture;

//varying vec2 oTexCoord;

//void main(void) {
//	vec4 texcolor = texture2D(texture, oTexCoord);
//	gl_FragColor = vec4(vec3(texcolor), 1.0);
//} 

// normal and lighting calculations are not done because 
// they already were done before rendering to the texture

uniform sampler2D warpTexture;

const vec2 LeftLensCenter = vec2(0.2863248, 0.5);
const vec2 RightLensCenter = vec2(0.7136753, 0.5);
const vec2 LeftScreenCenter = vec2(0.25, 0.5);
const vec2 RightScreenCenter = vec2(0.75, 0.5);
const vec2 Scale = vec2(0.1469278, 0.2350845);
const vec2 ScaleIn = vec2(4, 2.5);
const vec4 HmdWarpParam = vec4(1, 0.22, 0.24, 0);

// Scales input texture coordinates for distortion.
vec2 HmdWarp(vec2 in01, vec2 LensCenter)
{
vec2 theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]
float rSq = theta.x * theta.x + theta.y * theta.y;
vec2 rvector = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
HmdWarpParam.z * rSq * rSq +
HmdWarpParam.w * rSq * rSq * rSq);
return LensCenter + Scale * rvector;
}

void main()
{
// The following two variables need to be set per eye
vec2 LensCenter = gl_FragCoord.x < 640 ? LeftLensCenter : RightLensCenter;
vec2 ScreenCenter = gl_FragCoord.x < 640 ? LeftScreenCenter : RightScreenCenter;

vec2 oTexCoord = gl_FragCoord.xy / vec2(1280, 800);

vec2 tc = HmdWarp(oTexCoord, LensCenter);
if (any(bvec2(clamp(tc,ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc)))
{
gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
return;
}

tc.x = gl_FragCoord.x < 640 ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));
gl_FragColor = texture2D(warpTexture, tc);
}
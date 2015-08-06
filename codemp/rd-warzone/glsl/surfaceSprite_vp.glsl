attribute vec4 attr_Color;
attribute vec2 attr_TexCoord0;
attribute vec3 attr_Position;

uniform mat4   u_ModelViewProjectionMatrix;

varying vec4   var_Color;

uniform vec2	u_Dimensions;
uniform vec3	u_ViewOrigin;
uniform float	u_Time;
uniform vec4	u_Local0; // (1=water, 2=lava), 0, 0, 0
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType

varying vec2	var_TexCoords;
varying float	time;
varying vec4	var_Local0; // (1=water, 2=lava), 0, 0, 0
varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec2	var_Dimensions;

float waveTime = u_Time;
//float waveWidth = 1.0;
//float waveHeight = 1.0;

//float waveTime = 0.5;
float waveWidth = 0.6;
float waveHeight = 1.0;
//waveFreq = 0.1;
 
void main(void)
{
	vec4 v = vec4(attr_Position.xyz, 1.0);
	v.z = sin(waveWidth * v.x + waveTime) * cos(waveWidth * v.y + waveTime) * waveHeight;
 	gl_Position = u_ModelViewProjectionMatrix * v;
	var_TexCoords = attr_TexCoord0.xy;
}

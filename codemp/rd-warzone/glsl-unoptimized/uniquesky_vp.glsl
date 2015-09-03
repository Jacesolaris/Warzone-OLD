uniform mat4	u_ModelViewProjectionMatrix;
attribute vec4	attr_Position;
attribute vec3	attr_Normal;
attribute vec2	attr_TexCoord0;

uniform vec2	u_Dimensions;
uniform vec3	u_ViewOrigin;

varying vec2	texCoord1;
varying vec2	var_Dimensions;
varying vec3	viewPos;
varying vec3	pPos;
varying vec3	viewAngles;

uniform float	u_Time;
varying float	time;

void main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position.xyz, 1.0);
	pPos = gl_Position.xyz;

	// transform position into world space
	viewPos = (u_ModelViewProjectionMatrix * vec4(u_ViewOrigin.xyz/*attr_Position.xyz*/, 1.0)).xyz;

	// compute view direction in world space
	viewAngles = normalize(u_ViewOrigin - viewPos);

	texCoord1 = attr_TexCoord0.st;
	var_Dimensions = u_Dimensions;

	time = u_Time;
}

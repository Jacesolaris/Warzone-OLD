attribute vec3	attr_Position;
attribute vec3	attr_Normal;
attribute vec4	attr_Tangent;

uniform vec3	u_ViewOrigin;

uniform vec4	u_PrimaryLightOrigin;
uniform float	u_PrimaryLightRadius;

varying vec4	var_Normal;
varying vec4	var_Tangent;
varying vec4	var_Bitangent;
varying vec3	var_ViewDir;

varying vec4	var_PrimaryLightDir;

void main()
{
	gl_Position = vec4(attr_Position.xyz, 1.0);
}

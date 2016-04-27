attribute vec3	attr_Position;
attribute vec3	attr_Normal;
attribute vec2	attr_TexCoord0;

uniform mat4	u_ModelViewProjectionMatrix;

varying vec2	var_TexCoords;
varying vec3	var_vertPos;
varying vec3	var_Normal;

void main()
{
	vec3 position  = attr_Position.xyz;
	vec3 normal    = attr_Normal * 2.0 - vec3(1.0);

	var_vertPos = position.xyz;
	var_TexCoords = attr_TexCoord0.st;
	var_Normal = normal;
	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);
}

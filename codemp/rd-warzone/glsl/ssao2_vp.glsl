attribute vec3	attr_Position;
attribute vec2	attr_TexCoord0;

uniform mat4	u_ModelViewProjectionMatrix;
uniform vec2	u_Dimensions;
uniform vec4	u_Local0;

varying vec2	var_ScreenTex;
varying vec2	var_Dimensions;
varying vec4	var_Local0;

void main()
{
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_ScreenTex = attr_TexCoord0.xy;
	var_Dimensions = u_Dimensions;
	var_Local0 = u_Local0;
}

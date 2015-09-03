attribute vec3	attr_Position;
attribute vec4	attr_TexCoord0;

uniform mat4	u_ModelViewProjectionMatrix;
uniform vec2	u_Dimensions;
uniform vec4	u_ViewInfo; // zfar / znear, zfar

varying vec2	var_ScreenTex;
varying vec2	var_Dimensions;
varying vec4	var_ViewInfo; // zfar / znear, zfar, znear, zfar

void main()
{
	//gl_Position = attr_Position;
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_ScreenTex = attr_TexCoord0.xy;
	//vec2 screenCoords = gl_Position.xy / gl_Position.w;
	//var_ScreenTex = screenCoords * 0.5 + 0.5;
	var_Dimensions = u_Dimensions;
	var_ViewInfo = u_ViewInfo;
}

in vec3 attr_Position;
in vec4 attr_TexCoord0;

uniform mat4   u_ModelViewProjectionMatrix;

uniform vec2	u_Dimensions;
uniform vec4	u_Local0; // depthScale, 0, 0, 0
uniform vec4   u_Local1; // eyex, eyey, eyexz, grassSway

varying vec2   var_TexCoords;
varying vec2   var_Dimensions;
varying vec4   var_Local0; // depthScale, 0, 0, 0
varying vec4   var_Local1; // eyex, eyey, eyexz, grassSway

void main()
{
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_TexCoords = attr_TexCoord0.st;
	var_Dimensions = u_Dimensions.st;
	var_Local0 = u_Local0.rgba;
	var_Local1 = u_Local1.rgba;
}

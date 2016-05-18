attribute vec3	attr_Position;
attribute vec2	attr_TexCoord0;

uniform mat4	u_ModelViewProjectionMatrix;

uniform vec3	u_ViewOrigin;
uniform vec4	u_PrimaryLightOrigin;

varying vec2   var_TexCoords;
varying vec3   var_vertPos;
varying vec3   var_viewOrg;
varying vec3   var_rayOrg;
varying vec3   var_sunOrg;
varying vec3   var_rayDir;
varying vec3   var_sunDir;

void main()
{
	var_TexCoords = attr_TexCoord0.st;

	vec4 vertPos = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);

	var_vertPos = attr_Position.xyz;
	var_viewOrg = u_ViewOrigin.xyz;
	var_sunOrg = u_PrimaryLightOrigin.xyz;
	var_rayDir = u_ViewOrigin.xyz - attr_Position.xyz;
	var_sunDir = u_PrimaryLightOrigin.xyz - attr_Position.xyz;

	var_rayOrg = var_viewOrg;

	gl_Position = vertPos;
}

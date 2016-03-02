attribute vec2 attr_TexCoord0;

attribute vec4 attr_Color;

attribute vec3 attr_Position;
attribute vec3 attr_Normal;
attribute vec4 attr_Tangent;

uniform vec3   u_ViewOrigin;

uniform mat4   u_ModelViewProjectionMatrix;
uniform mat4	u_ViewProjectionMatrix;
uniform mat4   u_ModelMatrix;

uniform float	u_Time;

uniform vec4   u_BaseColor;
uniform vec4   u_VertColor;

uniform vec4  u_PrimaryLightOrigin;
uniform float u_PrimaryLightRadius;

varying vec2   var_TexCoords;
varying vec2   var_TexCoords2;

varying vec4   var_Color;

varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
varying vec3   var_ViewDir;

#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP)
varying vec4   var_PrimaryLightDir;
#endif

varying vec3   var_vertPos;
varying float  var_Time;


out GroundData {
	vec2 TexCoords;
	vec3 Normal;
} grnd;


void main()
{
	grnd.Normal = attr_Normal.xyz;
	grnd.TexCoords = attr_TexCoord0.st;
	gl_Position = vec4(attr_Position.xyz, 1.0);
}

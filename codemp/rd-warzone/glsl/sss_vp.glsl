attribute vec3				attr_Position;
attribute vec2				attr_TexCoord0;

uniform mat4				u_ModelViewProjectionMatrix;
uniform mat4				u_invProjectionMatrix;
uniform vec2				u_Dimensions;
uniform vec4				u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec4				u_LightOrigin;

varying vec2				var_ScreenTex;
varying vec2				var_Dimensions;
varying vec2				projAB;
varying vec3				viewRay;
varying vec3				light_p;

void main()
{
	//var_ScreenTex = attr_TexCoord0.xy;
	var_ScreenTex.x = (gl_VertexID == 2)? 2.0: 0.0;
	var_ScreenTex.y = (gl_VertexID == 1)? 2.0: 0.0;

	//gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	gl_Position = vec4(2.0 * var_ScreenTex - 1.0, 0.0, 1.0);
	
	var_Dimensions = u_Dimensions;
	projAB = vec2( u_ViewInfo.y / (u_ViewInfo.y - u_ViewInfo.x), u_ViewInfo.y * u_ViewInfo.x / (u_ViewInfo.y - u_ViewInfo.x) );
	
	//Position on far and near planes
    vec4 p_near = u_invProjectionMatrix * vec4(2.0 * var_ScreenTex - 1.0, -1.0, 1.0);
    vec4 p_far  = u_invProjectionMatrix * vec4(2.0 * var_ScreenTex - 1.0, 1.0, 1.0);
    p_near /= p_near.w;
    p_far  /= p_far.w;
	vec3 ray_direction = p_far.xyz - p_near.xyz;
	//vec3 ray_origin    = p_near.xyz - p_near.z * (ray_direction.xyz / ray_direction.z);
	viewRay = ray_direction;//ray_origin;

	//light_p = u_LightOrigin.xyz - (gl_Position.xyz * u_LightOrigin.w);
	light_p = (u_ModelViewProjectionMatrix * u_LightOrigin).xyz;
	//light_p = u_LightOrigin.xyz;
	//light_p = vec3(-30.0, 30.0, -10.0);


	//var_ScreenTex = attr_TexCoord0;
    //viewRay = (u_invProjectionMatrix * vec4(attr_Position, 1.0)).xyz;
    //gl_Position = vec4(attr_Position, 1.0);
}

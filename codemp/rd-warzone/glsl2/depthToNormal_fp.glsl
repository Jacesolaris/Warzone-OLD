uniform sampler2D		u_ScreenDepthMap;

uniform vec4			u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2			u_Dimensions;

varying vec2			var_TexCoords;

//#######################################
#define width			u_Dimensions.x
#define height			u_Dimensions.y

#define znear			u_ViewInfo.r									//camera clipping start
#define zfar			u_ViewInfo.g									//camera clipping end
#define fov				1.0												//check your camera settings, set this to (90.0 / fov) (make sure you put a ".0" after your number)
#define aspectratio		(u_Dimensions.x/u_Dimensions.y)					//width / height (make sure you put a ".0" after your number)
//#######################################

float getDepth(vec2 coord) {
    float zdepth = texture(u_ScreenDepthMap, coord).x;
    return -zfar * znear / (zdepth * (zfar - znear) - zfar);
	//return 1.0 / mix(u_ViewInfo.x, 1.0, zdepth);
}

vec3 getViewPosition(vec2 coord) {
    vec3 pos = vec3((coord.s * 2.0 - 1.0) / fov, (coord.t * 2.0 - 1.0) / aspectratio / fov, 1.0);
    return (pos * getDepth(coord));
}

vec3 getViewNormal(vec2 coord){
    
    vec3 p0 = getViewPosition(coord);
    vec3 p1 = getViewPosition(coord + vec2(1.0 / width, 0.0));
    vec3 p2 = getViewPosition(coord + vec2(0.0, 1.0 / height));

    vec3 dx = p1 - p0;
    vec3 dy = p2 - p0;
    return normalize(cross( dy , dx ));
}

void main()
{
	gl_FragColor = vec4(getViewNormal(var_TexCoords).xyz * 0.5 + 0.5, 1.0);
}

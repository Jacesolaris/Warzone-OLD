uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_ScreenDepthMap;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;

varying vec2		var_TexCoords;

#define znear		u_ViewInfo.r									//camera clipping start
#define zfar		u_ViewInfo.g									//camera clipping end
#define zfar2		u_ViewInfo.a									//camera clipping end

float linearize(float depth)
{
	float d = depth;
	d /= u_ViewInfo.z - depth * u_ViewInfo.z + depth;
	return clamp(d * znear, 0.0, 1.0);
}

float getDepth(vec2 coord) {
    return linearize(texture(u_ScreenDepthMap, coord).r);
}

float linearize2(float depth)
{
	float d = depth;
	d /= zfar2 - depth * zfar2 + depth;
	return clamp(d * znear, 0.0, 1.0);
}

float getDepth2(vec2 coord) {
    return linearize2(texture(u_ScreenDepthMap, coord).r);
}

vec3 getViewPosition(vec2 coord) {
    vec3 pos = vec3((coord.s * 2.0 - 1.0), (coord.t * 2.0 - 1.0) / (u_Dimensions.x/u_Dimensions.y), 1.0);
    return (pos * getDepth2(coord));
}

vec3 getViewNormal(vec2 coord) {
    
    vec3 p0 = getViewPosition(coord);
    vec3 p1 = getViewPosition(coord + vec2(1.0 / u_Dimensions.x, 0.0));
    vec3 p2 = getViewPosition(coord + vec2(0.0, 1.0 / u_Dimensions.y));

    vec3 dx = p1 - p0;
    vec3 dy = p2 - p0;
    return normalize(cross( dy , dx ));
}

const vec3 PLUMA_COEFFICIENT = vec3(0.2126, 0.7152, 0.0722);

float plumaAtCoord(vec2 coord) {
  vec3 pixel = texture(u_DiffuseMap, coord).rgb;
  
  //vec3 gamma = vec3(1.0/2.2);
  //pixel = pow(pixel, gamma);
  
  float luma = dot(pixel, PLUMA_COEFFICIENT);
  return luma;
}

float GetDisplacementAtCoord(vec2 coord)
{
	vec4 position = texture(u_PositionMap, coord);
	if (position.a != 5.0 
		&& position.a != 6.0 
		&& position.a != 8.0
		&& position.a != 27.0
		&& position.a != 9.0
		&& position.a != 23.0
		&& position.a != 1.0
		&& position.a != 2.0
		&& position.a != 17.0
		&& position.a != 7.0
		&& position.a != 11.0
		&& position.a != 28.0
		&& position.a != 15.0)
	{
		return 0.0;
	}

	float displacement = 1.0 - clamp(plumaAtCoord(coord), 0.0, 1.0);

// Contrast...
#define contLower ( 64.0/*96.0*/ / 255.0 )
#define contUpper (255.0 / 192.0 )
	displacement = clamp((clamp(displacement - contLower, 0.0, 1.0)) * contUpper, 0.0, 1.0);
	
	return displacement;
}

void main(void)
{
	vec2 texCoords = var_TexCoords;

	float depth = getDepth(texCoords);
	float invDepth = 1.0 - depth;
	vec3 norm = getViewNormal(texCoords);
	float displacement = texture(u_PositionMap, texCoords).r;

	if (u_Local1.g >= 5.0)
	{
		gl_FragColor = vec4(displacement / u_Local1.b, displacement / u_Local1.b, displacement / u_Local1.b, 1.0);
		return;
	}

	if (u_Local1.g >= 4.0)
	{
		gl_FragColor = vec4(norm.xyz * 0.5 + 0.5, 1.0);
		return;
	}

	if (u_Local1.g >= 3.0)
	{
		gl_FragColor = vec4(depth, depth, depth, 1.0);
		return;
	}

	if (u_Local1.g >= 2.0)
	{
		gl_FragColor = vec4(invDepth, invDepth, invDepth, 1.0);
		return;
	}

	if (u_Local1.g >= 1.0)
	{
		gl_FragColor = vec4(vec3(GetDisplacementAtCoord(texCoords)), 1.0);
		return;
	}

	vec3 offsetDir = norm;
	vec2 ParallaxXY = offsetDir.xy * vec2(u_Local1.r/*-16.0*/ / u_Dimensions) * invDepth;
	texCoords += ParallaxXY * displacement;

	gl_FragColor = vec4(textureLod(u_DiffuseMap, texCoords, 0.0).rgb, 1.0);
}

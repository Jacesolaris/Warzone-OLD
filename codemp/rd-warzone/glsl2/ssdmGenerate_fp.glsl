uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;

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
	if (position.a-1.0 != 5.0 
		&& position.a-1.0 != 6.0 
		&& position.a-1.0 != 8.0
		&& position.a-1.0 != 27.0
		&& position.a-1.0 != 9.0
		&& position.a-1.0 != 23.0
		&& position.a-1.0 != 1.0
		&& position.a-1.0 != 2.0
		&& position.a-1.0 != 17.0
		&& position.a-1.0 != 7.0
		&& position.a-1.0 != 11.0
		&& position.a-1.0 != 28.0
		&& position.a-1.0 != 15.0)
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

float ReliefMapping(vec2 dp, vec2 ds, float origDepth)
{
	const int linear_steps = 10;
	const int binary_steps = 5;
	float depth_step = 1.0 / linear_steps;
	float size = depth_step;
	float depth = 1.0;
	float best_depth = 1.0;

	for (int i = 0 ; i < linear_steps - 1 ; ++i) 
	{
		depth -= size;

		/*if (origDepth > getDepth(dp + ds * depth))
		{
			break;
		}*/

		float t = GetDisplacementAtCoord(dp + ds * depth);
		if (depth >= 1.0 - t)
			best_depth = depth;
	}

	depth = best_depth - size;

	for (int i = 0 ; i < binary_steps ; ++i) 
	{
		/*if (origDepth > getDepth(dp + ds * depth))
		{
			break;
		}*/

		size *= 0.5;
		float t = GetDisplacementAtCoord(dp + ds * depth);
		if (depth >= 1.0 - t) {
			best_depth = depth;
			depth -= 2 * size;
		}
		depth += size;
	}

	return clamp(best_depth, 0.0, 1.0);
}

void main(void)
{
	//vec4 position = texture(u_PositionMap, var_TexCoords);

	float depth = getDepth(var_TexCoords);
	float invDepth = 1.0 - depth;
	vec3 norm = getViewNormal(var_TexCoords);
	vec3 offsetDir = norm;
	vec2 ParallaxXY = offsetDir.xy * vec2(u_Local1.r/*-16.0*/ / u_Dimensions) * invDepth;
	float displacement = ReliefMapping(var_TexCoords, ParallaxXY, depth);
	gl_FragColor = vec4(displacement, displacement, displacement, 1.0);
}

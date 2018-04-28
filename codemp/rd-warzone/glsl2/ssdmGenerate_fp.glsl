uniform sampler2D				u_DiffuseMap;
uniform sampler2D				u_ScreenDepthMap;
uniform sampler2D				u_PositionMap;
uniform sampler2D				u_NormalMap;
uniform sampler2D				u_GlowMap;

uniform vec2					u_Dimensions;

uniform vec4					u_Local1; // DISPLACEMENT_MAPPING_STRENGTH, r_testShaderValue1, r_testShaderValue2, r_testShaderValue3

uniform vec4					u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3					u_ViewOrigin;

varying vec2					var_TexCoords;

#define DISPLACEMENT_STRENGTH	u_Local1.r

#define znear					u_ViewInfo.r									//camera clipping start
#define zfar					u_ViewInfo.g									//camera clipping end
#define zfar2					u_ViewInfo.a									//camera clipping end

/*
vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}
*/

float getDepth(vec2 coord) {
    return texture(u_ScreenDepthMap, coord).r;
}

float linearize2(float depth)
{
	float d = depth;
	d /= zfar2 - depth * zfar2 + depth;
	return clamp(d * znear, 0.0, 1.0);
}

float getDepth2(vec2 coord) {
    //return linearize2(texture(u_ScreenDepthMap, coord).r);
	return texture(u_ScreenDepthMap, coord).r;
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
  float luma = dot(pixel, PLUMA_COEFFICIENT);
  return luma;
}

float GetDisplacementAtCoord(vec2 coord)
{
#if 1
	if (texture(u_NormalMap, coord).b < 1.0)
	{
		return 0.0;
	}
#endif

	vec2 coord2 = coord;
	coord2.y = 1.0 - coord2.y;
	vec3 gMap = texture(u_GlowMap, coord2).rgb;													// Glow map strength at this pixel
	float invGlowStrength = 1.0 - clamp(max(gMap.r, max(gMap.g, gMap.b)), 0.0, 1.0);

	float displacement = invGlowStrength * clamp(plumaAtCoord(coord), 0.0, 1.0);

// Contrast...
#define contLower ( 16.0 / 255.0 )
#define contUpper (255.0 / 156.0 )
	displacement = clamp((clamp(displacement - contLower, 0.0, 1.0)) * contUpper, 0.0, 1.0);
	
	return displacement;
}

float ReliefMapping(vec2 dp, vec2 ds, float origDepth, float materialMultiplier)
{
	//return clamp(GetDisplacementAtCoord(dp + ds), 0.0, 1.0);

#ifdef __HQ_PARALLAX__
	int linear_steps = 10 * int(materialMultiplier);
	int binary_steps = 5 * int(materialMultiplier);
#else //!__HQ_PARALLAX__
	const int linear_steps = 4;// 5;// 10;
	const int binary_steps = 2;// 5;
#endif //__HQ_PARALLAX__
	float size = 1.0 / linear_steps;
	float depth = 1.0;
	float best_depth = 1.0;

	for (int i = 0 ; i < linear_steps - 1 ; ++i) 
	{
		depth -= size;
		float t = GetDisplacementAtCoord(dp + ds * depth);
		if (depth >= t)
			best_depth = depth;
	}

	depth = best_depth - size;

	for (int i = 0 ; i < binary_steps ; ++i) 
	{
		size *= 0.5;
		float t = GetDisplacementAtCoord(dp + ds * depth);
		if (depth >= t) {
			best_depth = depth;
			depth -= 2 * size;
		}
		depth += size;
	}

	return clamp(best_depth, 0.0, 1.0);
}

void main(void)
{
	if (texture(u_NormalMap, var_TexCoords).b < 1.0)
	{// An if based on output from a texture, but seems to increase FPS a little anyway...
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 norm = getViewNormal(var_TexCoords);
	float material = texture(u_PositionMap, var_TexCoords).a - 1.0;
	float materialMultiplier = 1.0;

	if (material == MATERIAL_ROCK)
	{// Rock gets more displacement...
		materialMultiplier = 6.0;
	}
	else if (material == MATERIAL_SOLIDWOOD)
	{// Rock gets more displacement...
		materialMultiplier = 3.0;
	}

#if 1
	vec2 coord2 = var_TexCoords;
	coord2.y = 1.0 - coord2.y;
	vec3 gMap = texture(u_GlowMap, coord2).rgb;													// Glow map strength at this pixel
	float invGlowStrength = 1.0 - clamp(max(gMap.r, max(gMap.g, gMap.b)), 0.0, 1.0);

	float depth = getDepth(var_TexCoords);
	float invDepth = 1.0 - depth;
	vec2 ParallaxXY = norm.xy * vec2((-DISPLACEMENT_STRENGTH * materialMultiplier) / u_Dimensions) * invDepth;
	float displacement = invGlowStrength * ReliefMapping(var_TexCoords, ParallaxXY, depth, materialMultiplier);
#else
	float displacement = GetDisplacementAtCoord(var_TexCoords);
#endif
	
	gl_FragColor = vec4(displacement, norm.x * 0.5 + 0.5, norm.y * 0.5 + 0.5, 1.0);
	//gl_FragColor = vec4(displacement, EncodeNormal(norm.xyz), 1.0);
}

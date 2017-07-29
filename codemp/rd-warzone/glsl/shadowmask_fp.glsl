uniform sampler2D		u_ScreenDepthMap;

uniform sampler2D		u_ShadowMap;
uniform sampler2D		u_ShadowMap2;
uniform sampler2D		u_ShadowMap3;
uniform sampler2D		u_ShadowMap4;

uniform mat4			u_ShadowMvp;
uniform mat4			u_ShadowMvp2;
uniform mat4			u_ShadowMvp3;
uniform mat4			u_ShadowMvp4;

uniform vec4			u_Settings0;			// shadowQuality, r_shadowMapSize, r_shadowCascadeZFar, 0.0
uniform vec3			u_ViewOrigin;
uniform vec4			u_ViewInfo;				// zfar / znear, zfar, depthBits
uniform float			u_ShadowZfar[5];

#define					r_shadowQuality			u_Settings0.r
#define					r_shadowMapSize			u_Settings0.g
#define					r_shadowCascadeZFar		u_Settings0.b

precise varying vec2	var_DepthTex;
precise varying vec3	var_ViewDir;

precise float DEPTH_MAX_ERROR = (1.0 / pow(2.0, u_ViewInfo.b));

precise float scale = 1.0 / r_shadowMapSize;

float getLinearDepth(sampler2D depthMap, vec2 tex)
{
	precise float sampleZDivW = texture(depthMap, tex).r;
	sampleZDivW -= DEPTH_MAX_ERROR;
	return 1.0 / mix(u_ViewInfo.x, 1.0, sampleZDivW);
}

float shadowPCF2(const sampler2D shadowmap, const vec2 st, const float dist)
{
	precise float mult = 0.0;

	mult += step(dist, texture(shadowmap, st).r + 0.005) * 3.0;
	mult += step(dist, texture(shadowmap, st + (scale * vec2(-1.0, 0.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(0.0, -1.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(1.0, 0.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(0.0, 1.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(1.0, 1.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(-1.0, -1.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(1.0, -1.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(-1.0, 1.0))).r + 0.005);
	mult /= 11.0;

	return mult;
}

float shadowPCF(const sampler2D shadowmap, const vec2 st, const float dist)
{
	precise float mult = 0.0;

	mult += step(dist, texture(shadowmap, st).r + 0.005) * 2.0;
	mult += step(dist, texture(shadowmap, st + (scale * vec2(-1.0, 0.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(0.0, -1.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(1.0, 0.0))).r + 0.005);
	mult += step(dist, texture(shadowmap, st + (scale * vec2(0.0, 1.0))).r + 0.005);
	mult /= 6.0;

	return mult;
}

float shadow(const sampler2D shadowmap, const vec2 st, const float dist)
{
	return step(dist, texture(shadowmap, st).r + 0.005);
}

void main()
{
	precise float result = 1.0;
	precise float depth = getLinearDepth(u_ScreenDepthMap, var_DepthTex);
	precise float sampleZ = u_ViewInfo.y * depth;

	if (sampleZ <= u_ShadowZfar[0])
	{
		precise vec4 biasPos = precise vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
		precise vec4 shadowpos = u_ShadowMvp * biasPos;
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = shadow(u_ShadowMap, shadowpos.xy, shadowpos.z);
	}
	else if (sampleZ <= u_ShadowZfar[1])
	{
		precise vec4 biasPos = precise vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
		precise vec4 shadowpos = u_ShadowMvp2 * biasPos;
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = shadowPCF(u_ShadowMap2, shadowpos.xy, shadowpos.z);
	}
	else if (sampleZ <= u_ShadowZfar[2])
	{
		precise vec4 biasPos = precise vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
		precise vec4 shadowpos = u_ShadowMvp3 * biasPos;
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = shadowPCF2(u_ShadowMap3, shadowpos.xy, shadowpos.z);
	}
	/*else if (sampleZ <= u_ShadowZfar[3])
	{
		precise vec4 biasPos = precise vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
		precise vec4 shadowpos = u_ShadowMvp4 * biasPos;
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = shadow(u_ShadowMap4, shadowpos.xy, shadowpos.z);
	}*/
	
	gl_FragColor = vec4(result, depth, 0.0, 1.0);
}

uniform sampler2D u_ScreenDepthMap;

uniform sampler2D u_ShadowMap;

#if defined(USE_SHADOW_CASCADE2)
uniform sampler2D u_ShadowMap2;
uniform sampler2D u_ShadowMap3;
#if defined(USE_SHADOW_CASCADE3)
uniform sampler2D u_ShadowMap4;
uniform sampler2D u_ShadowMap5;
#endif
#elif defined(USE_SHADOW_CASCADE)
uniform sampler2D u_ShadowMap2;
#endif

uniform mat4      u_ShadowMvp;
#if defined(USE_SHADOW_CASCADE2)
uniform mat4      u_ShadowMvp2;
uniform mat4      u_ShadowMvp3;
#if defined(USE_SHADOW_CASCADE3)
uniform mat4      u_ShadowMvp4;
uniform mat4      u_ShadowMvp5;
#endif
#elif defined(USE_SHADOW_CASCADE)
uniform mat4      u_ShadowMvp2;
#endif

uniform vec4   u_Settings0; // r_shadowMaxDepthError->value, r_shadowSolidityValue->value, 0.0, 0.0
uniform vec3   u_ViewOrigin;
uniform vec4   u_ViewInfo; // zfar / znear, zfar

varying vec2   var_DepthTex;
varying vec3   var_ViewDir;

// depth is GL_DEPTH_COMPONENT24
// so the maximum error is 1.0 / 2^24
#define DEPTH_MAX_ERROR 0.000000059604644775390625

float getLinearDepth(sampler2D depthMap, vec2 tex, float zFarDivZNear)
{
	float sampleZDivW = texture2D(depthMap, tex).r;
	sampleZDivW -= DEPTH_MAX_ERROR;
	return 1.0 / mix(zFarDivZNear, 1.0, sampleZDivW);
}

float PCF(const sampler2D shadowmap, const vec2 st, const float dist)
{
	float depth = texture2D(shadowmap, st).r;
	float mult = step(dist, depth/* + u_Settings0.r*/);
	return mult;
}

#if defined(USE_SHADOW_CASCADE) || defined(USE_FAST_SHADOW)
const float blendRange1 = 1024.0;
const float blendRange2 = 4096.0;
const float blendRange3 = 65536.0;
#endif


void main()
{
	float result;
	
	float depth = getLinearDepth(u_ScreenDepthMap, var_DepthTex, u_ViewInfo.x);
	float sampleZ = u_ViewInfo.y * depth;

	vec4 biasPos = vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
	
	vec4 shadowpos = u_ShadowMvp * biasPos;
	
#if defined(USE_SHADOW_CASCADE) || defined(USE_SHADOW_CASCADE2) || defined(USE_FAST_SHADOW)
	const float fadeTo = 1.0;
	//const float fadeTo = 0.0;
	result = fadeTo;
#else
	result = 0.0;
#endif

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCF(u_ShadowMap, shadowpos.xy, shadowpos.z);
	}

#if defined(USE_FAST_SHADOW)
	if (sampleZ / blendRange1 >= 1.0)
	{// In blend range...
		float fade = clamp((sampleZ-blendRange1) / blendRange2, 0.0, 1.0);
		result = mix(result, fadeTo, fade);
	}
#elif defined(USE_SHADOW_CASCADE)
	// Better looking blend... Only 2 levels to improve FPS...
	float result2 = fadeTo;

	shadowpos = u_ShadowMvp2 * biasPos;

	shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
	result2 = PCF(u_ShadowMap2, shadowpos.xy, shadowpos.z);

	if (sampleZ / blendRange1 >= 1.0)
	{// In blend range...
		if (sampleZ / blendRange2 < 1.0)
		{
			float fade = clamp((sampleZ-blendRange1) / blendRange2, 0.0, 1.0);
			result = mix(result, result2, fade);
		}
		else
		{
			float fade = clamp((sampleZ-blendRange2) / blendRange3, 0.0, 1.0);
			result = mix(result2, fadeTo, fade);
		}
	}
#elif defined(USE_SHADOW_CASCADE2)
	else
	{
		shadowpos = u_ShadowMvp2 * biasPos;

		if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
		{
			shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
			result = PCF(u_ShadowMap2, shadowpos.xy, shadowpos.z);
		}
	#if defined(USE_SHADOW_CASCADE3)
		else
		{
			shadowpos = u_ShadowMvp3 * biasPos;

			if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
			{
				shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
				result = PCF(u_ShadowMap3, shadowpos.xy, shadowpos.z);
			}
			else
			{
				shadowpos = u_ShadowMvp4 * biasPos;

				if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
				{
					shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
					result = PCF(u_ShadowMap4, shadowpos.xy, shadowpos.z);
				}
				else
				{
					shadowpos = u_ShadowMvp5 * biasPos;

					if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
					{
						shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
						result = PCF(u_ShadowMap5, shadowpos.xy, shadowpos.z);
					}
				}
			}
		}
	#else
		else
		{
			shadowpos = u_ShadowMvp3 * biasPos;

			if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
			{
				shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
				result = PCF(u_ShadowMap3, shadowpos.xy, shadowpos.z);

				float fade = clamp(sampleZ / r_shadowCascadeZFar * 10.0 - 9.0, 0.0, 1.0);
				result = mix(result, fadeTo, fade);
			}

			float fadeStart = abs(shadowpos.w);

			if (sampleZ > r_shadowCascadeZFar) 
			{
				result = 1.0;
			}
			else if (sampleZ > fadeStart)
			{
				result = 1.0 - result;
				result *= (1.0 - pow(clamp(sampleZ - fadeStart, 0.0, r_shadowCascadeZFar) / r_shadowCascadeZFar, 4.0));
				result = 1.0 - result;
			}
		}
	#endif
	}
#endif
		
	//gl_FragColor = vec4(vec3(result), 1.0);
	gl_FragColor = vec4(vec3(result), 0.5);
}

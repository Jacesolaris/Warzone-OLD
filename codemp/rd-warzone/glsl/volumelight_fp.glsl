#define __USING_SHADOW_MAP__

uniform sampler2D			u_ScreenDepthMap;

uniform vec4				u_Local0; // 0.0, 0.0, r_volumeLightStrength * SUN_VOLUMETRIC_SCALE, SUN_VOLUMETRIC_FALLOFF
uniform vec4				u_Local1; // nightScale, r_testvalue0->value, r_testvalue1->value, r_testvalue2->value

// General options...
#define VOLUMETRIC_STRENGTH		u_Local0.b


#if defined(HQ_VOLUMETRIC)
const int	iVolumetricSamples = 32;
const float	fVolumetricDecay = 0.96875;
#elif defined (MQ_VOLUMETRIC)
const int	iVolumetricSamples = 16;
const float	fVolumetricDecay = 0.9375;
#else //!defined(HQ_VOLUMETRIC) && !defined(MQ_VOLUMETRIC)
const int	iVolumetricSamples = 8;
const float	fVolumetricDecay = 0.875;
#endif //defined(HQ_VOLUMETRIC) && defined(MQ_VOLUMETRIC)

const float	fVolumetricWeight = 0.5;
const float	fVolumetricDensity = 1.0;
const float fVolumetricFalloffRange = 0.4;



uniform vec3				u_vlightColors;


#ifndef __USING_SHADOW_MAP__
uniform vec2				u_vlightPositions;

varying vec2				var_TexCoords;

void main ( void )
{
	if (u_Local1.r >= 1.0)
	{// Night...
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3	totalColor = vec3(0.0, 0.0, 0.0);
	vec3	lightColor = u_vlightColors * 1.5;

	if (u_Local1.r > 0.0)
	{// Adjust the sun color at sunrise/sunset...
		vec3 sunsetSun = vec3(2.0, 0.3, 0.1);
		lightColor = mix(lightColor, sunsetSun, u_Local1.r/*pow(u_Local1.r, u_Local1.g)*/);
	}

	float dist = length(var_TexCoords - u_vlightPositions);
	float fall = pow(clamp((2.0 - dist) / 2.0, 0.0, 1.0), 2.0);

	float	lens = 0.0;
	vec2	texCoord = var_TexCoords;
	vec2	deltaTexCoord = (texCoord.xy - u_vlightPositions);

	deltaTexCoord *= 1.0 / float(float(iVolumetricSamples) * fVolumetricDensity);

	float illuminationDecay = 1.0;

//#pragma unroll iVolumetricSamples
	for(int g = 0; g < iVolumetricSamples; g++)
	{
		texCoord -= deltaTexCoord;

		if (texCoord.x >= 0.0 && texCoord.x <= 1.0 && texCoord.y >= 0.0 && texCoord.y <= 1.0)
		{// Don't bother with lookups outside screen area...
			float linDepth = textureLod(u_ScreenDepthMap, texCoord.xy, 0.0).r;
			lens += linDepth * illuminationDecay * fVolumetricWeight;
		}

		illuminationDecay *= fVolumetricDecay;

		if (illuminationDecay <= 0.0)
			break;
	}

	totalColor += clamp(lightColor * (lens * 0.05) * pow(fall, u_Local0.a), 0.0, 1.0);

	totalColor.rgb += u_vlightColors * 0.05;

	totalColor.rgb *= VOLUMETRIC_STRENGTH * 1.5125;

	// Amplify contrast...
#define lightLower ( 64.0 / 255.0 )
#define lightUpper ( 255.0 / 64.0 )
	totalColor.rgb = clamp((totalColor.rgb - lightLower) * lightUpper, 0.0, 1.0);

	if (u_Local1.r > 0.0)
	{// Sunset, Sunrise, and Night times... Scale down screen color, before adding lighting...
		vec3 nightColor = vec3(0.0);
		totalColor.rgb = mix(totalColor.rgb, nightColor, u_Local1.r);
	}

	gl_FragColor = vec4(totalColor, 1.0);
}

#else //__USING_SHADOW_MAP__

uniform sampler2D			u_PositionMap;

uniform sampler2DShadow		u_ShadowMap;
uniform sampler2DShadow		u_ShadowMap2;
uniform sampler2DShadow		u_ShadowMap3;
uniform sampler2DShadow		u_ShadowMap4;

uniform mat4				u_ShadowMvp;
uniform mat4				u_ShadowMvp2;
uniform mat4				u_ShadowMvp3;
uniform mat4				u_ShadowMvp4;

uniform vec4				u_Settings0;			// r_shadowSamples (numBlockerSearchSamples), r_shadowMapSize, r_testshaderValue1->value, r_testshaderValue2->value
uniform vec3				u_ViewOrigin;
uniform vec4				u_PrimaryLightOrigin;
uniform vec3				u_ViewForward;
uniform vec4				u_ViewInfo;				// zfar / znear, zfar, depthBits, znear

varying vec2				var_DepthTex;
varying vec3				var_ViewDir;

#define						r_shadowMapSize			u_Settings0.g

float offset_lookup(sampler2DShadow shadowmap, vec4 loc, vec2 offset, float scale)
{
	float result = textureProj(shadowmap, vec4(loc.xy + offset * scale * loc.w, loc.z, loc.w)) > 0.1 ? 1.0 : 0.0;
	return result;
}

float PCF(const sampler2DShadow shadowmap, const vec4 st, const float dist, float scale)
{
	float mult;
	vec4 sCoord = vec4(st);

	//vec2 offset = vec2(greaterThan(fract(st.xy * 0.5), vec2(0.25)));  // mod
	vec2 offset = mod(sCoord.xy, 0.5);
	offset.y += offset.x;  // y ^= x in floating point
	if (offset.y > 1.1) offset.y = 0;
	/*float shadowCoeff = (offset_lookup(shadowmap, sCoord, offset + vec2(-1.5, 0.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(0.5, 0.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(-1.5, -1.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(0.5, -1.5), scale) ) 
			   * 0.25;

	return shadowCoeff;*/
	return offset_lookup(shadowmap, sCoord, offset, 0.0);
}

//////////////////////////////////////////////////////////////////////////
float GetVolumetricShadow(void)
{
	float result = 1.0;
	float fWeight = 0.0;
	float dWeight = 1.0;
	float invSamples = 1.0 / float(iVolumetricSamples);
	float sceneDepth = texture(u_ScreenDepthMap, var_DepthTex).x * 0.4;

	for (int i = 0; i < iVolumetricSamples; i++)
	{
		float depth = (i / float(iVolumetricSamples)) * sceneDepth;
		//float depth = (1.0 - (i / float(iVolumetricSamples))) * sceneDepth;
		vec4 biasPos = vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
		vec4 shadowpos = u_ShadowMvp * biasPos;

		if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
		{
			shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
			result += PCF(u_ShadowMap, shadowpos, shadowpos.z, 1.0 / r_shadowMapSize) * dWeight;
			dWeight -= invSamples;
			fWeight += invSamples;
			continue;
		}

		shadowpos = u_ShadowMvp2 * biasPos;

		if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
		{
			shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
			result += PCF(u_ShadowMap2, shadowpos, shadowpos.z, 1.0 / r_shadowMapSize) * dWeight;
			dWeight -= invSamples;
			fWeight += invSamples;
			continue;
		}

		shadowpos = u_ShadowMvp3 * biasPos;

		if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
		{
			shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
			result += PCF(u_ShadowMap3, shadowpos, shadowpos.z, 1.0 / (r_shadowMapSize * 2.0)) * dWeight;
			dWeight -= invSamples;
			fWeight += invSamples;
			continue;
		}
	}
	
	result /= fWeight;
	return result;
}

void main()
{
	if (u_Local1.r >= 1.0)
	{// Night...
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 lightColor = u_vlightColors;

	if (u_Local1.r > 0.0)
	{// Adjust the sun color at sunrise/sunset...
		vec3 sunsetSun = vec3(2.0, 0.3, 0.1);
		lightColor = mix(lightColor, sunsetSun, u_Local1.r/*pow(u_Local1.r, u_Local1.g)*/);
	}

	float shadow = GetVolumetricShadow();

	vec3 totalColor = lightColor * shadow;

	totalColor.rgb *= VOLUMETRIC_STRENGTH;// * 1.5125;

	
	// Amplify contrast...
#define lightLower ( 512.0 / 255.0 )
#define lightUpper ( 255.0 / 4096.0 )
	totalColor.rgb = clamp((totalColor.rgb - lightLower) * lightUpper, 0.0, 1.0);
	
	vec3 pMap = texture(u_PositionMap, var_DepthTex).xyz;
	vec3 lDir = normalize(u_PrimaryLightOrigin.xyz - u_ViewOrigin.xyz);
	vec3 vDir = normalize(pMap.xyz - u_ViewOrigin.xyz);
	float atten = 1.0 - clamp(pow(distance(vDir, lDir) / 5.0, 0.3), 0.0, 1.0);
	totalColor.rgb *= atten;
	//totalColor.rgb = vec3(atten);

	//float sun = 1.0 - clamp(pow(distance(vDir, lDir) / 5.0, u_Local1.b), 0.0, 1.0) * u_Local1.a;
	//totalColor.rgb += totalColor * sun;

	if (u_Local1.r > 0.0)
	{// Sunset, Sunrise, and Night times... Scale down screen color, before adding lighting...
		vec3 nightColor = vec3(0.0);
		totalColor.rgb = mix(totalColor.rgb, nightColor, u_Local1.r);
	}

	gl_FragColor = vec4(totalColor, 1.0);
}

#endif //__USING_SHADOW_MAP__


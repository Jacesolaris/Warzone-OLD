uniform sampler2D			u_ScreenDepthMap;
//uniform sampler2D			u_GlowMap;				// noise 1
//uniform sampler2D			u_SpecularMap;			// noise 2

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
uniform vec4				u_ViewInfo;				// zfar / znear, zfar, depthBits, znear
//uniform float				u_ShadowZfar[5];

precise varying vec2		var_DepthTex;
precise varying vec3		var_ViewDir;

#define						r_shadowBlurWidth		u_Settings0.r
#define						r_shadowMapSize			u_Settings0.g

precise float DEPTH_MAX_ERROR = (1.0 / pow(2.0, u_ViewInfo.b));

float linearizeDepth(float depth)
{
	return 1.0 / mix(u_ViewInfo.x, 1.0, depth);
}

float getLinearDepth(sampler2D depthMap, vec2 tex)
{
	return linearizeDepth(texture(depthMap, tex).r - DEPTH_MAX_ERROR);
}

float random( const vec2 p )
{
  // We need irrationals for pseudo randomness.
  // Most (all?) known transcendental numbers will (generally) work.
  const vec2 r = vec2(
    23.1406926327792690,  // e^pi (Gelfond's constant)
     2.6651441426902251); // 2^sqrt(2) (Gelfond-Schneider constant)
  //return fract( cos( mod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );
  return mod( 123456789., 1e-7 + 256. * dot(p,r) );  
}

float offset_lookup(sampler2DShadow shadowmap, vec4 loc, vec2 offset, float scale)
{
	float result = textureProj(shadowmap, vec4(loc.xy + offset * scale * loc.w, loc.z, loc.w));
	//result = clamp((result - 0.2) * 1.2, 0.0, 1.0);
	//result = clamp(result - DEPTH_MAX_ERROR, 0.0, 1.0);
	return result;
}

float PCF(const sampler2DShadow shadowmap, const vec4 st, const float dist, float scale)
{
	float mult;

#if 1
	vec4 sCoord = vec4(st);

#if 1
	//vec2 offset = vec2(greaterThan(fract(st.xy * 0.5), vec2(0.25)));  // mod
	vec2 offset = mod(sCoord.xy, 0.5);
	offset.y += offset.x;  // y ^= x in floating point
	if (offset.y > 1.1) offset.y = 0;

	float shadowCoeff = (offset_lookup(shadowmap, sCoord, offset + vec2(-1.5, 0.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(0.5, 0.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(-1.5, -1.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(0.5, -1.5), scale) ) 
			   * 0.25;
#else
	float shadowCoeff = 0.0;
	float weight = 0.0;

	for (float x = -1.0; x <= 1.0; x += 0.25)
	{
		for (float y = -1.0; y <= 1.0; y += 0.25)
		{
			float w = 1.0 - (distance(vec2(x, y), vec2(0.0)) / 4.0);
			shadowCoeff += offset_lookup(shadowmap, sCoord, vec2(x, y), scale) * w;
			weight += w;
		}
	}
	shadowCoeff /= weight;
#endif
	return shadowCoeff;
#elif 0
	// from http://http.developer.nvidia.com/GPUGems/gpugems_ch11.html
	vec2 offset = vec2(greaterThan(fract(var_DepthTex.xy * r_FBufScale * 0.5), vec2(0.25)));
	offset.y += offset.x;
	if (offset.y > 1.1) offset.y = 0.0;
	
	mult = texture(shadowmap, vec3(st.xy + (offset + vec2(-1.5,  0.5)) * scale, dist))
	     + texture(shadowmap, vec3(st.xy + (offset + vec2( 0.5,  0.5)) * scale, dist))
	     + texture(shadowmap, vec3(st.xy + (offset + vec2(-1.5, -1.5)) * scale, dist))
	     + texture(shadowmap, vec3(st.xy + (offset + vec2( 0.5, -1.5)) * scale, dist));
	 
	mult *= 0.25;
	return mult;
#elif 0
	#define USE_SHADOW_FILTER
	#define USE_SHADOW_FILTER2

	#if defined(USE_SHADOW_FILTER)
		float r = random(var_DepthTex.xy);
		float sinr = sin(r) * scale;
		float cosr = cos(r) * scale;
		mat2 rmat = mat2(cosr, sinr, -sinr, cosr);

		mult =  texture(shadowmap, vec3(st.xy + rmat * vec2(-0.7055767, 0.196515), dist));
		mult += texture(shadowmap, vec3(st.xy + rmat * vec2(0.3524343, -0.7791386), dist));
		mult += texture(shadowmap, vec3(st.xy + rmat * vec2(0.2391056, 0.9189604), dist));
		#if defined(USE_SHADOW_FILTER2)
			mult += texture(shadowmap, vec3(st.xy + rmat * vec2(-0.07580382, -0.09224417), dist));
			mult += texture(shadowmap, vec3(st.xy + rmat * vec2(0.5784913, -0.002528916), dist));
			mult += texture(shadowmap, vec3(st.xy + rmat * vec2(0.192888, 0.4064181), dist));
			mult += texture(shadowmap, vec3(st.xy + rmat * vec2(-0.6335801, -0.5247476), dist));
			mult += texture(shadowmap, vec3(st.xy + rmat * vec2(-0.5579782, 0.7491854), dist));
			mult += texture(shadowmap, vec3(st.xy + rmat * vec2(0.7320465, 0.6317794), dist));

			mult *= 0.11111;
		#else
			mult *= 0.33333;
		#endif
	#else
		mult = texture(shadowmap, vec3(st.xy, dist));
	#endif
	return mult;
#endif
}

//////////////////////////////////////////////////////////////////////////
void main()
{
	precise float result = 1.0;
	//precise float depth = getLinearDepth(u_ScreenDepthMap, var_DepthTex);
	precise float depth = texture(u_ScreenDepthMap, var_DepthTex).x;

	precise vec4 biasPos = vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
	precise vec4 shadowpos = u_ShadowMvp * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCF(u_ShadowMap, shadowpos, shadowpos.z, 1.0 / r_shadowMapSize);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp2 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCF(u_ShadowMap2, shadowpos, shadowpos.z, 1.0 / r_shadowMapSize);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp3 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCF(u_ShadowMap3, shadowpos, shadowpos.z, 1.0 / (r_shadowMapSize * 2.0));
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp4 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCF(u_ShadowMap4, shadowpos, shadowpos.z, 1.0 / (r_shadowMapSize * 4.0));
		//result = clamp(pow(result, 64.0), 0.0, 1.0);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}
	
	gl_FragColor = vec4(result, depth, 0.0, 1.0);
}

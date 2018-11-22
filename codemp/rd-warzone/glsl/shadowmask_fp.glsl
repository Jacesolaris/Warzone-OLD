uniform sampler2D			u_ScreenDepthMap;

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
	float shadowCoeff = (offset_lookup(shadowmap, sCoord, offset + vec2(-1.5, 0.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(0.5, 0.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(-1.5, -1.5), scale) +
               offset_lookup(shadowmap, sCoord, offset + vec2(0.5, -1.5), scale) ) 
			   * 0.25;

	return shadowCoeff;
}

//////////////////////////////////////////////////////////////////////////
void main()
{
	float result = 1.0;
	float depth = texture(u_ScreenDepthMap, var_DepthTex).x;

	vec4 biasPos = vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
	vec4 shadowpos = u_ShadowMvp * biasPos;

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
		result = PCF(u_ShadowMap2, shadowpos, shadowpos.z, 1.0 / (r_shadowMapSize * 4.0));
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp3 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCF(u_ShadowMap3, shadowpos, shadowpos.z, 1.0 / (r_shadowMapSize * 6.0));
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}
	
	gl_FragColor = vec4(result, depth, 0.0, 1.0);
}

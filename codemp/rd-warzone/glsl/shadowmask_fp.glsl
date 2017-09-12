uniform sampler2D		u_ScreenDepthMap;
uniform sampler2D		u_GlowMap;				// noise 1
uniform sampler2D		u_SpecularMap;			// noise 2

uniform sampler2D		u_ShadowMap;
uniform sampler2D		u_ShadowMap2;
uniform sampler2D		u_ShadowMap3;
uniform sampler2D		u_ShadowMap4;

uniform mat4			u_ShadowMvp;
uniform mat4			u_ShadowMvp2;
uniform mat4			u_ShadowMvp3;
uniform mat4			u_ShadowMvp4;

uniform vec4			u_Settings0;			// r_shadowSamples (numBlockerSearchSamples), r_shadowMapSize, r_testshaderValue1->value, r_testshaderValue2->value
uniform vec3			u_ViewOrigin;
uniform vec4			u_ViewInfo;				// zfar / znear, zfar, depthBits, znear
uniform float			u_ShadowZfar[5];

#define					r_shadowBlurQuality			u_Settings0.r
#define					r_shadowMapSize			u_Settings0.g

precise varying vec2	var_DepthTex;
precise varying vec3	var_ViewDir;

precise float DEPTH_MAX_ERROR = (1.0 / pow(2.0, u_ViewInfo.b));

precise float scale = 1.0 / r_shadowMapSize;


#define NEAR							u_ViewInfo.a//0.1

//uniform float directionalLightShadowMapBias;
#define directionalLightShadowMapBias	DEPTH_MAX_ERROR

//uniform vec3 eyePosition;
#define eyePosition						u_ViewOrigin

//uniform int numBlockerSearchSamples = 1;
//uniform int numPCFSamples = 1;

#define numBlockerSearchSamples			int(r_shadowBlurQuality)
#define numPCFSamples					int(r_shadowBlurQuality)

float linearizeDepth(float depth)
{
	return 1.0 / mix(u_ViewInfo.x, 1.0, depth);
}

float getLinearDepth(sampler2D depthMap, vec2 tex)
{
	precise float sampleZDivW = texture(depthMap, tex).r;
	//sampleZDivW -= DEPTH_MAX_ERROR;
	return linearizeDepth(sampleZDivW);
}

//////////////////////////////////////////////////////////////////////////
vec2 RandomDirection(sampler2D distribution, float u)
{
   return texture(distribution, vec2(u)).xy * 2 - vec2(1);
}

//////////////////////////////////////////////////////////////////////////
// this search area estimation comes from the following article: 
// http://developer.download.nvidia.com/whitepapers/2008/PCSS_DirectionalLight_Integration.pdf
float SearchWidth(float uvLightSize, float receiverDistance, vec3 shadowCoords, float sampleZ)
{
	//return uvLightSize * (receiverDistance - NEAR) / eyePosition.z;
	//return uvLightSize * (receiverDistance - NEAR) / distance(eyePosition, shadowCoords);
	return uvLightSize * (receiverDistance - NEAR) / sampleZ;
}

//////////////////////////////////////////////////////////////////////////
float FindBlockerDistance_DirectionalLight(vec3 shadowCoords, sampler2D shadowMap, float uvLightSize, float sampleZ)
{
	int blockers = 0;
	float avgBlockerDistance = 0;
	float searchWidth = SearchWidth(uvLightSize, shadowCoords.z, shadowCoords, sampleZ);

	for (int i = 0; i < numBlockerSearchSamples; i++)
	{
		float z = texture(shadowMap, shadowCoords.xy + RandomDirection(u_GlowMap, i / float(numBlockerSearchSamples)) * searchWidth).r;
		if (z < (shadowCoords.z - directionalLightShadowMapBias))
		{
			blockers++;
			avgBlockerDistance += z;
		}
	}

	if (blockers > 0)
		return 1.0 - (avgBlockerDistance / blockers);
	else
		return -1;
}

//////////////////////////////////////////////////////////////////////////
float PCF_DirectionalLight(vec3 shadowCoords, sampler2D shadowMap, float uvRadius)
{
	float sum = 0;
	for (int i = 0; i < numPCFSamples; i++)
	{
		float z = texture(shadowMap, shadowCoords.xy + RandomDirection(u_SpecularMap, i / float(numPCFSamples)) * uvRadius).r;
		sum += (z < (shadowCoords.z - directionalLightShadowMapBias)) ? 1 : 0;
	}
	return 1.0 - (sum / numPCFSamples);
}

//////////////////////////////////////////////////////////////////////////
float PCSS_DirectionalLight(vec3 shadowCoords, sampler2D shadowMap, float uvLightSize, float sampleZ)
{
	// blocker search
	float blockerDistance = FindBlockerDistance_DirectionalLight(shadowCoords, shadowMap, uvLightSize, sampleZ);
	if (blockerDistance == -1)
		return 1;		

	// penumbra estimation
	float penumbraWidth = (shadowCoords.z - blockerDistance) / blockerDistance;

	// percentage-close filtering
	float uvRadius = penumbraWidth * uvLightSize * NEAR / shadowCoords.z;
	return PCF_DirectionalLight(shadowCoords, shadowMap, uvRadius);
}

//////////////////////////////////////////////////////////////////////////
void main()
{
	precise float result = 1.0;
	precise float depth = getLinearDepth(u_ScreenDepthMap, var_DepthTex);
	precise float sampleZ = (u_ViewInfo.y - NEAR) * depth;

	precise vec4 biasPos = vec4(u_ViewOrigin + var_ViewDir * (depth - 0.5 / u_ViewInfo.x), 1.0);
	precise vec4 shadowpos = u_ShadowMvp * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCSS_DirectionalLight(shadowpos.xyz, u_ShadowMap, scale * 0.001/*lightSource.size / frustumSize*/, sampleZ);
		//result = PCF_DirectionalLight(shadowpos.xyz, u_ShadowMap, scale);
		//result = FindBlockerDistance_DirectionalLight(shadowpos.xyz, u_ShadowMap, scale * 0.001);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp2 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCSS_DirectionalLight(shadowpos.xyz, u_ShadowMap2, scale * 0.001/*lightSource.size / frustumSize*/, sampleZ);
		//result = PCF_DirectionalLight(shadowpos.xyz, u_ShadowMap2, scale);
		//result = FindBlockerDistance_DirectionalLight(shadowpos.xyz, u_ShadowMap2, scale * 0.001);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp3 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCSS_DirectionalLight(shadowpos.xyz, u_ShadowMap3, scale * 0.001/*lightSource.size / frustumSize*/, sampleZ);
		//result = PCF_DirectionalLight(shadowpos.xyz, u_ShadowMap3, scale);
		//result = FindBlockerDistance_DirectionalLight(shadowpos.xyz, u_ShadowMap3, scale * 0.001);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}

	shadowpos = u_ShadowMvp4 * biasPos;

	if (all(lessThanEqual(abs(shadowpos.xyz), vec3(abs(shadowpos.w)))))
	{
		shadowpos.xyz = shadowpos.xyz / shadowpos.w * 0.5 + 0.5;
		result = PCSS_DirectionalLight(shadowpos.xyz, u_ShadowMap4, scale * 0.001/*lightSource.size / frustumSize*/, sampleZ);
		//result = PCF_DirectionalLight(shadowpos.xyz, u_ShadowMap4, scale);
		//result = FindBlockerDistance_DirectionalLight(shadowpos.xyz, u_ShadowMap4, scale * 0.001);
		gl_FragColor = vec4(result, depth, 0.0, 1.0);
		return;
	}
	
	gl_FragColor = vec4(result, depth, 0.0, 1.0);
}

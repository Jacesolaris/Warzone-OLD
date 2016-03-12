uniform sampler2D	u_DiffuseMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif

uniform vec4		u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4		u_Local6; // useSunLightSpecular
uniform vec4		u_Local10; // foliageLODdistance, foliageDensity, doSway, overlaySway

#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;
uniform vec3		u_PrimaryLightAmbient;
uniform float		u_PrimaryLightRadius;
uniform vec4		u_ViewOrigin;

uniform int			u_lightCount;
uniform vec3		u_lightPositions2[16];
uniform float		u_lightDistances[16];
uniform vec3		u_lightColors[16];

smooth in vec3		vWorldPos;

#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 

smooth in vec2		vTexCoord;

#define m_TexCoords vTexCoord


out vec4			out_Glow;
out vec4			out_DetailedNormal;
out vec4			out_FoliageMap;


void main() 
{
	vec2 texCoords = m_TexCoords + vec2(u_Local10.b * u_Local10.a * ((1.0 - m_TexCoords.y) + 1.0), 0.0);
	vec4 diffuse = texture(u_DiffuseMap, texCoords);
	if (diffuse.a > 0.5) diffuse.a = 1.0;
	gl_FragColor = diffuse;
	
	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
		vec3 m_Normal = (vec3(clamp(length(1.0-diffuse.rgb) * 0.55, 0.0, 1.0)) * 2.0 - 1.0);
		vec4 PrimaryLightDir = vec4(u_PrimaryLightOrigin.xyz - (vWorldPos * u_PrimaryLightOrigin.w), u_PrimaryLightRadius * u_PrimaryLightRadius);
	#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 

	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		if (u_Local6.r > 0.0)
		{
			float lambertian2 = dot(PrimaryLightDir.xyz, m_Normal);
			vec3 E = normalize(-(u_ViewOrigin.xyz - vWorldPos.xyz));

			if(lambertian2 > 0.0)
			{// this is blinn phong
				vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
				float specAngle = max(dot(halfDir2, m_Normal), 0.0);
				float spec2 = pow(specAngle, 16.0);
				gl_FragColor.rgb -= vec3(spec2 * (1.0 - length(diffuse.rgb) / 3.0)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * 0.2 * u_Local5.b;
			}

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightDir = u_lightPositions2[li] - vWorldPos.xyz;
				float lambertian3 = dot(lightDir.xyz, m_Normal);
				float spec3 = 0.0;

				if(lambertian3 > 0.0)
				{
					float lightStrength = clamp(1.0 - (length(lightDir) * (1.0 / u_lightDistances[li])), 0.0, 1.0) * 0.5;

					if(lightStrength > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, m_Normal), 0.0);
						spec3 = pow(specAngle3, 16.0);
						gl_FragColor.rgb += vec3(spec3 * (1.0 - length(diffuse.rgb) / 3.0)) * u_lightColors[li].rgb * lightStrength * u_Local5.b;
					}
				}
			}
		}
	#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)


	#if defined(USE_SHADOWMAP) 
		vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
		float shadowValue = texture2D(u_ShadowMap, shadowTex).r;

		// surfaces not facing the light are always shadowed
		shadowValue *= float(dot(m_Normal.xyz, PrimaryLightDir.xyz) > 0.0);

		gl_FragColor.rgb *= clamp(shadowValue + 0.5, 0.0, 1.0);
	#endif //defined(USE_SHADOWMAP) 


	out_Glow = vec4(0.0);

	out_DetailedNormal = vec4(m_Normal.xyz, 0.1);

	out_FoliageMap.r = 1.0;
	out_FoliageMap.g = 1.0;
}

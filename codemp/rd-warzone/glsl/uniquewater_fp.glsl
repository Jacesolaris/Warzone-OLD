#define ORIGINAL_WATER

uniform sampler2D u_DiffuseMap;
//uniform sampler2D u_OverlayMap; // sky diffuse

varying vec4		var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec4		u_Local2; // ExtinctionCoefficient
varying vec4		u_Local3; // RimScalar, MaterialThickness, subSpecPower
uniform vec4		u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4		u_Local6; // useSunLightSpecular
uniform vec4		u_Local9; // 

varying vec2		var_Dimensions;

varying float  var_Time;

uniform sampler2D u_NormalMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

#if defined(USE_CUBEMAP)
#define textureCubeLod textureLod // UQ1: > ver 140 support
uniform samplerCube u_CubeMap;
#endif

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

#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 


varying vec2		var_TexCoords;
varying vec4		var_Color;
varying vec3		var_ViewDir;
varying vec3		var_Normal;
varying vec3		var_N;
varying float		var_Fresnel;
varying vec3		var_vertPos;

out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;



#if defined(ORIGINAL_WATER)




mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

void main()
{
	vec3 viewDir, lightColor, ambientColor;
	vec4 specular = vec4(1.0);
	vec3 N, E;
	vec2 tex_offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

	mat3 tangentToWorld = cotangent_frame(var_Normal.xyz, -var_ViewDir, var_TexCoords.xy);
	viewDir = var_ViewDir;

	E = normalize(viewDir);

	lightColor	= var_Color.rgb;

	vec2 texCoords = var_TexCoords.xy;
	

	//
	// Water
	//
	
	vec2 cPos = -1.0 + 2.0 * texCoords.xy;
	float cLength = length(cPos);
	
	vec2 uvOff = (cPos/cLength)*cos(cLength*12.0-var_Time*4.0)*0.03;

	vec2 uv = texCoords.xy+uvOff;
	vec4 diffuse = texture2D(u_DiffuseMap, uv);
	vec4 orig_diffuse = diffuse;
	gl_FragColor = vec4(diffuse.rgb,1.0);
	


	/*
	float shininess = 0.5;
	vec3 mirrorEye = vec3(2.0) * dot(normalize(L), var_Normal.xyz) * var_Normal.xyz - normalize(L);
	float dotSpec = clamp(dot(mirrorEye.xyz, -L), 0.0, 1.0) * 0.5 + 0.5;
	specular.rgb = vec3(1.0 - var_Fresnel) * clamp(-L.y, 0.0, 1.0) * (pow(dotSpec, 512.0) * (shininess * 1.8 + 0.2));
	specular.rgb += specular.rgb * 25.0 * clamp(shininess - 0.05, 0.0, 1.0);
	gl_FragColor.rgb += specular.rgb;
	*/

	//
	//
	//


	vec3 norm1 = -vec3(clamp((vec3(uvOff, 0.0) * 1.0/*u_Local9.b*/), 0.0, 1.0) * 2.0 - 1.0);
//	vec3 m_Normal = tangentToWorld * norm1;

	vec4 norm = texture2D(u_NormalMap, uv);
	vec3 m_Normal = norm.xyz * 2.0 - 1.0;
	m_Normal.z = sqrt(clamp((0.25 - m_Normal.x * m_Normal.x) - m_Normal.y * m_Normal.y, 0.0, 1.0));
	m_Normal = tangentToWorld * ((m_Normal + norm1) / 2.0);
	m_Normal = normalize(m_Normal);

	/*if (u_Local9.a > 0.0)
	{
		gl_FragColor = vec4(m_Normal.xyz * 0.5 + 0.5, 1.0);
		out_Glow = vec4(0.0);
		out_DetailedNormal = vec4(m_Normal.xyz, 0.75);
		out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
		return;
	}*/

	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		if (u_Local6.r > 0.0)
		{
			vec3 E = normalize(-(u_ViewOrigin.xyz - var_vertPos.xyz));
			vec3 PrimaryLightDir = u_PrimaryLightOrigin.xyz - var_vertPos.xyz;
			float lambertian2 = dot(PrimaryLightDir.xyz, m_Normal);

			if(lambertian2 > 0.0)
			{// this is blinn phong
				vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
				float specAngle = max(dot(halfDir2, m_Normal), 0.0);
				float spec2 = pow(specAngle, 16.0);
				gl_FragColor.rgb += vec3(spec2 * (1.0 - length(gl_FragColor.rgb) / 3.0)) * u_PrimaryLightColor.rgb * 0.2/*u_Local9.g*/ * u_Local5.b;
			}

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightDir = u_lightPositions2[li] - var_vertPos.xyz;
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
						gl_FragColor.rgb += vec3(spec3 * (1.0 - length(gl_FragColor.rgb) / 3.0)) * u_lightColors[li].rgb * lightStrength * u_Local5.b;
					}
				}
			}
		}
	#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)


	gl_FragColor.a = 0.7;//1.0;
	out_Glow = vec4(0.0);


	out_DetailedNormal = vec4(m_Normal.xyz, 0.75);

	out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
}





#else //!defined(ORIGINAL_WATER)




uniform float	u_Time;


//uniform vec3 playerPos;
//uniform vec3 sunPosition;
//uniform float elapsedTime;
//uniform sampler3D noiseTexture;

uniform sampler2D u_SpecularMap;

#define noiseTexture u_SpecularMap

#define sunPosition u_PrimaryLightOrigin.xyz
#define playerPos u_ViewOrigin.xyz
#define elapsedTime u_Time

// Vertex shader input
in vec3 fWorldPos;
in vec3 fNormal;
in float fIntensity;

float tween(float t)
{
	return clamp(t*t*t*(t*(t*6-15)+10),0,1);
}

void main()
{
	vec3 normal = normalize(fNormal);
	vec3 up = normalize(fWorldPos);
	vec3 samplePos = fWorldPos;
	float ss = 0.00005;//0.195;
	float ns = 1.0;
	float timescale = 0.5;
	float s = 2;

	/*
	vec3 samp_yz = texture3D(noiseTexture, vec3(samplePos.yz * ss, elapsedTime*timescale)).rgb - vec3(0.5);
	vec3 samp_xz = texture3D(noiseTexture, vec3(samplePos.xz * ss, elapsedTime*timescale)).rgb - vec3(0.5);
	vec3 samp_xy = texture3D(noiseTexture, vec3(samplePos.xy * ss, elapsedTime*timescale)).rgb - vec3(0.5);
	*/
	vec3 samp_yz = texture2D(noiseTexture, vec2(samplePos.yz * ss * elapsedTime*timescale)).rgb - vec3(0.5);
	vec3 samp_xz = texture2D(noiseTexture, vec2(samplePos.xz * ss * elapsedTime*timescale)).rgb - vec3(0.5);
	vec3 samp_xy = texture2D(noiseTexture, vec2(samplePos.xy * ss * elapsedTime*timescale)).rgb - vec3(0.5);
	
	vec3 norm_xy = samp_xy.r * vec3(1, 0, 0) + samp_xy.g * vec3(0, 1, 0) + samp_xy.b * vec3(0, 0, 1);
	vec3 norm_xz = samp_xz.r * vec3(0, 0, 1) + samp_xz.g * vec3(1, 0, 0) + samp_xz.b * vec3(0, 1, 0);
	vec3 norm_yz = samp_yz.r * vec3(0, 1, 0) + samp_yz.g * vec3(0, 0, 1) + samp_yz.b * vec3(1, 0, 0);	
	
	vec3 modNorm = norm_yz*normal.x + norm_xz*normal.y + norm_xy*normal.z;

	normal = normalize(mix(normal, modNorm, ns));
	
	//normal = normalize(normal);
	
	int c1 = 3;
	int c2 = 5;
	
	vec3 FtoP = normalize(playerPos - fWorldPos);
	vec3 FtoS = normalize(sunPosition - fWorldPos);
	vec3 sum = normalize(FtoP + FtoS);
	
	float alpha = 0.5;
	
	float closeToNorm = clamp(dot(FtoP, normal)*1.5,0,1);
	vec3 hclr = mix(vec3(0.4, 0.7, 1.0), vec3(0.1, 0.1, 0.4), closeToNorm);
	alpha = mix(0.5, 0.3, closeToNorm);
	
	
	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		if (u_Local6.r > 0.0)
		{
			vec3 E = normalize(-(u_ViewOrigin.xyz - fWorldPos.xyz));

			vec3 PrimaryLightDir = u_PrimaryLightOrigin.xyz - fWorldPos.xyz;
			float lambertian2 = dot(PrimaryLightDir.xyz, normal);

			if(lambertian2 > 0.0)
			{// this is blinn phong
				vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
				float specAngle = max(dot(halfDir2, normal), 0.0);
				float spec2 = pow(specAngle, 16.0);
				hclr.rgb += vec3(spec2 * (1.0 - length(hclr.rgb) / 3.0)) * u_PrimaryLightColor.rgb * hclr * u_Local9.g * u_Local5.b;
			}

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightDir = u_lightPositions2[li] - fWorldPos.xyz;
				float lambertian3 = dot(lightDir.xyz, normal);
				float spec3 = 0.0;

				if(lambertian3 > 0.0)
				{
					float lightStrength = clamp(1.0 - (length(lightDir) * (1.0 / u_lightDistances[li])), 0.0, 1.0) * 0.5;

					if(lightStrength > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, normal), 0.0);
						spec3 = pow(specAngle3, 16.0);
						hclr.rgb += vec3(spec3 * (1.0 - length(hclr.rgb) / 3.0)) * u_lightColors[li].rgb * hclr * lightStrength * u_Local5.b;
					}
				}
			}
		}
	#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
	
	gl_FragColor = vec4( hclr, alpha);

	//gl_FragColor.a = 0.5;//1.0;
	out_Glow = vec4(0.0);


	out_DetailedNormal = vec4(normal.xyz, 0.75);

	out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
}



#endif //defined(ORIGINAL_WATER)
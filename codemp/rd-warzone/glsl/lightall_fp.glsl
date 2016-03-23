uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SteepMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4	u_Local6; // useSunLightSpecular, 0, 0, 0
uniform vec4	u_Local9;

#define USE_TRI_PLANAR
//#define USE_SUBSURFACE_SCATTER

varying float  var_Time;

uniform sampler2D u_LightMap;

uniform sampler2D u_NormalMap;
uniform sampler2D u_NormalMap2;

uniform sampler2D u_DeluxeMap;

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

#if defined(USE_CUBEMAP)
#define textureCubeLod textureLod // UQ1: > ver 140 support
uniform samplerCube u_CubeMap;
#endif

#if defined(USE_SUBSURFACE_SCATTER)
uniform sampler2D u_SubsurfaceMap;
#endif

uniform sampler2D u_OverlayMap;

uniform vec4      u_EnableTextures;

uniform vec3  u_PrimaryLightColor;
uniform vec3  u_PrimaryLightAmbient;

uniform vec4      u_NormalScale;
uniform vec4      u_SpecularScale;

uniform vec4      u_CubeMapInfo;
uniform float     u_CubeMapStrength;


varying vec2      var_TexCoords;
varying vec2	  var_TexCoords2;

varying vec4      var_Color;

varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
#define var_Normal2 var_Normal.w

varying vec3 var_N;

varying vec4      var_PrimaryLightDir;

varying vec3   var_vertPos;

varying vec3      var_ViewDir;
varying vec2   var_nonTCtexCoords; // for steep maps


uniform int			u_lightCount;
uniform vec3		u_lightPositions2[16];
uniform float		u_lightDistances[16];
uniform vec3		u_lightColors[16];


#if defined(USE_TESSELLATION)

in vec2					TexCoord_FS_in;
in vec4					Normal_FS_in;
in vec4					WorldPos_FS_in;
in vec3					ViewDir_FS_in;

#define m_Normal		Normal_FS_in
#define m_TexCoords		TexCoord_FS_in
#define m_vertPos		WorldPos_FS_in.xyz
#define m_ViewDir		var_ViewDir

#else //!defined(USE_TESSELLATION)

#define m_Normal		var_Normal
#define m_TexCoords		var_TexCoords
#define m_vertPos		var_vertPos
#define m_ViewDir		var_ViewDir

#endif //defined(USE_TESSELLATION)


varying vec3	var_Blending;
varying float	var_Slope;
varying float	var_usingSteepMap;


out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_PositionMap;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;


vec4 GetSteepMap(vec2 texCoords, vec2 ParallaxOffset)
{
#if defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
	if (u_Local5.a > 0.0 && var_Slope > 0)
	{// Steep maps...
		const float scale = 0.0025;
		vec4 xaxis = texture2D( u_SteepMap, (m_vertPos.yz + ParallaxOffset.xy) * scale);
		vec4 yaxis = texture2D( u_SteepMap, (m_vertPos.xz + ParallaxOffset.xy) * scale);
		vec4 zaxis = texture2D( u_SteepMap, (m_vertPos.xy + ParallaxOffset.xy) * scale);
		vec4 tex = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
		return tex;
	}
	else
	{
		return texture2D(u_DiffuseMap, texCoords);
	}
#else //!defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
	return texture2D(u_DiffuseMap, texCoords);
#endif //defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
}

vec4 GetDiffuse(vec2 texCoords, vec2 ParallaxOffset)
{
#if defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
	if (u_Local5.a > 0.0)
	{// Steep maps...
		const float scale = 0.01;
		vec4 xaxis = texture2D( u_DiffuseMap, (m_vertPos.yz + ParallaxOffset.xy) * scale);
		vec4 yaxis = texture2D( u_DiffuseMap, (m_vertPos.xz + ParallaxOffset.xy) * scale);
		vec4 zaxis = texture2D( u_DiffuseMap, (m_vertPos.xy + ParallaxOffset.xy) * scale);
		vec4 tex = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
		return tex;
	}
	else
	{
		return texture2D(u_DiffuseMap, texCoords);
	}
#else //!defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
	return texture2D(u_DiffuseMap, texCoords);
#endif //defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
}

vec4 GetNormal(vec2 texCoords, vec2 ParallaxOffset)
{
#if defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
	if (u_Local5.a > 0.0 && var_Slope > 0)
	{// Steep maps...
		const float scale = 0.0025;
		vec4 xaxis = texture2D( u_NormalMap2, (m_vertPos.yz + ParallaxOffset.xy) * scale);
		vec4 yaxis = texture2D( u_NormalMap2, (m_vertPos.xz + ParallaxOffset.xy) * scale);
		vec4 zaxis = texture2D( u_NormalMap2, (m_vertPos.xy + ParallaxOffset.xy) * scale);
		vec4 tex = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
		return tex;
	}
	else if (u_Local5.a > 0.0)
	{// Steep maps...
		const float scale = 0.01;
		vec4 xaxis = texture2D( u_NormalMap, (m_vertPos.yz + ParallaxOffset.xy) * scale);
		vec4 yaxis = texture2D( u_NormalMap, (m_vertPos.xz + ParallaxOffset.xy) * scale);
		vec4 zaxis = texture2D( u_NormalMap, (m_vertPos.xy + ParallaxOffset.xy) * scale);
		vec4 tex = xaxis * var_Blending.x + yaxis * var_Blending.y + zaxis * var_Blending.z;
		return tex;
	}
	else
	{
		return texture2D(u_NormalMap, texCoords);
	}
#else //!defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
	return texture2D(u_NormalMap, texCoords);
#endif //defined(USE_OVERLAY) || defined(USE_TRI_PLANAR)
}

float GetDepth(vec2 t)
{
	return 1.0 - GetNormal(t, vec2(0.0)).a;
}

#if defined(USE_PARALLAXMAP)

float RayIntersectDisplaceMap(vec2 dp)
{
	if (u_Local1.x == 0.0)
		return 0.0;
	
	float depth = GetDepth(dp) - 1.0;
	return depth * u_Local1.x;
}
#endif

vec3 EnvironmentBRDF(float gloss, float NE, vec3 specular)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96,0.25 ) * gloss;
	t += vec4( 0.0, 0.0, (0.015 - 0.75 * 0.04)/0.96,0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28 * NE ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + specular * ( a1 - a0 ), 0.0, 1.0 );
}



float blinnPhongSpecular(in vec3 normalVec, in vec3 lightVec, in float specPower)
{
    vec3 halfAngle = normalize(normalVec + lightVec);
    return pow(clamp(dot(normalVec,halfAngle),0.0,1.0),specPower);
}

 
#ifdef USE_SUBSURFACE_SCATTER
float halfLambert(vec3 vect1, vec3 vect2)
{
	float product = dot(vect1,vect2);
	return product * 0.5 + 0.5;
}

// Main fake sub-surface scatter lighting function

vec3 ExtinctionCoefficient = u_Local2.xyz;
float RimScalar = u_Local3.x;
float MaterialThickness = u_Local3.y;
float SpecPower = u_Local3.z;

vec4 subScatterFS(vec4 BaseColor, vec4 SpecColor, vec3 lightVec, vec3 LightColor, vec3 eyeVec, vec3 worldNormal, vec2 texCoords)
{
	vec4 subsurface = vec4(ExtinctionCoefficient, MaterialThickness);

	if (u_Local4.z != 0.0)
	{// We have a subsurface image, use it instead...
		subsurface = texture2D(u_SubsurfaceMap, texCoords.xy);
	}
	else if (length(ExtinctionCoefficient) == 0.0)
	{// Default if not specified...
		subsurface.rgb = vec3(BaseColor.rgb);
	}

	if (MaterialThickness == 0.0)
	{// Default if not specified...
		MaterialThickness = 0.01;
	}
	
	if (subsurface.a == 0.0 && MaterialThickness != 0.0)
	{// Backup in case image is missing alpha channel...
		subsurface.a = MaterialThickness;
	}

	subsurface.a = 1.0 - subsurface.a;

	if (RimScalar == 0.0)
	{// Default if not specified...
		RimScalar = 0.5;
	}

	if (SpecPower == 0.0)
	{// Default if not specified...
		SpecPower = 0.3;
	}

	float attenuation = 1.0;//10.0 * (1.0 / distance(lightVec.xyz,m_vertPos.xyz));
    vec3 eVec = eyeVec;
    vec3 lVec = lightVec;
    vec3 wNorm = worldNormal;
     
    //vec4 dotLN = vec4(halfLambert(lVec,wNorm) * attenuation);
	vec3 halfDir2 = normalize(lVec + eVec);
	float specAngle = max(dot(halfDir2, wNorm), 0.0);
	vec4 dotLN = vec4(specAngle * attenuation);

    dotLN *= BaseColor;


	vec3 halfDir3 = normalize(lVec + -eVec);
	float specAngle2 = max(dot(halfDir3, -wNorm), 0.0);
     
    vec3 indirectLightComponent = vec3(subsurface.a * max(vec3(0.0),length(halfDir3)/3.0/*dot(-wNorm,lVec)*/));
    indirectLightComponent += subsurface.a * specAngle2;//halfLambert(-eVec,lVec);
    indirectLightComponent *= attenuation;
    indirectLightComponent.rgb *= subsurface.rgb;
     
    vec3 rim = vec3(1.0 - max(0.0,dot(wNorm,eVec)));
    rim *= rim;
    rim *= max(0.0,dot(wNorm,lVec)) * SpecColor.rgb;
     
    vec4 finalCol = dotLN + vec4(indirectLightComponent,1.0);
    finalCol.rgb += (rim * RimScalar * attenuation * finalCol.a);
    //finalCol.rgb += vec3(blinnPhongSpecular(wNorm,lVec,SpecPower) * attenuation * SpecColor * finalCol.a * 0.05);
    finalCol.rgb *= LightColor.rgb;
     
    return finalCol;   
}
#endif //USE_SUBSURFACE_SCATTER


void main()
{
#if 0
#if defined(USE_TESSELLATION)
	gl_FragColor = texture2D(u_DiffuseMap, m_TexCoords);

	#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
	#else
		out_Glow = vec4(0.0);
	#endif

	out_DetailedNormal = vec4(m_Normal.xyz, 0.2);

	if (u_Local1.a == 20 || u_Local1.a == 19 /*|| u_Local1.a == 5 || u_Local1.a == 6*/) 
	{// (Foliage/Plants), (billboard trees), ShortGrass, LongGrass
		out_FoliageMap.r = 1.0;
		out_FoliageMap.g = 1.0;
	}
	else
	{
		out_FoliageMap.r = 1.0;
		out_FoliageMap.g = 0.0;
	}
	return;
#endif //defined(USE_TESSELLATION)
#endif

	vec3 viewDir = vec3(0.0), lightColor = vec3(0.0), ambientColor = vec3(0.0);
	vec4 specular = vec4(0.0);
	vec3 N, E, H;
	vec3 DETAILED_NORMAL = vec3(1.0);
	float NL, NH, NE, EH, attenuation;
	vec2 tex_offset = vec2(1.0 / u_Dimensions);

	vec2 texCoords = m_TexCoords.xy;

	#ifdef USE_OVERLAY//USE_SWAY
		if (u_Local4.a > 0.0)
		{// Sway...
			texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - m_TexCoords.y) + 1.0), 0.0);
		}
	#endif //USE_OVERLAY//USE_SWAY




	mat3 tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, m_Normal.xyz);
	viewDir = vec3(var_Normal2, var_Tangent.w, var_Bitangent.w);

	E = normalize(viewDir);

	#if defined(USE_LIGHTMAP)

		vec4 lightmapColor = texture2D(u_LightMap, var_TexCoords2.st);
  
		#if defined(RGBM_LIGHTMAP)
			lightmapColor.rgb *= lightmapColor.a;
		#endif //defined(RGBM_LIGHTMAP)
 
		#if defined(USE_TESSELLATION)
			lightmapColor = vec4(1.0);
		#endif //defined(USE_TESSELLATION)

	#endif //defined(USE_LIGHTMAP)


	vec2 ParallaxOffset = vec2(0.0);


	#if defined(USE_PARALLAXMAP)
		vec3 offsetDir = normalize(E * tangentToWorld);
		vec2 ParallaxXY = vec2(0.0);

		if (u_Local5.a > 0.0 && var_Slope > 0)
		{// Steep maps...
			ParallaxXY = offsetDir.xy * 2.5; // Always rock?!?!?!?
		}
		else
		{
			ParallaxXY = offsetDir.xy * u_Local1.x;
		}

		#if defined(FAST_PARALLAX)

			ParallaxOffset = ParallaxXY * RayIntersectDisplaceMap(texCoords);
			texCoords += ParallaxOffset;

		#else //!defined(FAST_PARALLAX)

			// Steep Parallax
			float Step = 0.01;
			vec2 dt = ParallaxXY * Step;
			float Height = 0.5;
			float oldHeight = 0.5;
			vec2 Coord = texCoords;
			vec2 oldCoord = Coord;
			float HeightMap = GetDepth( Coord );
			float oldHeightMap = HeightMap;
 
			while( HeightMap < Height )
			{
				oldHeightMap = HeightMap;
				oldHeight = Height;
				oldCoord = Coord;
 
				Height -= Step;
				Coord += dt;
				HeightMap = GetDepth( Coord );
			}
		
			Coord = (Coord + oldCoord)*0.5;
			if( Height < 0.0 )
			{
				Coord = oldCoord;
				Height = 0.0;
			}
		
			ParallaxOffset = Coord - texCoords;
			texCoords = Coord;

		#endif //defined(FAST_PARALLAX)

	#endif //defined(USE_PARALLAXMAP)




	vec4 diffuse = GetDiffuse(texCoords, ParallaxOffset);

	#if defined(USE_GAMMA2_TEXTURES)
		diffuse.rgb *= diffuse.rgb;
	#endif


	vec4 norm = GetNormal(texCoords, ParallaxOffset);

	N = norm.xyz * 2.0 - 1.0;
	N.xy *= u_NormalScale.xy;
	N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));
	N = tangentToWorld * N;
	N = normalize(N);

	DETAILED_NORMAL = N;


	#if defined(USE_OVERLAY)//USE_STEEPMAP

		//
		// Steep Maps...
		//

		if (var_Slope > 0.0)
		{// Steep maps...
			diffuse.rgb = mix( diffuse.rgb, GetSteepMap(var_nonTCtexCoords, ParallaxOffset).rgb, clamp(var_Slope,0.0,1.0));
		}

	#endif //USE_OVERLAY//USE_STEEPMAP
	

	#if defined(USE_OVERLAY)

		//
		// Overlay Maps...
		//

		#define OVERLAY_HEIGHT 40.0

		if (u_Local5.x > 0.0)
		{// Have overlay map...
			vec2 ovCoords = m_TexCoords.xy + vec2(u_Local5.y); // u_Local5.y == sway ammount
			vec4 overlay = texture2D(u_OverlayMap, ovCoords);

			if (overlay.a > 0.1)
			{// Have an overlay, and it is visible here... Set it as diffuse instead...
				diffuse = overlay;
			}
			else
			{// Have an overlay, but it is not visibile at this pixel... Still need to check if we need a shadow casted on this pixel...
				vec2 ovCoords2 = ovCoords - (tex_offset * OVERLAY_HEIGHT);
				vec4 overlay2 = texture2D(u_OverlayMap, ovCoords2);

				if (overlay2.a > 0.1)
				{// Add shadow...
					diffuse.rgb *= 0.25;
				}
			}
		}

	#endif //USE_OVERLAY

	ambientColor = vec3(0.0);
	lightColor = var_Color.rgb;
	attenuation = 1.0;

	#if defined(USE_LIGHTMAP)
		float lmBrightMult = clamp(1.0 - (length(lightmapColor.rgb) / 3.0), 0.0, 0.9);
		lmBrightMult *= lmBrightMult * 0.7;
		lightColor	= lightmapColor.rgb * lmBrightMult * var_Color.rgb;
	#endif


	#if defined(USE_SHADOWMAP) 

		vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
		float shadowValue = texture2D(u_ShadowMap, shadowTex).r;

		// surfaces not facing the light are always shadowed
		shadowValue *= float(dot(m_Normal.xyz, var_PrimaryLightDir.xyz) > 0.0);

		#if defined(SHADOWMAP_MODULATE)
			vec3 shadowColor = u_PrimaryLightAmbient * lightColor;
			lightColor = mix(shadowColor, lightColor, shadowValue);
		#endif //defined(SHADOWMAP_MODULATE)

	#endif //defined(USE_SHADOWMAP) 



	#if defined(USE_LIGHTMAP)

		ambientColor = lightColor;
		float surfNL = clamp(-dot(var_PrimaryLightDir.xyz, N.xyz/*m_Normal.xyz*/), 0.0, 1.0);
		lightColor /= max(surfNL, 0.25);
		ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);

	#endif //defined(USE_LIGHTMAP)
  

		#if defined(USE_LIGHTMAP) 
			lightColor *= lightmapColor.rgb;
		#endif


		gl_FragColor = vec4 (diffuse.rgb * lightColor, diffuse.a * var_Color.a);

		if (u_Local1.a == 19 || u_Local1.a == 20)
		{// Tree billboards... Need to match tree colors...
			gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.3));
		}

		//specular.a = (1.0 - norm.a);
		specular.a = ((clamp(u_Local1.g, 0.0, 1.0) + clamp(u_Local3.a, 0.0, 1.0)) / 2.0) * 1.6;

		#if defined(USE_SPECULARMAP)
		if (u_Local1.g != 0.0)
		{// Real specMap...
			specular = texture2D(u_SpecularMap, texCoords);
		}
		else
		#endif //defined(USE_SPECULARMAP)
		{// Fake it...
			specular.rgb = gl_FragColor.rgb;
		}

		#if defined(USE_GAMMA2_TEXTURES)
			specular.rgb *= specular.rgb;
		#endif //defined(USE_GAMMA2_TEXTURES)

		specular.rgb *= u_SpecularScale.rgb;



	#if defined(USE_CUBEMAP)
		if (u_Local3.a > 0.0 && u_EnableTextures.w > 0.0 && u_CubeMapStrength > 0.0) 
		{
			float spec = 0.0;

			NE = clamp(dot(m_Normal.xyz/*N*/, E), 0.0, 1.0);

			vec3 reflectance = EnvironmentBRDF(clamp(specular.a, 0.5, 1.0) * 100.0, NE, specular.rgb);
			vec3 R = reflect(E, m_Normal.xyz/*N*/);
			vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;
			vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * u_EnableTextures.w * 0.25; //u_Local3.a
			gl_FragColor.rgb += (cubeLightColor * reflectance * (u_Local3.a * specular.a)) * u_CubeMapStrength;
		}
	#endif


	#if defined(USE_SHADOWMAP)
		gl_FragColor.rgb *= clamp(shadowValue + 0.5, 0.0, 1.0);
	#endif //defined(USE_SHADOWMAP)


	#ifdef USE_SUBSURFACE_SCATTER

		#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)

			// Let's add some sub-surface scatterring shall we???
			if (/*MaterialThickness > 0.0 ||*/ u_Local1.a == 20) // tree leaves
			{
				gl_FragColor.rgb += subScatterFS(gl_FragColor, diffuse, var_PrimaryLightDir.xyz, u_PrimaryLightColor.xyz, E, N, texCoords).rgb;
				gl_FragColor = clamp(gl_FragColor, 0.0, 1.0);
			}

		#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)

	#endif //USE_SUBSURFACE_SCATTER


	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		if (u_Local6.r > 0.0)
		{
			float lambertian2 = dot(var_PrimaryLightDir.xyz,N);
			float spec2 = 0.0;

			if(lambertian2 > 0.0)
			{// this is blinn phong
				vec3 halfDir2 = normalize(var_PrimaryLightDir.xyz + E);
				float specAngle = max(dot(halfDir2, N), 0.0);
				spec2 = pow(specAngle, 16.0);
				gl_FragColor.rgb += vec3(spec2 * (1.0 - specular.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * u_Local5.b;
			}

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightDir = u_lightPositions2[li] - var_vertPos.xyz;
				float lambertian3 = dot(lightDir.xyz,N);
				float spec3 = 0.0;

				if(lambertian3 > 0.0)
				{
					float lightStrength = clamp(1.0 - (length(lightDir) * (1.0 / u_lightDistances[li])), 0.0, 1.0) * 0.5;

					if(lightStrength > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, N), 0.0);
						spec3 = pow(specAngle3, 16.0);
						gl_FragColor.rgb += vec3(spec3 * (1.0 - specular.a)) * u_lightColors[li].rgb * lightStrength * u_Local5.b;
					}
				}
			}
		}
	#endif

	//gl_FragColor.rgb = N.xyz * 0.5 + 0.5;

	#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
	#else
		out_Glow = vec4(0.0);
	#endif

	out_DetailedNormal = vec4(DETAILED_NORMAL.xyz * 0.5 + 0.5, specular.a / 8.0);
	out_PositionMap = vec4(gl_FragCoord.xyz, 0.0);

	if (u_Local1.a == 20 || u_Local1.a == 19 || ((u_Local1.a == 5 || u_Local1.a == 6) && var_usingSteepMap == 0.0)) 
	{// (Foliage/Plants), (billboard trees), ShortGrass, LongGrass
		out_FoliageMap.r = 1.0;
		out_FoliageMap.g = 1.0;
	}
	else
	{
		out_FoliageMap.r = 1.0;
		out_FoliageMap.g = 0.0;
	}
}

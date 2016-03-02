//precision highp float;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SteepMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4	u_Local6; // useSunLightSpecular

//#define SUBSURFACE_SCATTER

varying float  var_Time;

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;
#endif

//#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
//#endif

#if defined(USE_DELUXEMAP)
uniform sampler2D u_DeluxeMap;
#endif

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

#if defined(SUBSURFACE_SCATTER)
uniform sampler2D u_SubsurfaceMap;
#endif

uniform sampler2D u_OverlayMap;

//#if defined(USE_NORMALMAP) || defined(USE_DELUXEMAP) || defined(USE_SPECULARMAP) || defined(USE_CUBEMAP)
// y = deluxe, w = cube
uniform vec4      u_EnableTextures;
//#endif

#if defined(USE_LIGHT_VECTOR)
uniform vec3      u_DirectedLight;
uniform vec3      u_AmbientLight;
uniform vec4		u_LightOrigin;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP)
uniform vec3  u_PrimaryLightColor;
uniform vec3  u_PrimaryLightAmbient;
#endif

uniform vec4      u_NormalScale;
uniform vec4      u_SpecularScale;

#if defined(USE_LIGHT)
#if defined(USE_CUBEMAP)
uniform vec4      u_CubeMapInfo;
uniform float     u_CubeMapStrength;
#endif
#endif


varying vec2      var_TexCoords;
varying vec2	  var_TexCoords2;

varying vec4      var_Color;

varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
#define var_Normal2 var_Normal.w

varying vec3 var_N;

#if defined(USE_LIGHT)
varying vec4      var_LightDir;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP)
varying vec4      var_PrimaryLightDir;
#endif

varying vec3   var_vertPos;

varying vec3      var_ViewDir;




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




out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;

#if defined(USE_PARALLAXMAP)
float SampleDepth(sampler2D normalMap, vec2 t)
{
	return 1.0 - texture2D(normalMap, t).a;
}


float RayIntersectDisplaceMap(vec2 dp, vec2 ds, sampler2D normalMap)
{
	if (u_Local1.x == 0.0)
		return 0.0;
	
  #if !defined(FAST_PARALLAX)
	float MAX_SIZE = u_Local1.x / 3.0;//1.25;//1.5;//1.0;
	if (MAX_SIZE > 1.75) MAX_SIZE = 1.75;
	if (MAX_SIZE < 1.0) MAX_SIZE = 1.0;
	const int linearSearchSteps = 16;
	const int binarySearchSteps = 6;

	// current size of search window
	float size = MAX_SIZE / float(linearSearchSteps);

	// current depth position
	float depth = 0.0;

	// best match found (starts with last position 1.0)
	float bestDepth = MAX_SIZE;

	// search front to back for first point inside object
	for(int i = 0; i < linearSearchSteps - 1; ++i)
	{
		depth += size;
		
		float t = SampleDepth(normalMap, dp + ds * depth) * MAX_SIZE;
		
		//if(bestDepth > 0.996)		// if no depth found yet
		if(bestDepth > MAX_SIZE - (MAX_SIZE / linearSearchSteps))		// if no depth found yet
			if(depth >= t)
				bestDepth = depth;	// store best depth
	}

	depth = bestDepth;
	
	// recurse around first point (depth) for closest match
	for(int i = 0; i < binarySearchSteps; ++i)
	{
		size *= 0.5;

		float t = SampleDepth(normalMap, dp + ds * depth) * MAX_SIZE;
		
		if(depth >= t)
		{
			bestDepth = depth;
			depth -= 2.0 * size;
		}

		depth += size;
	}

	return bestDepth * u_Local1.x;
  #else
	float depth = SampleDepth(normalMap, dp) - 1.0;
	return depth * u_Local1.x;
  #endif
  
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

float CalcLightAttenuation(float point, float normDist)
{
	// zero light at 1.0, approximating q3 style
	// also don't attenuate directional light
	float attenuation = (0.5 * normDist - 1.5) * point + 1.0;

	// clamp attenuation
	#if defined(NO_LIGHT_CLAMP)
	attenuation = max(attenuation, 0.0);
	#else
	attenuation = clamp(attenuation, 0.0, 1.0);
	#endif

	return attenuation;
}


float blinnPhongSpecular(in vec3 normalVec, in vec3 lightVec, in float specPower)
{
    vec3 halfAngle = normalize(normalVec + lightVec);
    return pow(clamp(dot(normalVec,halfAngle),0.0,1.0),specPower);
}
 
#if defined(USE_LIGHT_VECTOR)
float halfLambert(in vec3 vect1, in vec3 vect2)
{
    float product = dot(vect1,vect2);
    return product * 0.5 + 0.5;
}
 
#ifdef SUBSURFACE_SCATTER
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
		MaterialThickness = 0.8;
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

	float attenuation = 10.0 * (1.0 / distance(u_LightOrigin.xyz,m_vertPos.xyz));
    vec3 eVec = normalize(eyeVec);
    vec3 lVec = normalize(lightVec);
    vec3 wNorm = normalize(worldNormal);
     
    vec4 dotLN = vec4(halfLambert(lVec,wNorm) * attenuation);
    dotLN *= BaseColor;
     
    vec3 indirectLightComponent = vec3(subsurface.a * max(0.0,dot(-wNorm,lVec)));
    indirectLightComponent += subsurface.a * halfLambert(-eVec,lVec);
    indirectLightComponent *= attenuation;
    indirectLightComponent.r *= subsurface.r;
    indirectLightComponent.g *= subsurface.g;
    indirectLightComponent.b *= subsurface.b;
     
    vec3 rim = vec3(1.0 - max(0.0,dot(wNorm,eVec)));
    rim *= rim;
    rim *= max(0.0,dot(wNorm,lVec)) * SpecColor.rgb;
     
    vec4 finalCol = dotLN + vec4(indirectLightComponent,1.0);
    finalCol.rgb += (rim * RimScalar * attenuation * finalCol.a);
    finalCol.rgb += vec3(blinnPhongSpecular(wNorm,lVec,SpecPower) * attenuation * SpecColor * finalCol.a * 0.05);
    finalCol.rgb *= LightColor.rgb;
     
    return finalCol;   
}
#endif //SUBSURFACE_SCATTER
#endif //defined(USE_LIGHT_VECTOR)


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
	vec3 L, N, E, H;
	vec3 DETAILED_NORMAL = vec3(1.0);
	float NL, NH, NE, EH, attenuation;
	vec2 tex_offset = vec2(1.0 / u_Dimensions);

	mat3 tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, m_Normal.xyz);
	viewDir = vec3(var_Normal2, var_Tangent.w, var_Bitangent.w);

	E = normalize(viewDir);

	#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
		L = var_LightDir.xyz;
		
		#if defined(USE_DELUXEMAP)
			L += (texture2D(u_DeluxeMap, var_TexCoords2.st).xyz - vec3(0.5)) * u_EnableTextures.y;
		#endif //defined(USE_DELUXEMAP)
	
		float sqrLightDist = dot(L, L);
	#endif //defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)




	#if defined(USE_LIGHTMAP)

		vec4 lightmapColor = texture2D(u_LightMap, var_TexCoords2.st);
  
		#if defined(RGBM_LIGHTMAP)
			lightmapColor.rgb *= lightmapColor.a;
		#endif //defined(RGBM_LIGHTMAP)
 
		#if defined(USE_TESSELLATION)
			lightmapColor = vec4(1.0);
		#endif //defined(USE_TESSELLATION)

	#endif //defined(USE_LIGHTMAP)




	vec2 texCoords = m_TexCoords.xy;

	#ifdef USE_OVERLAY//USE_SWAY
		if (u_Local4.a > 0.0)
		{// Sway...
			texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - m_TexCoords.y) + 1.0), 0.0);
		}
	#endif //USE_OVERLAY//USE_SWAY

	vec4 diffuse = vec4(0.0);

	#if defined(USE_PARALLAXMAP)
		vec3 offsetDir = normalize(E * tangentToWorld);
		offsetDir.xy *= tex_offset * -u_Local1.x;//-4.0;//-5.0; // -3.0
		texCoords += offsetDir.xy * RayIntersectDisplaceMap(texCoords, offsetDir.xy, u_NormalMap);
	#endif

	diffuse = texture2D(u_DiffuseMap, texCoords);

	#if defined(USE_GAMMA2_TEXTURES)
		diffuse.rgb *= diffuse.rgb;
	#endif


	vec4 norm = texture2D(u_NormalMap, texCoords);

	/*
	gl_FragColor = vec4(norm.a, norm.a, norm.a, diffuse.a);
	#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
	#else
		out_Glow = vec4(0.0);
	#endif
	return;
	*/

	N = norm.xyz * 2.0 - 1.0;
	N.xy *= u_NormalScale.xy;
	//N.xyz *= u_NormalScale.xyx;
	N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));
	N = tangentToWorld * N;
	N = normalize(N);

	DETAILED_NORMAL = N;
		

	//gl_FragColor.rgb = N.xyz/*DETAILED_NORMAL.xyz*/ * 0.5 + 0.5;
	//gl_FragColor.a = diffuse.a;
	//return;



	#if defined(USE_OVERLAY)//USE_STEEPMAP

		//
		// Steep Maps...
		//

		if (u_Local5.a > 0.0)
		{// Steep maps...
			float slope = dot(normalize(N.xyz),vec3(0.0,1.0,0.0));
			if (slope < 0.0) slope = slope *= -1.0;
			float slope2 = dot(normalize(N.xyz),vec3(0.0,0.0,1.0));
			if (slope2 < 0.0) slope2 = slope2 *= -1.0;
			float slope3 = dot(normalize(N.xyz),vec3(1.0,0.0,0.0));
			if (slope3 < 0.0) slope3 = slope3 *= -1.0;
			slope = length(slope + slope2 + slope3) / 3.0;
			vec4 steepDiffuse = texture2D(u_SteepMap, texCoords);
			diffuse.rgb = mix( diffuse.rgb, steepDiffuse.rgb, clamp(slope,0.0,1.0));
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




	#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)



		// =========================================================================================================================================
		// Full Lighting stuff...
		// =========================================================================================================================================



		ambientColor = vec3 (0.0);
		attenuation = 1.0;

		#if defined(USE_LIGHTMAP)
			lightColor	= lightmapColor.rgb * (var_Color.rgb * 0.66666 + 0.33333); // UQ1:  * 0.66666 + 0.33333 is because they are too dark...
		#elif defined(USE_LIGHT_VECTOR)
			lightColor	= u_DirectedLight * var_Color.rgb;
			ambientColor = u_AmbientLight * var_Color.rgb;
			attenuation = CalcLightAttenuation(float(var_LightDir.w > 0.0), var_LightDir.w / sqrLightDist);
		#elif defined(USE_LIGHT_VERTEX)
			lightColor	= var_Color.rgb;
		#endif // defined(USE_LIGHTMAP) || defined(USE_LIGHT_VECTOR) || defined(USE_LIGHT_VERTEX)


		L /= sqrt(sqrLightDist);



		#if defined(USE_SHADOWMAP) 

			vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
			float shadowValue = texture2D(u_ShadowMap, shadowTex).r;

			// surfaces not facing the light are always shadowed
			shadowValue *= float(dot(m_Normal.xyz, var_PrimaryLightDir.xyz) > 0.0);

			#if defined(SHADOWMAP_MODULATE)

				//vec3 shadowColor = min(u_PrimaryLightAmbient, lightColor);
				vec3 shadowColor = u_PrimaryLightAmbient * lightColor;

				#if 0
					shadowValue = 1.0 + (shadowValue - 1.0) * clamp(dot(L, var_PrimaryLightDir.xyz), 0.0, 1.0);
				#endif
		
				lightColor = mix(shadowColor, lightColor, shadowValue);

			#endif //defined(SHADOWMAP_MODULATE)

		#endif //defined(USE_SHADOWMAP) 



		#if defined(USE_LIGHTMAP) || defined(USE_LIGHT_VERTEX)

			ambientColor = lightColor;
			float surfNL = clamp(dot(m_Normal.xyz, L), 0.0, 1.0);
			lightColor /= max(surfNL, 0.25);
			ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);

		#endif //defined(USE_LIGHTMAP) || defined(USE_LIGHT_VERTEX)
  

		vec3 reflectance;

		NL = clamp(dot(N, L), 0.0, 1.0);
		NE = clamp(dot(N, E), 0.0, 1.0);

		gl_FragColor = vec4(0.0);



		if (u_Local1.g != 0.0)
		{// Real specMap...
			specular = texture2D(u_SpecularMap, texCoords);
		}
		else
		{// Fake it...
			if (u_Local1.b > 0.0)
			{
				specular.rgb = clamp( pow( vec3(((length(diffuse.rgb) / 3.0) * 2.0) * (1.0-norm.a)) , vec3(3.0)) , 0.0, 1.0) * 0.5;
				specular.a = (1.0 - norm.a);

				//specular.rgb = vec3(0.04);
			}
			else
			{
				specular = vec4(1.0);
			}
		}

		#if defined(USE_GAMMA2_TEXTURES)
			specular.rgb *= specular.rgb;
		#endif //defined(USE_GAMMA2_TEXTURES)


		specular.rgb *= (length(specular.rgb) / 3.0) * u_SpecularScale.rgb;


		if (u_Local4.b != 0.0)
		{// Metalic...
			float metallic = specular.r;

			specular.rgb = (0.96 * metallic) * diffuse.rgb + vec3(0.04);
			diffuse.rgb *= 1.0 - metallic;
		}
		else
		{// Non Metalic...
			diffuse.rgb *= vec3(1.0) - specular.rgb;
		}



		// Reduce brightness of really bright spots (eg: light right above a reflective surface)
		float refMult = (specular.r + specular.g + specular.b + specular.a) / 4.0;
		refMult = 1.0 - refMult;
		refMult = clamp(refMult, 0.25, 0.75);


		reflectance = diffuse.rgb;

		#if defined(r_deluxeSpecular) || defined(USE_LIGHT_VECTOR)
			float lambertian = dot(L.xyz,N);
			float spec = 0.0;

			if(lambertian > 0.0)
			{// this is blinn phong
				vec3 halfDir = normalize(L.xyz + E);
				float specAngle = max(dot(halfDir, N), 0.0);
				spec = pow(specAngle, 16.0);
				reflectance.rgb += vec3(spec * (1.0 - specular.a)) * reflectance.rgb * lightColor.rgb * u_Local5.b;
			}
		#endif


		gl_FragColor.rgb  += lightColor   * reflectance * (attenuation/* * NL*/);
		gl_FragColor.rgb += ambientColor * (diffuse.rgb + (specular.rgb * refMult));



		#if defined(USE_CUBEMAP)
			if (u_Local3.a > 0.0 && u_EnableTextures.w > 0.0) 
			{
				reflectance = EnvironmentBRDF(specular.a * refMult, NE, specular.rgb * refMult);
				vec3 R = reflect(E, N);
				vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;
				vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * u_EnableTextures.w;
				gl_FragColor.rgb += (cubeLightColor * reflectance * (u_Local3.a * refMult)) * u_CubeMapStrength;
			}
		#endif


		#if defined(USE_SHADOWMAP)
			gl_FragColor.rgb *= clamp(shadowValue + 0.5, 0.0, 1.0);
		#endif //defined(USE_SHADOWMAP)


		gl_FragColor.a = diffuse.a * var_Color.a;




	#else //!(defined(USE_LIGHT) && !defined(USE_FAST_LIGHT))



		// =========================================================================================================================================
		// Ambient Lighting Only Stuff...
		// =========================================================================================================================================



		lightColor = var_Color.rgb;
	
		#if defined(USE_LIGHTMAP) 
			lightColor *= lightmapColor.rgb;
		#endif


		gl_FragColor = vec4 (diffuse.rgb * lightColor, diffuse.a * var_Color.a);


	#endif //defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)


	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		if (u_Local6.r > 0.0)
		{
			if (u_Local1.b > 0.0)
			{
				specular.a = (1.0 - norm.a);
			}
			else
			{
				specular.a = 1.0;
			}

			float lambertian2 = dot(var_PrimaryLightDir.xyz,N);
			float spec2 = 0.0;

			if(lambertian2 > 0.0)
			{// this is blinn phong
				vec3 halfDir2 = normalize(var_PrimaryLightDir.xyz + E);
				float specAngle = max(dot(halfDir2, N), 0.0);
				spec2 = pow(specAngle, 16.0);
				gl_FragColor.rgb += vec3(spec2 * (1.0 - specular.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * u_Local5.b;
			}
		}
	#endif


	#ifdef SUBSURFACE_SCATTER

		#if defined(USE_LIGHT_VECTOR) || defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)

			// Let's add some sub-surface scatterring shall we???
			//if (MaterialThickness > 0.0 || u_Local4.z != 0.0 /*|| u_Local1.a == 5 || u_Local1.a == 6 || u_Local1.a == 12 
			//	|| u_Local1.a == 14 || u_Local1.a == 15 || u_Local1.a == 16 || u_Local1.a == 17 || u_Local1.a == 19 
			//	|| u_Local1.a == 20 || u_Local1.a == 21 || u_Local1.a == 22*/)
			if (u_Local1.a == 20)
			{
				#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
					gl_FragColor.rgb += subScatterFS(gl_FragColor, gl_FragColor, var_PrimaryLightDir.xyz, u_PrimaryLightColor.xyz, E, N, texCoords).rgb;
				#else
					gl_FragColor.rgb += subScatterFS(gl_FragColor, gl_FragColor, L, lightColor.xyz, E, N, texCoords).rgb;
				#endif

				gl_FragColor = clamp(gl_FragColor, 0.0, 1.0);
			}

		#endif //defined(USE_LIGHT_VECTOR) || defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)

	#endif //SUBSURFACE_SCATTER



	#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
	#else
		out_Glow = vec4(0.0);
	#endif

	out_DetailedNormal = vec4(DETAILED_NORMAL.xyz, specular.a / 8.0);

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
}

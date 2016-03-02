//precision highp float;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SteepMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap

//#define USE_LIGHTING

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




#define Texture2 u_OverlayMap
#define Texture3 u_OverlayMap
#define Texture4 u_OverlayMap

uniform vec4 u_Local10; // AmbientMulti, LightMulti, Intensity
#define AmbientMulti u_Local10.r
#define LightMulti u_Local10.g
#define Intensity u_Local10.b

in Grass {
#ifdef USE_LIGHTING
  vec3 LightColor;
  vec2 ColorMulti;
#endif //USE_LIGHTING
  vec2 GrassTexCoords;
  vec3 Normal;
} grass;

flat in int Tex;

#define m_TexCoords (vec2(1.0) - grass.GrassTexCoords)
#define m_Normal grass.Normal
#define m_vertPos var_vertPos
#define m_ViewDir var_ViewDir





out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;





void main()
{
	float NearLum;
	float FarLum;

	vec4 color;

	//textures & factor for brightening with distance
	if (Tex == 1)
	{
		NearLum = 1.0;
		FarLum = 2.0;
		color = texture2D(Texture2, vec2(1.0)-grass.GrassTexCoords);
	}
	else
	{
		NearLum = 0.75;
		FarLum = (Tex == 3)? 0.75: 1.5;
		color = (Tex == 3)? texture2D(Texture4, vec2(1.0)-grass.GrassTexCoords): texture2D(Texture3, vec2(1.0)-grass.GrassTexCoords);
	}
	
	//brighten with distance
	float Dist = gl_FragCoord.z / gl_FragCoord.w;	   //get the distance from camera
	color.rgb *= mix(NearLum, FarLum, clamp(Dist/25.0,0.0,1.5));


#ifdef USE_LIGHTING
	//random coloring
	vec3 RandomColor = (grass.ColorMulti.x > 1.19)? vec3(0.15,0.125,0.1): vec3(0.0);    //Needs moved to the geo shader - unfortunately exceeds max output components there
	RandomColor = (grass.ColorMulti.x < 0.81)? vec3(0.1,0.04,0.04): RandomColor;        //Needs moved to the geo shader - unfortunately exceeds max output components there

	//assemble coloring and lighting
	color.rgb += RandomColor;
    color.rgb *= grass.ColorMulti.x*(AmbientMulti+1.0);
	color.rgb *= clamp(grass.LightColor*LightMulti, vec3(0.0), vec3(LightMulti*1.25));
	color.rgb *= ((1.0-grass.GrassTexCoords.y)*0.6+1.2)*(Intensity);

	//assemble alpha and LOD fade factoring
	color.a = color.a*grass.ColorMulti.y*1.375;
#endif //USE_LIGHTING


	gl_FragColor = color;


#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		vec3 viewDir = vec3(0.0);

		mat3 tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, m_Normal.xyz);
		viewDir = vec3(var_Normal2, var_Tangent.w, var_Bitangent.w);

		vec3 E = normalize(viewDir);

		float lambertian2 = dot(var_PrimaryLightDir.xyz, m_Normal.xyz);
		float spec2 = 0.0;

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 halfDir2 = normalize(var_PrimaryLightDir.xyz + E);
			float specAngle = max(dot(halfDir2, m_Normal.xyz), 0.0);
			spec2 = pow(specAngle, 16.0);
			gl_FragColor.rgb += vec3(spec2 * 0.25) * gl_FragColor.rgb * u_PrimaryLightColor.rgb;
		}
#endif

	out_Glow = vec4(0.0);

	out_DetailedNormal = vec4(m_Normal.xyz, 0.1);

	out_FoliageMap.r = 1.0;
	out_FoliageMap.g = 1.0;
}

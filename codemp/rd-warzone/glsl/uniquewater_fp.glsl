uniform sampler2D u_DiffuseMap;
//uniform sampler2D u_OverlayMap; // sky diffuse

varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec4	u_Local2; // ExtinctionCoefficient
varying vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower
varying vec2	var_Dimensions;

varying float  var_Time;

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

#if defined(USE_NORMALMAP) || defined(USE_DELUXEMAP) || defined(USE_SPECULARMAP) || defined(USE_CUBEMAP)
// y = deluxe, w = cube
uniform vec4      u_EnableTextures;
#endif

#if defined(USE_LIGHT_VECTOR) && !defined(USE_FAST_LIGHT)
uniform vec3      u_DirectedLight;
uniform vec3      u_AmbientLight;
uniform vec4		u_LightOrigin;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
uniform vec3  u_PrimaryLightColor;
uniform vec3  u_PrimaryLightAmbient;
#endif

//#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4      u_NormalScale;
uniform vec4      u_SpecularScale;
//#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
#if defined(USE_CUBEMAP)
uniform vec4      u_CubeMapInfo;
#endif
#endif


varying vec2      var_TexCoords;

varying vec4      var_Color;

varying vec3   var_ViewDir;

varying vec3   var_Normal;

varying vec3 var_N;

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
varying vec4      var_PrimaryLightDir;
#endif

varying vec3   var_vertPos;

out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;


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
	vec3 L, N, E, H;
	float NL, NH, NE, EH, attenuation;
	vec2 tex_offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

	mat3 tangentToWorld = cotangent_frame(var_Normal.xyz, -var_ViewDir, var_TexCoords.xy);
	viewDir = var_ViewDir;

	E = normalize(viewDir);

	L = vec3(0.0, 1.0, 1.0);
	float sqrLightDist = dot(L, L);

  #if defined(USE_LIGHT_VECTOR)
	lightColor	= u_DirectedLight * var_Color.rgb;
	ambientColor = u_AmbientLight * var_Color.rgb;
	attenuation = CalcLightAttenuation(float(var_LightDir.w > 0.0), var_LightDir.w / sqrLightDist);
  #else
	lightColor	= var_Color.rgb;
  #endif

	vec2 texCoords = var_TexCoords.xy;
	
	L /= sqrt(sqrLightDist);


	//
	// Water
	//
	
	vec2 cPos = -1.0 + 2.0 * texCoords.xy;
	float cLength = length(cPos);

	vec2 uv = texCoords.xy+(cPos/cLength)*cos(cLength*12.0-var_Time*4.0)*0.03;
	vec4 diffuse = texture2D(u_DiffuseMap, uv);
	vec4 orig_diffuse = diffuse;
	gl_FragColor = vec4(diffuse.rgb,1.0);
	/*
	vec2 uv = texCoords.xy;
	vec4 diffuse = texture2D(u_DiffuseMap, uv);
	vec4 orig_diffuse = diffuse;
	gl_FragColor = vec4(diffuse.rgb,1.0);
	*/

	/*
	vec4 skyColor = texture2D(u_OverlayMap, uv2);

	if (skyColor.a > 0.0)
	{// Add some sky color...
		//diffuse.rgb = ((diffuse.rgb * 2.0) + skyColor.rgb) / 3.0;
		gl_FragColor.rgb = skyColor.rgb;
	}
	*/

	N.xyz = normalize(var_Normal.xyz + vec3(uv.xy, (uv.x + uv.y) / 2.0)) * 2.0 - 1.0; // make one up... lol

	//
	//
	//

	/*
#if defined (USE_CUBEMAP)
	// Cubemapping...
	vec3 R = reflect(E, N);
	uv = R.xy+(cPos/cLength)*cos(cLength*12.0-var_Time*4.0)*0.03;
	vec3 cubeLightColor = textureCubeLod(u_CubeMap, vec3(uv, R.z) * viewDir, 7.0 - specular.a * 7.0).rgb;
	gl_FragColor.rgb = ((gl_FragColor.rgb * 19.0) + cubeLightColor) / 20.0;
#endif
	*/

	gl_FragColor.a = 0.9;//1.0;
	out_Glow = vec4(0.0);


	out_DetailedNormal = vec4(N.xyz, 0.75);

	out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
}

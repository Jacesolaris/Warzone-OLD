uniform sampler2D u_DiffuseMap;
uniform sampler2D u_RandomMap;
uniform sampler2D u_ScreenDepthMap;

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

float SampleDepth(sampler2D normalMap, vec2 t)
{
	vec3 color = texture2D(u_DiffuseMap, t).rgb;

#define const_1 ( 16.0 / 255.0)
#define const_2 (255.0 / 219.0)
	vec3 color2 = ((color - const_1) * const_2);
#define const_3 ( 125.0 / 255.0)
#define const_4 (255.0 / 115.0)
		
	color = ((color - const_3) * const_4);

	color = clamp(color * color * (color * 5.0), 0.0, 1.0); // testing

	vec3 orig_color = color + color2;

	orig_color = clamp(orig_color * 2.5, 0.0, 1.0); // testing

	orig_color = clamp(orig_color, 0.0, 1.0);
	float combined_color2 = orig_color.r + orig_color.g + orig_color.b;
	combined_color2 /= 4.0;

	return clamp(1.0 - combined_color2, 0.0, 1.0);
}

vec3 EnvironmentBRDF(float gloss, float NE, vec3 specular)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96,0.25 ) * gloss;
	t += vec4( 0.0, 0.0, (0.015 - 0.75 * 0.04)/0.96,0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28 * NE ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + specular * ( a1 - a0 ), 0.0, 1.0 );
}

float CalcGGX(float NH, float gloss)
{
	float a_sq = exp2(gloss * -13.0 + 1.0);
	float d = ((NH * NH) * (a_sq - 1.0) + 1.0);
	return a_sq / (d * d);
}

float CalcFresnel(float EH)
{
	return exp2(-10.0 * EH);
}

float CalcVisibility(float NH, float NL, float NE, float EH, float gloss)
{
	float roughness = exp2(gloss * -6.5);

	float k = roughness + 1.0;
	k *= k * 0.125;

	float k2 = 1.0 - k;
	
	float invGeo1 = NL * k2 + k;
	float invGeo2 = NE * k2 + k;

	return 1.0 / (invGeo1 * invGeo2);
}


vec3 CalcSpecular(vec3 specular, float NH, float NL, float NE, float EH, float gloss, float shininess)
{
	float distrib = CalcGGX(NH, gloss);

	vec3 fSpecular = mix(specular, vec3(1.0), CalcFresnel(EH));

	float vis = CalcVisibility(NH, NL, NE, EH, gloss);

	return fSpecular * (distrib * vis);
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





vec3 DETAILED_NORMAL = vec3(0.0);

vec4 doGrass( in vec2 fragCoord, in mat3 tangentToWorld )
{
#define m_GrassTexScale 1.0
#define m_MaskScale 1.0

	DETAILED_NORMAL = var_Normal.xyz * 2.0 - 1.0;
	DETAILED_NORMAL *= tangentToWorld;
        
	vec4 color = texture2D( u_DiffuseMap, fragCoord * vec2(m_GrassTexScale,m_GrassTexScale));
    color.a = (texture2D( u_RandomMap, fragCoord * m_MaskScale).a);
	return color;
}

void main()
{
	vec3 viewDir, lightColor, ambientColor;
	vec4 specular = vec4(1.0);
	vec3 L, N, E, H;
	//vec3 NORMAL = vec3(1.0);
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
	float fakedepth = SampleDepth(u_DiffuseMap, texCoords);

	float norm = (fakedepth - 0.5);
	float norm2 = 0.0 - (fakedepth - 0.5);
    #if defined(SWIZZLE_NORMALMAP)
		N.xy = vec2(norm, norm2);
    #else
		N.xy = vec2(norm, norm2);
    #endif
	N = N * 0.5 + 0.5;
	N.xy *= u_NormalScale.xy;
	N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));

	N = tangentToWorld * N;

	//vec3 normal = texture2D(u_NormalMap, texCoords).xyz;
	//DETAILED_NORMAL = normalize(normal * 2.0 - 1.0);
	//DETAILED_NORMAL = tangentToWorld * DETAILED_NORMAL;
	//DETAILED_NORMAL = N;

	N = normalize(N);
	L /= sqrt(sqrLightDist);

	vec4 diffuse = texture2D(u_DiffuseMap, texCoords);
	vec4 orig_diffuse = diffuse;

	vec3 grassColor = doGrass( texCoords.xy, tangentToWorld ).rgb;
	diffuse.rgb = clamp(grassColor, 0.0, 1.0);
	gl_FragColor = vec4(diffuse.rgb, diffuse.a);


/*
	// Cubemapping...
	NL = clamp(dot(N, L), 0.0, 1.0);
	NE = clamp(dot(N, E), 0.0, 1.0);
	vec3 reflectance = EnvironmentBRDF(specular.a, NE, specular.rgb);
	vec3 R = reflect(E, N);
	vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;
	vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * u_EnableTextures.w;
	gl_FragColor.rgb += (cubeLightColor * reflectance);
*/

		out_Glow = vec4(0.0);
		//out_Normal = vec4(NORMAL.xyz, 0.0);
		out_DetailedNormal = vec4(DETAILED_NORMAL.xyz, 0.75);
		//out_DetailedNormal = vec4(N.xyz, 0.75);
}

uniform sampler2D u_DiffuseMap;
varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, meterialType
varying vec4	u_Local2; // ExtinctionCoefficient
varying vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower
varying vec2	var_Dimensions;

varying float  var_Time;

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;
#endif

#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
#endif

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


varying vec4      var_TexCoords;

varying vec4      var_Color;

varying vec3   var_ViewDir;

#if (defined(USE_LIGHT) && !defined(USE_FAST_LIGHT))
  #if defined(USE_VERT_TANGENT_SPACE)
varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
  #else
varying vec3   var_Normal;
  #endif
#else
  #if defined(USE_VERT_TANGENT_SPACE)
varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
  #else
varying vec3   var_Normal;
  #endif
#endif

varying vec3 var_N;

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
varying vec4      var_LightDir;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
varying vec4      var_PrimaryLightDir;
#endif

varying vec3   var_vertPos;

out vec4 out_Glow;

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

// Water
vec2 iResolution = var_Dimensions;

float iGlobalTime = var_Time * 1.3;

/*
#define NUM_STEPS		8
#define PI	 			3.1415
//#define EPSILON			1e-3
const float EPSILON		= 1e-3;
//const float		= 0.1 / iResolution.x;
const float EPSILON_NRM = 0.1 * tex_offset;
*/

const int NUM_STEPS = 8;
const float PI	 	= 3.1415;
const float EPSILON	= 1e-3;
float EPSILON_NRM	= 1.0;//0.1 / iResolution.x;

// sea
const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
//const float SEA_SPEED = 0.8;
//const float SEA_FREQ = 0.16;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.36;
const vec3 SEA_BASE = vec3(0.1,0.19,0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
float SEA_TIME = iGlobalTime * SEA_SPEED;
mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

// math
mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
	return m;
}
float hash( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

// lighting
float diffuser(vec3 n,vec3 l,float p) {
    return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(vec3 n,vec3 l,vec3 e,float s) {    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sea
float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    //return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
	return pow(1.0-pow(wv.x * wv.y,0.65),choppy) * 0.3;
}

float map(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float map_detailed(vec3 p, float scale) {
    float freq = SEA_FREQ * scale;
    float amp = SEA_HEIGHT * scale;
    float choppy = SEA_CHOPPY * scale;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {  
    float fresnel = 1.0 - max(dot(n,-eye),0.0);
    fresnel = pow(fresnel,3.0) * 0.65;
        
    //vec3 reflected = getSkyColor(reflect(eye,n));    
	vec3 reflected = vec3(0.2, 0.3, 0.5);
    vec3 refracted = SEA_BASE + diffuser(n,l,80.0) * SEA_WATER_COLOR * 0.12; 
    
    vec3 color = mix(refracted,reflected,fresnel);
    
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    color += vec3(specular(n,l,eye,60.0));
    
    return color;
}

// tracing
vec3 getNormal(vec3 p, float eps, float scale) {
    vec3 n;
    n.y = map_detailed(p, scale);    
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z), scale) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps), scale) - n.y;
    n.y = eps;
    return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {  
    float tm = 0.0;
    float tx = 1000.0;    
    float hx = map(ori + dir * tx);
    if(hx > 0.0) return tx;   
    float hm = map(ori + dir * tm);    
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));                   
        p = ori + dir * tmid;                   
    	float hmid = map(p);
		if(hmid < 0.0) {
        	tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

void main()
{
	vec3 viewDir, lightColor, ambientColor;
	vec3 L, N, E, H;
	float NL, NH, NE, EH, attenuation;
	vec2 tex_offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

	mat3 tangentToWorld = cotangent_frame(var_Normal.xyz, -var_ViewDir, var_TexCoords.xy);
	viewDir = var_ViewDir;

	E = normalize(viewDir);

	L = var_LightDir.xyz;
  #if defined(USE_DELUXEMAP)
	L += (texture2D(u_DeluxeMap, var_TexCoords.zw).xyz - vec3(0.5)) * u_EnableTextures.y;
  #endif
	float sqrLightDist = dot(L, L);

	vec4 lightmapColor = texture2D(u_LightMap, var_TexCoords.zw);
  #if defined(RGBM_LIGHTMAP)
	lightmapColor.rgb *= lightmapColor.a;
  #endif

  #if defined(USE_LIGHTMAP)
	lightColor	= lightmapColor.rgb * var_Color.rgb;
  #elif defined(USE_LIGHT_VECTOR)
	lightColor	= u_DirectedLight * var_Color.rgb;
	ambientColor = u_AmbientLight * var_Color.rgb;
	attenuation = CalcLightAttenuation(float(var_LightDir.w > 0.0), var_LightDir.w / sqrLightDist);
  #elif defined(USE_LIGHT_VERTEX)
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
	N.xy *= u_NormalScale.xy;
	N.z = sqrt(clamp((0.25 - N.x * N.x) - N.y * N.y, 0.0, 1.0));
	N = tangentToWorld * N;

	N = normalize(N);
	L /= sqrt(sqrLightDist);

	vec4 diffuse = texture2D(u_DiffuseMap, texCoords);
	vec4 orig_diffuse = diffuse;

	// Water/Lava Code...
	vec2 uv = texCoords.xy;
	uv = uv * 2.0 - 1.0;
	uv.x *= iResolution.x / iResolution.y;
	float current_time = iGlobalTime * 0.3;
        
	// ray
	vec3 ang = vec3(0.0, 1.0, 0.0);
	vec3 ori = vec3(1.0, 1.0, 1.0);
	vec3 dir = normalize(vec3(uv.xy,-PI/*-2.0*/)); 
	dir.z += length(uv) * 0.15;
	dir = normalize(dir) * fromEuler(ang);
		
	// tracing
	vec3 p;
	heightMapTracing(ori,dir,p);
		
	vec3 dist = p - ori;
	
	float scaleWater = 1.0;
	if (N.b < N.r && N.b < N.g) scaleWater = 10.0;
	float d = length(dist);
	vec3 n = getNormal(p, d*d*.0003, scaleWater);

	vec3 light = normalize(vec3(0.0,1.0,0.8)); 
	//vec3 light = normalize(vec3(0.0,0.4,0.3)); 
	//vec3 light = normalize(lightColor.rgb);//normalize(vec3(0.0,1.0,0.8));
		
	// color
	/*vec3 color = mix(
		diffuse.rgb,
		getSeaColor(p,n,light,dir,dist),
   		pow(smoothstep(0.0,-0.05,dir.y),0.3));*/

	vec3 color = getSeaColor(p,n,light,dir,dist);
        
	// post
	diffuse.rgb = vec3(pow(color,vec3(0.75)));
	//diffuse.rgb *= 0.75;
	//diffuse.rgb = getSeaColor(p,n,light,dir,dist);

	float waveheight = (diffuse.r + diffuse.g + diffuse.b) / 3.0;

	gl_FragColor = vec4(diffuse.rgb, diffuse.a);
	//out_Glow = vec4(0.0, 0.0, 0.0, 0.0);
	//return;

#if defined(USE_GAMMA2_TEXTURES)
	diffuse.rgb *= diffuse.rgb;
#endif

	ambientColor = vec3 (0.0);
	attenuation = 1.0;

  #if defined(USE_SHADOWMAP) 
	vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
	float shadowValue = texture2D(u_ShadowMap, shadowTex).r;

	// surfaces not facing the light are always shadowed
	shadowValue *= float(dot(var_Normal.xyz, var_PrimaryLightDir.xyz) > 0.0);

    #if defined(SHADOWMAP_MODULATE)
	//vec3 shadowColor = min(u_PrimaryLightAmbient, lightColor);
	vec3 shadowColor = u_PrimaryLightAmbient * lightColor;

      #if 0
	shadowValue = 1.0 + (shadowValue - 1.0) * clamp(dot(L, var_PrimaryLightDir.xyz), 0.0, 1.0);
      #endif
	lightColor = mix(shadowColor, lightColor, shadowValue);
    #endif
  #endif

  #if defined(USE_LIGHTMAP) || defined(USE_LIGHT_VERTEX)
	ambientColor = lightColor;
	float surfNL = clamp(dot(var_Normal.xyz, L), 0.0, 1.0);
	lightColor /= max(surfNL, 0.25);
	ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
  #endif
  
	vec3 reflectance;

	NL = clamp(dot(N, L), 0.0, 1.0);
	NE = clamp(dot(N, E), 0.0, 1.0);

	vec4 specular;

	specular = vec4(1.0-fakedepth) * diffuse;
	specular.a = ((clamp((1.0 - fakedepth), 0.0, 1.0) * 0.5) + 0.5);
	specular.a = clamp((specular.a * 2.0) * specular.a, 0.2, 0.9);

    #if defined(USE_GAMMA2_TEXTURES)
	specular.rgb *= specular.rgb;
    #endif

	specular *= 1.5;

	diffuse.rgb *= vec3(1.0) - specular.rgb;

	reflectance = diffuse.rgb;

	//gl_FragColor.rgb += ambientColor * (diffuse.rgb + specular.rgb);
	//gl_FragColor.rgb += ambientColor * diffuse.rgb;

  #if defined(USE_CUBEMAP)
	// Cubemapping...
	reflectance = EnvironmentBRDF(specular.a, NE, specular.rgb);
	vec3 R = reflect(E, N);
	vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * viewDir;
	vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * u_EnableTextures.w;
	gl_FragColor.rgb += (cubeLightColor * reflectance) * 3.0;
  #endif

  #if defined(USE_PRIMARY_LIGHT)
	vec3 L2, H2;
	float NL2, EH2, NH2;

	L2 = var_PrimaryLightDir.xyz;

	// enable when point lights are supported as primary lights
	//sqrLightDist = dot(L2, L2);
	//L2 /= sqrt(sqrLightDist);

	NL2 = clamp(dot(N, L2), 0.0, 1.0);

	H2 = normalize(L2 + E);
	EH2 = clamp(dot(E, H2), 0.0, 1.0);
	NH2 = clamp(dot(N, H2), 0.0, 1.0);

	reflectance  = diffuse.rgb;
	reflectance += CalcSpecular(specular.rgb, NH2, NL2, NE, EH2, specular.a, exp2(specular.a * 13.0));

	lightColor = u_PrimaryLightColor * var_Color.rgb;

	// enable when point lights are supported as primary lights
	//lightColor *= CalcLightAttenuation(float(u_PrimaryLightDir.w > 0.0), u_PrimaryLightDir.w / sqrLightDist);

    #if defined(USE_SHADOWMAP)
	lightColor *= shadowValue;
    #endif

	// enable when point lights are supported as primary lights
	//lightColor *= CalcLightAttenuation(float(u_PrimaryLightDir.w > 0.0), u_PrimaryLightDir.w / sqrLightDist);

	gl_FragColor.rgb += lightColor * reflectance * NL2;
  #endif

	//gl_FragColor.rgb = N;
	if (scaleWater > 1.0)
	{
		gl_FragColor.rgb = clamp((gl_FragColor.rgb + orig_diffuse.rgb) / 2.0, 0.0, 1.0);
		gl_FragColor.a = clamp(waveheight, 0.5, 1.0);//diffuse.a * var_Color.a;

#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
		out_Glow.a = 1.0;
		//out_Glow = vec4(0.0);
#else
		out_Glow = vec4(0.0);
#endif
	}
	else
	{
		gl_FragColor.rgb = clamp((gl_FragColor.rgb + gl_FragColor.rgb + orig_diffuse.rgb) / 3.0, 0.0, 1.0);
		gl_FragColor.a = clamp(waveheight, 0.0, 1.0);//diffuse.a * var_Color.a;

#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
		out_Glow.a = 0.3;
		//out_Glow = vec4(0.0);
#else
		out_Glow = vec4(0.0);
#endif
	}
}

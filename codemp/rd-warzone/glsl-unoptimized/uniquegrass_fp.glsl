uniform sampler2D u_DiffuseMap;
varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec4	u_Local2; // ExtinctionCoefficient
varying vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower
varying vec2	var_Dimensions;

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

//#if defined(USE_NORMALMAP) || defined(USE_DELUXEMAP) || defined(USE_SPECULARMAP) || defined(USE_CUBEMAP)
// y = deluxe, w = cube
uniform vec4      u_EnableTextures;
//#endif

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
varying vec3	var_ViewPos;

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

const int c_terramarch_steps = 8;//32;//64;
const int c_grassmarch_steps = 12;
const float c_maxdist = 20000.;
const float c_grassmaxdist = 3.;
const float c_scale = .001;
const float c_height = 16.;
const float c_rslope = 1. / (c_scale * c_height);
const float c_gscale =  15.;
const float c_gheight = 1.0;
const float c_rgslope = 1. / (c_gscale * c_gheight);
const vec3 c_skycolor = vec3(.59, .79, 1.);

float t = iGlobalTime;

float ambient = .8;

float hash(in float p) { return fract(sin(p) * 43758.2317); }
float hash(in vec2 p) { return hash(dot(p, vec2(87.1, 313.7))); }
vec2 hash2(in float p) {
	float x = hash(p);
	return vec2(x, hash(p+x));
}
vec2 hash2(in vec2 p) { return hash2(dot(p, vec2(87.1, 313.7))); }

float noise(in vec2 p) {
	vec2 F = floor(p), f = fract(p);
	f = f * f * (3. - 2. * f);
	return mix(
		mix(hash(F), 			 hash(F+vec2(1.,0.)), f.x),
		mix(hash(F+vec2(0.,1.)), hash(F+vec2(1.)),	  f.x), f.y);
}

vec2 noise2(in vec2 p) {
	vec2 F = floor(p), f = fract(p);
	f = f * f * (3. - 2. * f);
	return mix(
		mix(hash2(F), 			  hash2(F+vec2(1.,0.)), f.x),
		mix(hash2(F+vec2(0.,1.)), hash2(F+vec2(1.)),	f.x), f.y);
}

float fnoise(in vec2 p) {
	return .5 * noise(p) + .25 * noise(p*2.03) + .125 * noise(p*3.99);
}

struct ray_t {
	vec3 o, d;
};
	
struct xs_t {
	float l;
	vec3 pos, nor;
	float occlusion;
};
	
struct tree_t {
	vec2 pos;
	float r;
};

xs_t empty_xs(float maxdist) {
	return xs_t(maxdist, vec3(0.), vec3(0.), 0.);
}

xs_t ray_xs(in ray_t ray, float dist) {
	return xs_t(dist, ray.o + ray.d * dist, vec3(0.), 0.);
}
	
float height(in vec2 p) {
	float n = fnoise(p * c_scale); 
	return (n - .5) * c_height * 2.;
}

vec2 wind_displacement(in vec2 p) {
	return noise2(p*.1+t) * 0.1;
}

float grass_height(in vec3 p) {
	float base_h = height(p.xz);
	float depth = 1. - (base_h - p.y) / c_gheight;
	vec2 gpos = (p.xz + depth * wind_displacement(p.xz));
	return base_h - noise(gpos * c_gscale) * c_gheight;
}

vec3 grass_normal(in vec3 p) {
	return vec3(0.,1.,0.);
}

xs_t trace_terrain(in ray_t ray, float Lmax) {
	float L = 0.;
	for (int i = 0; i < c_terramarch_steps; ++i) {
		vec3 pos = ray.o + ray.d * L;
		float h = height(pos.xz);
		float dh = pos.y - h;
		if (dh < .005*L) break;
		L += dh;// * c_rslope;
		if (L > Lmax) break;
	}
	return ray_xs(ray, L);
}

xs_t trace_grass(in ray_t ray, float Lmin, float Lmax) {
	float L = Lmin;
	for (int i = 0; i < c_grassmarch_steps; ++i) {
		vec3 pos = ray.o + ray.d * L;
		float h = grass_height(pos);
		float dh = pos.y - h;
		if (dh < .005*L) break;
		L += dh * c_rgslope;
		if (L > Lmax) break;
	}
	vec3 pos = ray.o + ray.d * L;
	float occlusion = 1. - 2.*(height(pos.xz) - pos.y) / c_gheight;
	return xs_t(L, pos, grass_normal(pos), (L>Lmax)?1.:min(1.,occlusion));
}

vec3 shade_grass(in xs_t xs) {
	vec2 typepos = xs.pos.xz + wind_displacement(xs.pos.xz);
	float typemask1 = fnoise(2.5*typepos);
	float typemask2 = pow(fnoise(.4*typepos), 3.);
	float typemask3 = step(.71,fnoise(.8*typepos));
	vec3 col1 = vec3(.6, .87, .5);
	vec3 col2 = vec3(.7, .73, .4)*.3;
	vec3 col3 = vec3(1., 1., .1);
	vec3 col4 = vec3(1., .4, .7);
	vec3 color = mix(mix(mix(col1, col2, typemask1),
			col3, typemask2), col4, typemask3) * ambient;
	color *= xs.occlusion;
	return color;
}

ray_t lookAtDir(in vec3 uv_dir, in vec3 pos, in vec3 at) {
	vec3 f = normalize(at - pos);
	vec3 r = cross(f, vec3(0.,1.,0.));
	vec3 u = cross(r, f);
	return ray_t(pos, normalize(uv_dir.x * r + uv_dir.y * u + uv_dir.z * f));
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

	vec3 offsetDir = normalize(E * tangentToWorld);

	// Water/Lava Code...
	vec2 uv = texCoords.xy;
	uv = uv * 2.0 - 1.0;
	uv.x *= iResolution.x / iResolution.y;
	float current_time = iGlobalTime * 0.3;
        
	//vec3 pos = vec3(uv, 0.);
	//pos += vec3(sin(.3*sin(t*.18)), 0., sin(.2*cos(t*.18)))*20.;
	//pos += vec3(30., 5.+height(pos.xz), 10.);

	ray_t ray = lookAtDir(normalize(vec3(uv, 0.2)), vec3(0.0, 6.0, 1.0)/*pos*/, vec3(0.));
	//ray_t ray = lookAtDir(normalize(vec3(uv, 0.5))*tangentToWorld, vec3(0.0, 6.0, 1.0)/*pos*/, vec3(0.));
	//ray_t ray = lookAtDir(normalize((vec3(uv, 0.5))+viewDir)*tangentToWorld, vec3(0.0, 6.0, 1.0)/*pos*/, vec3(0.));
	//ray_t ray = lookAtDir(normalize((E*N)+(vec3(uv, 1.0))*tangentToWorld), vec3(0.0, 5.0, 1.0), vec3(0.5));
	//ray_t ray = lookAtDir(normalize((vec3(uv, 0.5))*tangentToWorld), vec3(0.0, 6.0, 1.0)+(N), vec3(0.));
	//ray_t ray = lookAtDir(normalize((vec3(uv, 0.5))*tangentToWorld), var_vertPos, var_ViewPos);
	//ray_t ray = lookAtDir(normalize(vec3(uv, 0.5)), normalize((var_ViewDir/*var_ViewPos-var_vertPos*/)*0.25)+vec3(0.0, 6.0, 1.0),vec3(0.));
	
	vec3 color = vec3(0.);

	xs_t xs = empty_xs(c_maxdist);
	xs_t terr = trace_terrain(ray, xs.l);
	if (terr.l < xs.l) {
		xs = trace_grass(ray, terr.l, terr.l+c_grassmaxdist);
	}
	
	//if (xs.l < c_maxdist) {
		color = shade_grass(xs);
	//	color = mix(color, c_skycolor, smoothstep(c_maxdist*.35, c_maxdist, xs.l));
	//} else {
	//	color = c_skycolor;
	//}
	
	// gamma correction is for those who understand it
	//fragColor = vec4(pow(color, vec3(2.2)), 1.);
	gl_FragColor = vec4(color, 1.);

	out_Glow = vec4(0.0, 0.0, 0.0, 0.0);
	return;

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

	gl_FragColor.rgb = clamp((gl_FragColor.rgb + gl_FragColor.rgb + orig_diffuse.rgb) / 3.0, 0.0, 1.0);
	gl_FragColor.a = 1.0;//diffuse.a * var_Color.a;

#if defined(USE_GLOW_BUFFER)
	out_Glow = gl_FragColor;
	out_Glow.a = 0.3;
	//out_Glow = vec4(0.0);
#else
	out_Glow = vec4(0.0);
#endif
}

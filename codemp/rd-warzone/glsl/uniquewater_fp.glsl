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

// Water
vec2 iResolution = var_Dimensions;

float iGlobalTime = var_Time * 1.3;

float coast2water_fadedepth = 0.10;
float large_waveheight      = 0.;//0.50; // change to adjust the "heavy" waves
float large_wavesize        = 0.;//4.;  // factor to adjust the large wave size
float small_waveheight      = .6;  // change to adjust the small random waves
float small_wavesize        = 0.25;   // factor to ajust the small wave size
float water_softlight_fact  = 120.;//15.;  // range [1..200] (should be << smaller than glossy-fact)
float water_glossylight_fact= 200.;//120.; // range [1..200]
float particle_amount       = 70.;
vec3 watercolor             = vec3(0.43, 0.60, 0.66); // 'transparent' low-water color (RGB)
vec3 watercolor2            = vec3(0.06, 0.07, 0.11); // deep-water color (RGB, should be darker than the low-water color)
vec3 water_specularcolor    = vec3(1.3, 1.3, 0.9);    // specular Color (RGB) of the water-highlights
vec3 light                  = vec3(-0., sin(iGlobalTime*0.5)*.5 + .35, 2.8); // position of the sun





// calculate random value
float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

// 2d noise function
float noise1( in vec2 x )
{
  vec2 p  = floor(x);
  vec2 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0;
  return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
}

float noise(vec2 p)
{
  return texture2D(u_RandomMap,p*vec2(1./256.)).x;
}

float height_map( vec2 p )
{
  mat2 m = mat2( 0.9563*1.4,  -0.2924*1.4,  0.2924*1.4,  0.9563*1.4 );
  p = p*6.;
  float f = 0.6000*noise1( p ); p = m*p*1.1;
  f += 0.2500*noise1( p ); p = m*p*1.32;
  f += 0.1666*noise1( p ); p = m*p*1.11;
  f += 0.0834*noise( p ); p = m*p*1.12;
  f += 0.0634*noise( p ); p = m*p*1.13;
  f += 0.0444*noise( p ); p = m*p*1.14;
  f += 0.0274*noise( p ); p = m*p*1.15;
  f += 0.0134*noise( p ); p = m*p*1.16;
  f += 0.0104*noise( p ); p = m*p*1.17;
  f += 0.0084*noise( p );
  const float FLAT_LEVEL = 0.525;
  if (f<FLAT_LEVEL)
      f = f;
  else
      f = pow((f-FLAT_LEVEL)/(1.-FLAT_LEVEL), 2.)*(1.-FLAT_LEVEL)*2.0+FLAT_LEVEL; // makes a smooth coast-increase
  return clamp(f, 0., 10.);
}

vec3 terrain_map( vec2 p )
{
  //return vec3(0.7, .55, .4)+texture2D(iChannel1, p*2.).rgb*.5; // test-terrain is simply 'sandstone'
  return vec3(0.0);
}

const mat2 m = mat2( 0.72, -1.60,  1.60,  0.72 );

float water_map( vec2 p, float height )
{
  vec2 p2 = p*large_wavesize;
  vec2 shift1 = 0.001*vec2( iGlobalTime*160.0*2.0, iGlobalTime*120.0*2.0 );
  vec2 shift2 = 0.001*vec2( iGlobalTime*190.0*2.0, -iGlobalTime*130.0*2.0 );

  // coarse crossing 'ocean' waves...
  float f = 0.6000*noise( p );
  f += 0.2500*noise( p*m );
  f += 0.1666*noise( p*m*m );
  float wave = sin(p2.x*0.622+p2.y*0.622+shift2.x*4.269)*large_waveheight*f*height*height ;

  p *= small_wavesize;
  f = 0.;
  float amp = 1.0, s = .5;
  for (int i=0; i<9; i++)
  { p = m*p*.947; f -= amp*abs(sin((noise( p+shift1*s )-.5)*2.)); amp = amp*.59; s*=-1.329; }
 
  return wave+f*small_waveheight;
}

float nautic(vec2 p)
{
  p *= 18.;
  float f = 0.;
  float amp = 1.0, s = .5;
  for (int i=0; i<3; i++)
  { p = m*p*1.2; f += amp*abs(smoothstep(0., 1., noise( p+iGlobalTime*s ))-.5); amp = amp*.5; s*=-1.227; }
  return pow(1.-f, 5.);
}

float particles(vec2 p)
{
  p *= 200.;
  float f = 0.;
  float amp = 1.0, s = 1.5;
  for (int i=0; i<3; i++)
  { p = m*p*1.2; f += amp*noise( p+iGlobalTime*s ); amp = amp*.5; s*=-1.227; }
  return pow(f*.35, 7.)*particle_amount;
}


float test_shadow( vec2 xy, float height)
{
    vec3 r0 = vec3(xy, height);
    vec3 rd = normalize( light - r0 );
    
    float hit = 1.0;
    float t   = 0.001;
    for (int j=1; j<25; j++)
    {
        vec3 p = r0 + t*rd;
        float h = height_map( p.xy );
        float height_diff = p.z - h;
        if (height_diff<0.0)
        {
            return 0.0;
        }
        t += 0.01+height_diff*.02;
        hit = min(hit, 2.*height_diff/t); // soft shaddow   
    }
    return hit;
}

vec3 CalcTerrain(vec2 uv, float height)
{
  vec3 col = terrain_map( uv );
  float h1 = height_map(uv-vec2(0., 0.01));
  float h2 = height_map(uv+vec2(0., 0.01));
  float h3 = height_map(uv-vec2(0.01, 0.));
  float h4 = height_map(uv+vec2(0.01, 0.));
  vec3 norm = normalize(vec3(h3-h4, h1-h2, 1.));
  vec3 r0 = vec3(uv, height);
  vec3 rd = normalize( light - r0 );
  float grad = dot(norm, rd);
  col *= grad+pow(grad, 8.);
  float terrainshade = test_shadow( uv, height );
  col = mix(col*.25, col, terrainshade);
  return col;
}

vec3 DETAILED_NORMAL = vec3(0.0);

vec4 doWater( in vec2 fragCoord, in mat3 tangentToWorld )
{
	vec2 uv = (fragCoord.xy - vec2(-0.12, +0.25));

    float WATER_LEVEL = 2.4; // Water level (range: 0.0 - 2.0)
    //if (iMouse.z>0.)
	//	WATER_LEVEL = iMouse.x*.003; 
    float deepwater_fadedepth   = 0.5 + coast2water_fadedepth;
    //if (iMouse.z>0.)
	//  deepwater_fadedepth = iMouse.y*0.003 + coast2water_fadedepth;
    
    //float height = height_map( uv );
	float height = noise(uv);
	//float height = (1.0 - texture2D( u_ScreenDepthMap, uv ).x) * 3.0;// * 255.0;
	//return vec4(height,height,height,1.0);
    vec3 col;
    
    float waveheight = clamp(WATER_LEVEL*3.-1.5, 0., 1.);
    float level = WATER_LEVEL + .2*water_map(uv*15. + vec2(iGlobalTime*.1), waveheight);
    //if (height > level)
    //{
    //    col = CalcTerrain(uv, height);
    //}
    //if (height <= level)
    {
        vec2 dif = vec2(.0, .01);
        vec2 pos = uv*15. + vec2(iGlobalTime*.01);
        float h1 = water_map(pos-dif,waveheight);
        float h2 = water_map(pos+dif,waveheight);
        float h3 = water_map(pos-dif.yx,waveheight);
        float h4 = water_map(pos+dif.yx,waveheight);
        vec3 normwater = normalize(vec3(h3-h4, h1-h2, .125)); // norm-vector of the 'bumpy' water-plane
        uv += normwater.xy*.002*(level-height);

		//DETAILED_NORMAL = clamp(var_Normal + vec3(h3-h4, h1-h2, .125), -1.0, 1.0);
		DETAILED_NORMAL = clamp(vec3(h3-h4, h1-h2, .125), -1.0, 1.0);
		//DETAILED_NORMAL = var_Normal;
		//DETAILED_NORMAL = normwater;
		//DETAILED_NORMAL = normwater * 2.0 - 1.0;
		//DETAILED_NORMAL = normwater * 0.5 + 0.5;
		DETAILED_NORMAL *= tangentToWorld;
        
        col = CalcTerrain(uv, height);

        float coastfade = clamp((level-height)/coast2water_fadedepth, 0., 1.);
        float coastfade2= clamp((level-height)/deepwater_fadedepth, 0., 1.);
        float intensity = col.r*.2126+col.g*.7152+col.b*.0722;
        watercolor = mix(watercolor*intensity, watercolor2, smoothstep(0., 1., coastfade2));

        vec3 r0 = vec3(uv, WATER_LEVEL);
        vec3 rd = normalize( light - r0 ); // ray-direction to the light from water-position
        float grad     = dot(normwater, rd); // dot-product of norm-vector and light-direction
        float specular = pow(grad, water_softlight_fact);  // used for soft highlights                          
        float specular2= pow(grad, water_glossylight_fact); // used for glossy highlights
        float gradpos  = dot(vec3(0., 0., 1.), rd);
        float specular1= smoothstep(0., 1., pow(gradpos, 5.));  // used for diffusity (some darker corona around light's specular reflections...)                          
        float watershade  = test_shadow( uv, level );
        watercolor *= 2.2+watershade;
   		watercolor += (.2+.8*watershade) * ((grad-1.0)*.5+specular) * .25;
   		watercolor /= (1.+specular1*1.25);
   		watercolor += watershade*specular2*water_specularcolor;
        watercolor += watershade*coastfade*(1.-coastfade2)*(vec3(.5, .6, .7)*nautic(uv)+vec3(1., 1., 1.)*particles(uv));
        
        col = mix(col, watercolor, coastfade);
    }
    
	return vec4(col , 1.0);
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

	float scaleWater = 1.0;
	if (N.b < N.r && N.b < N.g) scaleWater = 10.0;
	//diffuse.rgb = (diffuse.rgb + clamp(vec3(0.1,0.19,0.22) + vec3(pow(doWater( texCoords.xy, tangentToWorld ).rgb * 2.0,vec3(0.75))), 0.0, 1.0)) / 2.0;
	diffuse.rgb = clamp(vec3(0.1,0.19,0.22) + vec3(pow(doWater( texCoords.xy, tangentToWorld ).rgb * 2.0,vec3(0.75))), 0.0, 1.0);
	float waveheight = (diffuse.r + diffuse.g + diffuse.b) / 3.0;
	gl_FragColor = vec4(diffuse.rgb, diffuse.a);

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

  #if defined(USE_LIGHT_VERTEX)
	ambientColor = lightColor;
	float surfNL = clamp(dot(var_Normal.xyz, L), 0.0, 1.0);
	lightColor /= max(surfNL, 0.25);
	ambientColor = clamp(ambientColor - lightColor * surfNL, 0.0, 1.0);
  #endif
  
	vec3 reflectance;

	NL = clamp(dot(N, L), 0.0, 1.0);
	NE = clamp(dot(N, E), 0.0, 1.0);

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
	gl_FragColor.rgb += (cubeLightColor * reflectance);// * 3.0;
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

	if (scaleWater >= 10.0)
	{
		//gl_FragColor.rgb = clamp((gl_FragColor.rgb + orig_diffuse.rgb) / 2.0, 0.0, 1.0);
		gl_FragColor.a = clamp(waveheight, 0.5, 1.0);//diffuse.a * var_Color.a;

#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
		//out_Glow.a = 1.0;
		out_Glow = vec4(0.0);
#else
		out_Glow = vec4(0.0);
#endif
	}
	else
	{
		//gl_FragColor.rgb = clamp((gl_FragColor.rgb + gl_FragColor.rgb + orig_diffuse.rgb) / 3.0, 0.0, 1.0);
		gl_FragColor.a = clamp(waveheight, 0.1, 1.0);//diffuse.a * var_Color.a;

#if defined(USE_GLOW_BUFFER)
		//out_Glow = gl_FragColor;
		//out_Glow.a = 0.3;
		out_Glow = vec4(0.0);
#else
		out_Glow = vec4(0.0);
#endif
	}

	//if (u_EnableTextures.r > 0.0)
	{
		//out_Normal = vec4(NORMAL.xyz, 0.0);
		out_DetailedNormal = vec4(DETAILED_NORMAL.xyz, 0.75);
		//out_DetailedNormal = vec4(N.xyz, 0.75);
	}
}

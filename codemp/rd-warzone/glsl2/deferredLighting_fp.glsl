#define __AMBIENT_OCCLUSION__
#define __ENVMAP__
//#define __NORMAL_METHOD_1__
#define __NORMAL_METHOD_2__
#define __RANDOMIZE_LIGHT_PIXELS__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_OverlayMap; // Real normals. Alpha channel 1.0 means enabled...
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_HeightMap;
uniform sampler2D	u_ShadowMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;
uniform samplerCube	u_CubeMap;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, r_ao, r_env
uniform vec4		u_Local2; // SSDO, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4
uniform vec4		u_Local4; // MAP_INFO_MAXSIZE, MAP_WATER_LEVEL, floatTime, 0.0

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

#define MAX_DEFERRED_LIGHTS 64//128//64//16//24

uniform int			u_lightCount;
//uniform vec2		u_lightPositions[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float		u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float		u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightColors[MAX_DEFERRED_LIGHTS];

varying vec2		var_TexCoords;


vec2 pixel = vec2(1.0) / u_Dimensions;


#ifdef __RANDOMIZE_LIGHT_PIXELS__
float lrand(vec2 co) {
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453) * 0.25 + 0.75;
}
#endif //__RANDOMIZE_LIGHT_PIXELS__

float rand(vec2 co) {
        return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float hash( float n ) {
    return fract(sin(n)*687.3123);
}

float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0;
    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
               mix( hash(n+157.0), hash(n+158.0),f.x),f.y);
}

vec3 TangentFromNormal ( vec3 normal )
{
	vec3 tangent;
	vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0)); 
	vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0)); 

	if( length(c1) > length(c2) )
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	return normalize(tangent);
}

#ifdef __NORMAL_METHOD_1__
const vec3 LUMA_COEFFICIENT = vec3(0.2126, 0.7152, 0.0722);

float lumaAtCoord(vec2 coord) {
  vec3 pixel = texture(u_DiffuseMap, coord).rgb;
  float luma = dot(pixel, LUMA_COEFFICIENT);
  return luma;
}

vec4 normalVector(vec2 coord) {
  float lumaU0 = lumaAtCoord(coord + vec2(-1.0,  0.0) / u_Dimensions);
  float lumaU1 = lumaAtCoord(coord + vec2( 1.0,  0.0) / u_Dimensions);
  float lumaV0 = lumaAtCoord(coord + vec2( 0.0, -1.0) / u_Dimensions);
  float lumaV1 = lumaAtCoord(coord + vec2( 0.0,  1.0) / u_Dimensions);

  vec2 slope = vec2(lumaU0 - lumaU1, lumaV0 - lumaV1) * 0.5 + 0.5;

// Contrast...
#define normLower ( 128.0/*48.0*/ / 255.0 )
#define normUpper (255.0 / 192.0/*128.0*/ )
  slope = clamp((clamp(slope - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

  return vec4(slope, 1.0, length(slope.rg / 2.0));
}
#endif //__NORMAL_METHOD_1__

#ifdef __NORMAL_METHOD_2__
float getHeight(vec2 uv) {
  return length(texture(u_DiffuseMap, uv).rgb) / 3.0;
}

vec4 bumpFromDepth(vec2 uv, vec2 resolution, float scale) {
  vec2 step = 1. / resolution;
    
  float height = getHeight(uv);
    
  vec2 dxy = height - vec2(
      getHeight(uv + vec2(step.x, 0.)), 
      getHeight(uv + vec2(0., step.y))
  );

  vec3 N = vec3(dxy * scale / step, 1.);

// Contrast...
#define normLower ( 128.0 / 255.0 )
#define normUpper (255.0 / 192.0 )
  N = clamp((clamp(N - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

  return vec4(normalize(N) * 0.5 + 0.5, height);
}

vec4 normalVector(vec2 coord) {
	return bumpFromDepth(coord, u_Dimensions, 0.1 /*scale*/);
}
#endif //__NORMAL_METHOD_2__


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)
float drawObject(in vec3 p){
    p = abs(fract(p)-.5);
    return dot(p, vec3(.5));
}

float cellTile(in vec3 p)
{
    p /= 5.5;
    // Draw four overlapping objects at various positions throughout the tile.
    vec4 v, d; 
    d.x = drawObject(p - vec3(.81, .62, .53));
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.y = drawObject(p - vec3(.39, .2, .11));
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawObject(p - vec3(.62, .24, .06));
    p.xz = vec2(p.z-p.x, p.z + p.x)*.7071;
    d.w = drawObject(p - vec3(.2, .82, .64));

    v.xy = min(d.xz, d.yw), v.z = min(max(d.x, d.y), max(d.z, d.w)), v.w = max(v.x, v.y); 
   
    d.x =  min(v.z, v.w) - min(v.x, v.y); // Maximum minus second order, for that beveled Voronoi look. Range [0, 1].
    //d.x =  min(v.x, v.y); // First order.
        
    return d.x*2.66; // Normalize... roughly.
}

float map(vec3 p)
{
    float n = (.5-cellTile(p))*1.5;
    return p.y + dot(sin(p/2. + cos(p.yzx/2. + 3.14159/2.)), vec3(.5)) + n;
}
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)


#ifdef __AMBIENT_OCCLUSION__
float calculateAO(in vec3 pos, in vec3 nor)
{
	float sca = 0.00013/*2.0*/, occ = 0.0;
    for( int i=0; i<5; i++ ){
    
        float hr = 0.01 + float(i)*0.5/4.0;        
        float dd = map(nor * hr + pos);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - occ, 0.0, 1.0 );    
}
#endif //__AMBIENT_OCCLUSION__

#ifdef __ENVMAP__
vec3 envMap(vec3 p, float warmth)
{
    float c = cellTile(p*6.);
    c = smoothstep(0.2, 1., c); // Contract gives it more of a lit look... kind of.
    
	// Icy glow... for whatever reason.
    vec3 coolMap = vec3(pow(c, 8.), c*c, c);
    
	// Alternate firey glow.
    vec3 heatMap = vec3(min(c*1.5, 1.), pow(c, 2.5), pow(c, 12.));

	// Mix Ice and Heat based on warmth setting...
	return mix(coolMap, heatMap, warmth);
}
#endif //__ENVMAP__



//
// Full lighting... Blinn phong and basic lighting as well...
//

// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor) {
	vec3 halfLV = normalize(light + view);
	float spe = pow(max(dot(normal, halfLV), 0.0), 32.0);
	float dif = dot(normal, light) * 0.5 + 0.75;
	return dif*diffuseColor + spe*specularColor;
}

void main(void)
{
	vec4 color = textureLod(u_DiffuseMap, var_TexCoords, 0.0);
	gl_FragColor = vec4(color.rgb, 1.0);

	vec2 texCoords = var_TexCoords;


	vec3 viewOrg = u_ViewOrigin.xyz;
	vec4 position = textureLod(u_PositionMap, texCoords, 0.0);

	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);

	if (position.a == MATERIAL_SKY || position.a == MATERIAL_SUN || position.a == MATERIAL_NONE)
	{// Skybox... Skip...
		return;
	}

#if 0
	if (u_Local3.r > 0.0)
	{// Debug position map by showing pixel distance from view...
		float dist = distance(u_ViewOrigin.xyz, position.xyz);
		float d = clamp(dist / u_Local3.r, 0.0, 1.0);
		gl_FragColor.rgb = vec3(d);
		gl_FragColor.a = 1.0;
		return;
	}
#endif

	vec4 norm = textureLod(u_NormalMap, texCoords, 0.0);
	vec4 normalDetail = textureLod(u_OverlayMap, texCoords, 0.0);

	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	
	if (normalDetail.a < 1.0)
	{// Don't have real normalmap, make normals for this pixel...
		normalDetail = normalVector(texCoords);
	}

	normalDetail.rgb = normalize(normalDetail.rgb * 2.0 - 1.0);
	normalDetail.rgb *= 0.25;
	//normalDetail.z = sqrt(clamp((0.25 - normalDetail.x * normalDetail.x) - normalDetail.y * normalDetail.y, 0.0, 1.0));
	norm.rgb = normalize(norm.rgb + normalDetail.rgb);

	//vec3 tangent = TangentFromNormal( norm.xyz );
	//vec3 bitangent = normalize( cross(norm.xyz, tangent) );
	//mat3 tangentToWorld = mat3(tangent.xyz, bitangent.xyz, norm.xyz);
	//norm.xyz = tangentToWorld * normalDetail.xyz;

	vec3 N = norm.xyz;


	float shadowMult = 1.0;

#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0)
	{
		float shadowValue = texture(u_ShadowMap, texCoords).r;

		shadowValue = pow(shadowValue, 1.5);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);

		gl_FragColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
		shadowMult = clamp(shadowValue, 0.2, 1.0);
	}
#endif //defined(USE_SHADOWMAP)


	vec3 surfaceToCamera = normalize(u_ViewOrigin.xyz - position.xyz);

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	bool noSunPhong = false;
	float phongFactor = u_Local1.r;

	if (phongFactor*u_Local1.g < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
		noSunPhong = true;
		phongFactor = 0.0;
	}

	vec4 occlusion = vec4(0.0);
	bool useOcclusion = false;

	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
	}

	float reflectivePower = (max(norm.a, 0.3) * 0.3);


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)
	vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
	float to_light_dist = length(to_light);
	vec3 to_light_norm = (to_light / to_light_dist);

	vec3 to_pos = u_ViewOrigin.xyz - position.xyz;
	float to_pos_dist = length(to_pos);
	vec3 to_pos_norm = (to_pos / to_pos_dist);

	#define rd reflect(surfaceToCamera, N)
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__)


	float lightPower = clamp(1.0 - clamp(max(gl_FragColor.r, max(gl_FragColor.g, gl_FragColor.b)), 0.0, 1.0), 0.0, 0.8);

	if (!noSunPhong && shadowMult > 0.0 && lightPower > 0.0)
	{// this is blinn phong
		float light_occlusion = 1.0;

		if (useOcclusion)
		{
			light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm*E, 1.0), occlusion), 0.0, 1.0);
		}

		vec3 lColor = blinn_phong(N, E, -to_light_norm, u_PrimaryLightColor.rgb, u_PrimaryLightColor.rgb) * light_occlusion * reflectivePower * phongFactor * 0.5;

#ifdef __RANDOMIZE_LIGHT_PIXELS__
		float lightRand = lrand(texCoords * length(lColor.rgb) * u_Local4.b);
		gl_FragColor.rgb = gl_FragColor.rgb + (lColor * max(norm.a, 0.3) * shadowMult * lightPower * lightRand);
#else //!__RANDOMIZE_LIGHT_PIXELS__
		gl_FragColor.rgb = gl_FragColor.rgb + (lColor * max(norm.a, 0.3) * shadowMult * lightPower);
#endif //__RANDOMIZE_LIGHT_PIXELS__
	}

	lightPower = clamp(1.0 - clamp(max(gl_FragColor.r, max(gl_FragColor.g, gl_FragColor.b)), 0.0, 1.0), 0.0, 0.8);

	if (u_lightCount > 0.0 && lightPower > 0.0)
	{
		if (noSunPhong)
		{// Invert phong value so we still have non-sun lights...
			phongFactor = -u_Local1.r;
		}

		vec3 addedLight = vec3(0.0);

		for (int li = 0; li < u_lightCount; li++)
		{
			vec3 lightPos = u_lightPositions2[li].xyz;

			float lightDist = distance(lightPos, position.xyz);

			if (u_lightHeightScales[li] > 0.0)
			{// ignore height differences, check later...
				lightDist -= length(lightPos.z - position.z);
			}

			float lightMax = u_lightDistances[li];

			float lightDistMult = clamp(1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0), 0.0, 1.0);

			if (lightDist < lightMax && lightDistMult > 0.0)
			{
				float lightStrength = 1.0 - clamp(lightDist / lightMax, 0.0, 1.0);
				lightStrength = pow(lightStrength, 2.0);

				if (lightStrength > 0.0)
				{
					vec3 lightColor = u_lightColors[li].rgb;
					vec3 lightDir = normalize(lightPos - position.xyz);
					float light_occlusion = 1.0;

					addedLight.rgb += lightColor * lightStrength * lightDistMult * lightPower;// * u_Local3.g;

					if (useOcclusion)
					{
						light_occlusion = (1.0 - clamp(dot(vec4(-lightDir*E, 1.0), occlusion), 0.0, 1.0));
					}

					lightColor = lightStrength * lightDistMult * lightColor;

					vec3 lColor = blinn_phong(N, E, lightDir, lightColor, lightColor);
					addedLight.rgb += (lColor * light_occlusion * lightStrength * lightDistMult * reflectivePower * phongFactor * lightPower * 8.0);//u_Local3.r/*1.5*/);
				}
			}
		}

#ifdef __RANDOMIZE_LIGHT_PIXELS__
		float lightRand = lrand(texCoords * length(addedLight.rgb) * u_Local4.b);
		gl_FragColor.rgb = gl_FragColor.rgb + (addedLight * reflectivePower * lightRand);
#else //!__RANDOMIZE_LIGHT_PIXELS__
		gl_FragColor.rgb = gl_FragColor.rgb + (addedLight * reflectivePower);
#endif //__RANDOMIZE_LIGHT_PIXELS__
	}

#ifdef __AMBIENT_OCCLUSION__
	if (u_Local1.b > 0.0)
	{
		float ao = calculateAO(to_light_norm, N * 10000.0);
		ao = clamp(ao, 0.3, 1.0);
		gl_FragColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __ENVMAP__
	if (u_Local1.a > 0.0)
	{
		float lightScale = clamp(1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale), 0.2, 1.0);
		vec3 env = envMap(rd, 0.6 /* warmth */);
		gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + ((env * (norm.a * 0.5) * invLightScale) * lightScale), (norm.a * 0.5) * lightScale);
	}
#endif //__ENVMAP__
}


//#define __SSS__
#define __AMBIENT_OCCLUSION__
#define __ENVMAP__
//#define __NORMAL_METHOD_1__
#define __NORMAL_METHOD_2__
//#define __EMISSION__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
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
uniform vec4		u_Local4; // MAP_INFO_MAXSIZE, MAP_WATER_LEVEL, 0.0, 0.0

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

#define MAX_DEFERRED_LIGHTS 128//64//16//24

uniform int			u_lightCount;
uniform vec2		u_lightPositions[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float		u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float		u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightColors[MAX_DEFERRED_LIGHTS];

varying vec2		var_TexCoords;

#define textureCubeLod textureLod // UQ1: > ver 140 support


#define LIGHT_THRESHOLD  0.001

float rand(vec2 co) {
        return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 pixel = vec2(1.0) / u_Dimensions;

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


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__SSS__)
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
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__SSS__)

#ifdef __SSS__
float calcSSS( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    for( int i=0; i<8; i++ )
    {
        float h = 0.002 + 0.11*float(i)/7.0;
        vec3 dir = normalize( sin( float(i)*13.0 + vec3(0.0,2.1,4.2) ) );
        dir *= sign(dot(dir,nor));
        occ += (h-map(pos-h*dir));
    }
    occ = clamp( 1.0 - 11.0*occ/8.0, 0.0, 1.0 );    
    return occ*occ;
}
#endif //__SSS__

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

	if (position.a == 1024.0 || position.a == 1025.0)
	{// Skybox... Skip...
		//gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
		return;
	}

#if 0 // Debugging stuff
	/*if (u_Local3.g >= 2.0)
	{
		if (u_Local3.g >= 4.0)
			gl_FragColor.rgb = vec3((position.rg / u_Local3.r), 0.0) * 0.5 + 0.5;
		else if (u_Local3.g >= 3.0)
			gl_FragColor.rgb = vec3(position.b / u_Local3.r) * 0.5 + 0.5;
		else
			gl_FragColor.rgb = vec3(position.rgb / u_Local3.r) * 0.5 + 0.5;
		return;
	}

	if (u_Local3.g >= 1.0)
	{*/
		float dist = distance(position.xyz, u_ViewOrigin.xyz);
		dist = clamp(dist / 4096.0, 0.0, 1.0);
		gl_FragColor.rgb = vec3(dist);
		return;
	//}
#endif


	vec4 norm = textureLod(u_NormalMap, texCoords, 0.0);

	norm.rgb = norm.rgb * 2.0 - 1.0;
	vec4 normalDetail = normalVector(texCoords);
	normalDetail.rgb = normalDetail.rgb * 2.0 - 1.0;
	normalDetail.rgb *= 0.25;
	normalDetail.z = sqrt(clamp((0.25 - normalDetail.x * normalDetail.x) - normalDetail.y * normalDetail.y, 0.0, 1.0));
	norm.rgb = normalize(norm.rgb + normalDetail.rgb);

	vec3 N = norm.xyz;


	float shadowMult = 1.0;

#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0)
	{
		float shadowValue = texture(u_ShadowMap, texCoords).r;

#if 1
		shadowValue = pow(shadowValue, 1.5);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);

		gl_FragColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
		shadowMult = clamp(shadowValue, 0.2, 1.0);
#else		
		gl_FragColor.rgb *= shadowValue;
		shadowMult = clamp(shadowValue, 0.2, 1.0);
#endif
	}
#endif //defined(USE_SHADOWMAP)

	float lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);


	vec3 specular = gl_FragColor.rgb;
#define specLower ( 48.0 / 255.0)
#define specUpper (255.0 / 192.0)
	specular = clamp((clamp(specular.rgb - specLower, 0.0, 1.0)) * specular, 0.0, 1.0);


	vec3 surfaceToCamera = normalize(u_ViewOrigin.xyz - position.xyz);

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	bool noSunPhong = false;
	float phongFactor = u_Local1.r;

	if (phongFactor < 0.0)
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

	float reflectivePower = (norm.a * 0.2);


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__SSS__)
	vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
	float to_light_dist = length(to_light);
	vec3 to_light_norm = (to_light / to_light_dist);

	vec3 to_pos = u_ViewOrigin.xyz - position.xyz;
	float to_pos_dist = length(to_pos);
	vec3 to_pos_norm = (to_pos / to_pos_dist);

	#define rd reflect(surfaceToCamera, N)
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__SSS__)



	if (!noSunPhong && shadowMult > 0.0)
	{// this is blinn phong
		float light_occlusion = 1.0;

		if (useOcclusion)
		{
			light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm*E, 1.0), occlusion), 0.0, 1.0);
		}

		vec3 lColor = blinn_phong(N, E, -to_light_norm, gl_FragColor.rgb, specular) * light_occlusion * reflectivePower * phongFactor * 8.0;
		gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + (lColor * norm.a * shadowMult), norm.a * lightScale);
		lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);
	}

	if (u_lightCount > 0.0)
	{
		if (noSunPhong)
		{// Invert phong value so we still have non-sun lights...
			phongFactor = -u_Local1.r;
		}

		lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);
		//float invLightScale = clamp((1.0 - lightScale), 0.2, 0.7);

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

					if (useOcclusion)
					{
						light_occlusion = (1.0 - clamp(dot(vec4(-lightDir*E, 1.0), occlusion), 0.0, 1.0));
					}

					vec3 lColor = blinn_phong(N, E, lightDir, gl_FragColor.rgb * lightColor, specular * lightColor);
					addedLight.rgb += (lColor * light_occlusion * lightStrength * lightScale * lightDistMult * reflectivePower * phongFactor * 32.0);
				}
			}
		}

		gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + clamp(addedLight * norm.a * lightScale, 0.0, 1.0), norm.a * lightScale);
	}

#ifdef __AMBIENT_OCCLUSION__
	if (u_Local1.b > 0.0)
	{
		float ao = calculateAO(to_light_norm, N * 10000.0);
		
		ao = clamp(ao, 0.3, 1.0);

#ifdef __SSS__
		if (position.a == 1.0 || position.a == 2.0 || position.a == 19.0 || position.a == 20.0 || position.a == 5.0 || position.a == 6.0)
		{
			float sss = calcSSS( to_light_norm, N * 10000.0 );
			float dif1 = clamp( dot(N,to_light_norm), 0.0, 1.0 );
			vec3 sssColor = 0.4*sss*(vec3(0.15,0.1,0.05)+vec3(u_PrimaryLightColor.rgb)*dif1)*(0.05+0.95*ao)*0.333; // sss
			gl_FragColor.rgb += sssColor;
		}
#endif //__SSS__

		gl_FragColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __ENVMAP__
	if (u_Local1.a > 0.0)
	{
		lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale), 0.2, 1.0);

		vec3 env = envMap(rd, 0.6 /* warmth */);//.5;
		gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + ((env * (norm.a * 0.5) * invLightScale) * lightScale), (norm.a * 0.5) * lightScale);
	}
#endif //__ENVMAP__

#ifdef __EMISSION__
	if (u_Local3.r > 0.0)
	{
		float shinyness = reflectivePower;
		
		if (u_Local3.g > 0.0)
			shinyness = (norm.a * 0.5);

		if (u_Local3.b > 0.0)
			shinyness = u_Local3.b;

		if (u_lightCount > 0.0)
		{
			vec3 addedLight = vec3(0.0);

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightPos = u_lightPositions2[li].xyz;
				float lightDist = distance(lightPos, position.xyz);
				float lightPower = lightDist / 512.0;

				if (lightPower > 1.0) continue;

				float pDist = distance(lightPos.xyz, u_ViewOrigin.xyz);
				float viewDist = distance(position.xyz, u_ViewOrigin.xyz);

				if (viewDist > pDist) continue;

				lightPower = 1.0 - clamp(lightPower, 0.0, 1.0);

				vec3 bounceDir = reflect(surfaceToCamera, N);
				vec3 lightColor = u_lightColors[li].rgb;

				if (length(lightColor) <= 0.01)
				{
					lightColor = vec3(1.0);
				}

				vec3 lightDir = normalize(lightPos - position.xyz);

				vec3 crs = cross(bounceDir, lightDir);
				//float a = dot(bounceDir, lightDir);
				//float a = distance(bounceDir, lightDir);
				float a = length(crs);

				if (a < u_Local3.a && rand(crs.yz) < 0.02)
				{
					addedLight += lightColor * lightPower * shinyness * a * rand(crs.xy);
				}
			}

			gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + clamp(addedLight * shinyness * lightScale, 0.0, 1.0), shinyness * lightScale);
		}
	}
#endif //__EMISSION__
}


#define __AMBIENT_OCCLUSION__
#define __FOLIAGE_VIBRANCY__
#define __ENVMAP__
//#define __CURVE__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;
uniform sampler2D	u_ScreenDepthMap;
uniform samplerCube	u_CubeMap;
uniform sampler2D	u_HeightMap;
uniform sampler2D	u_DetailMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif //defined(USE_SHADOWMAP)

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, r_ao, r_env
uniform vec4		u_Local2; // SSDO, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov

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


#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__CURVE__)
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
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__CURVE__)

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

#ifdef __CURVE__
float curve(in vec3 p, in float w)
{
    vec2 e = vec2(-1., 1.)*w;
    
    float t1 = map(p + e.yxx), t2 = map(p + e.xxy);
    float t3 = map(p + e.xyx), t4 = map(p + e.yyy);
    
    return 0.125/(w*w) *(t1 + t2 + t3 + t4 - 4.*map(p));
}
#endif //__CURVE__


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

	// Because rend2 tangents are all fucked - re-calculate them.
	//vec3 tangent = TangentFromNormal(norm);
	//vec3 bitangent = normalize(cross(norm, tangent));

	norm.rgb = norm.rgb * 2.0 - 1.0;
	vec4 normalDetail = normalVector(texCoords);
	normalDetail.rgb = normalDetail.rgb * 2.0 - 1.0;
	normalDetail.rgb *= 0.25;//u_Local3.r;
	normalDetail.z = sqrt(clamp((0.25 - normalDetail.x * normalDetail.x) - normalDetail.y * normalDetail.y, 0.0, 1.0));
	norm.rgb = normalize(norm.rgb + normalDetail.rgb);

	vec3 N = norm.xyz;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);

	float shadowMult = 1.0;

#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0)
	{
		float shadowValue = textureLod(u_ShadowMap, texCoords, 0.0).r;
		shadowValue = pow(shadowValue, 1.5/*u_Local3.r*/);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);

		gl_FragColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
		shadowMult = clamp(shadowValue, 0.2, 1.0);
	}
#endif //defined(USE_SHADOWMAP)

	float lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);

	vec3 surfaceToCamera = normalize(u_ViewOrigin.xyz - position.xyz);

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
#ifdef __OLD_SPECULAR__
	float lambertian2 = dot(PrimaryLightDir.xyz, N);
	float spec2 = 0.0;
#endif //__OLD_SPECULAR__
	bool noSunPhong = false;
	float phongFactor = u_Local1.r;

	if (phongFactor < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
		noSunPhong = true;
		phongFactor = 0.0;
	}

	vec4 occlusion = vec4(0.0);
	vec3 norm2 = vec3(0.0);
	bool useOcclusion = false;

	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
		norm2 = texture(u_DetailMap, texCoords).xyz * 2.0 - 1.0;
	}

	float reflectivePower = (norm.a * 0.2);

#ifdef __OLD_SPECULAR__
	if (!noSunPhong && lambertian2 > 0.0 && shadowMult > 0.0)
#else
	if (!noSunPhong && shadowMult > 0.0)
#endif
	{// this is blinn phong
#ifdef __OLD_SPECULAR__
		vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
		float specAngle = max(dot(halfDir2, N), 0.0);
		spec2 = pow(specAngle, 16.0);
		vec3 lightAdd = (vec3(clamp(spec2, 0.0, 1.0) * reflectivePower) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor * 8.0 * u_Local1.g) * lightScale;
		gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + (lightAdd * shadowMult), lightScale);
#else //!__OLD_SPECULAR__
		vec3 lightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
		float diffuseCoefficient = max(0.0, dot(N, lightDir));

		if (diffuseCoefficient > 0.0)
		{
			vec3 specular = u_PrimaryLightColor.rgb * gl_FragColor.rgb * diffuseCoefficient * u_Local1.g;

			//final color (after gamma correction)
			vec3 gamma = vec3(1.0/2.2);
			vec3 finalColor = pow(specular, gamma);

			vec3 lightAdd = clamp(finalColor * lightScale * reflectivePower * phongFactor * 4.0, 0.0, 1.0);

			if (useOcclusion)
			{
				vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
				float to_light_dist = length(to_light);
				vec3 to_light_norm = (to_light / to_light_dist);
				float light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm*E, 1.0), occlusion), 0.0, 1.0);

				gl_FragColor.rgb = mix(gl_FragColor.rgb, (gl_FragColor.rgb * (light_occlusion * 0.3 + 0.66666)) + ((lightAdd * light_occlusion * shadowMult)), lightScale);
			}
			else
			{
				gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + (lightAdd * shadowMult), lightScale);
			}
		}
#ifdef __FOLIAGE_VIBRANCY__
		else if (position.a == 19.0 || position.a == 20.0)
		{// Leaves and foliages subsurface scatter...
			lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);

			float diffuseCoefficient = max(0.0, dot(N, -lightDir));

			if (diffuseCoefficient > 0.0)
			{
				vec3 specular = u_PrimaryLightColor.rgb * gl_FragColor.rgb * diffuseCoefficient * u_Local1.g;

				//final color (after gamma correction)
				vec3 gamma = vec3(1.0/2.2);
				vec3 finalColor = pow(specular, gamma);

				vec3 lightAdd = clamp(finalColor * lightScale * reflectivePower * phongFactor * 6.0, 0.0, 1.0);

				if (useOcclusion)
				{
					vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
					float to_light_dist = length(to_light);
					vec3 to_light_norm = (to_light / to_light_dist);
					float light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm*E, 1.0), occlusion), 0.0, 1.0);

					gl_FragColor.rgb = mix(gl_FragColor.rgb, (gl_FragColor.rgb * (light_occlusion * 0.3 + 0.66666)) + ((lightAdd * light_occlusion * shadowMult)), lightScale);
				}
				else
				{
					gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + (lightAdd * shadowMult), lightScale);
				}
			}
		}
#endif //__FOLIAGE_VIBRANCY__
#endif //__OLD_SPECULAR__
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
		phongFactor = -u_Local1.r;
	}

	if (u_lightCount > 0.0)
	{
		lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale), 0.2, 1.0);

		vec3 addedLight = vec3(0.0);

		for (int li = 0; li < u_lightCount/*MAX_DEFERRED_LIGHTS*/; li++)
		{
			//if (li > u_lightCount) break;

			vec3 lightPos = u_lightPositions2[li].xyz;

			float lightDist = distance(lightPos, position.xyz);

			if (u_lightHeightScales[li] > 0.0)
			{// ignore height differences, check later...
				lightDist -= length(lightPos.z - position.z);
			}

			float lightMax = u_lightDistances[li];

			float lightDistMult = clamp(1.0 - (distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0);

			if (lightDist < lightMax && lightDistMult > 0.0)
			{
				/*if (u_lightHeightScales[li] > 0.0)
				{// Check height difference...
					if (length(lightPos.z - position.z) > lightMax * u_lightHeightScales[li])
					{// Out of height range...
						continue;
					}
				}*/

				float lightStrength = clamp(1.0 - clamp(lightDist / lightMax, 0.0, 1.0), 0.0, 1.0);;
				lightStrength = pow(lightStrength, 4.0);

				if (lightStrength > 0.01)
				{
					vec3 lightColor = u_lightColors[li].rgb;
					float lightColorLength = length(lightColor) / 3.0;

					if (lightColorLength > LIGHT_THRESHOLD)
					{
						vec3 lightDir = normalize(lightPos - position.xyz);

						// Add some basic light...
						vec3 ambientLight = lightColor * lightStrength * lightScale * 0.25;
						vec3 ambient = ambientLight; // Always add some basic light...

						float diffuseCoefficient = max(0.0, dot(N, lightDir));
						
						if (diffuseCoefficient > 0.0)
						{
							vec3 diffuseLight = lightColor * lightStrength * lightScale * gl_FragColor.rgb * 8.0;
							vec3 diffuse = diffuseLight * diffuseCoefficient; // Always add some basic diffuse light...

							//float specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-lightDir, N))), 1.0/*materialShininess*/);

							//vec3 specular = specularCoefficient * lightColor;
							vec3 specular = diffuseCoefficient * lightColor * gl_FragColor.rgb;
							specular = specular * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 0.5 * reflectivePower * phongFactor * 8.0;

							//attenuation
							float distanceToLight = length(lightPos - position.xyz);
							float attenuation = 1.0 / (1.0 + reflectivePower * pow(distanceToLight, 2.0));

							//linear color (color before gamma correction)
							vec3 linearColor = ambient + attenuation*(diffuse + specular);
    
							//final color (after gamma correction)
							vec3 gamma = vec3(1.0/2.2);
							vec3 finalColor = pow(linearColor, gamma);

							vec3 lightAdd = finalColor * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 48.0 * lightDistMult;

							if (useOcclusion)
							{
								vec3 to_light = position.xyz - lightPos;
								float to_light_dist = distanceToLight;
								vec3 to_light_norm = to_light / to_light_dist;
								float light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm+E, 1.0), occlusion), 0.0, 1.0);
								addedLight.rgb += lightAdd * (light_occlusion * 0.75 - 0.05) * invLightScale;
							}
							else
							{
								addedLight += (lightAdd * invLightScale);
							}
						}
#ifdef __FOLIAGE_VIBRANCY__
						else if (position.a == 19.0 || position.a == 20.0)
						{// Leaves and foliages subsurface scatter...
							diffuseCoefficient = max(0.0, dot(N, -lightDir));

							if (diffuseCoefficient > 0.0)
							{
								vec3 diffuseLight = lightColor * lightStrength * lightScale * gl_FragColor.rgb * 8.0;
								vec3 diffuse = diffuseLight * diffuseCoefficient; // Always add some basic diffuse light...

								vec3 specular = lightColor.rgb * gl_FragColor.rgb * diffuseCoefficient * u_Local1.g;

								//attenuation
								float distanceToLight = length(lightPos - position.xyz);
								float attenuation = 1.0 / (1.0 + reflectivePower * pow(distanceToLight, 2.0));

								//linear color (color before gamma correction)
								vec3 linearColor = ambient + attenuation*(diffuse + specular);
    
								//final color (after gamma correction)
								vec3 gamma = vec3(1.0/2.2);
								vec3 finalColor = pow(linearColor, gamma);

								vec3 lightAdd = finalColor * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 48.0 * lightDistMult * 0.5;

								if (useOcclusion)
								{
									vec3 to_light = position.xyz - lightPos;
									float to_light_dist = distanceToLight;
									vec3 to_light_norm = to_light / to_light_dist;
									float light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm+E, 1.0), occlusion), 0.0, 1.0);
									addedLight.rgb += lightAdd * (light_occlusion * 0.75 - 0.05) * invLightScale;
								}
								else
								{
									addedLight += (lightAdd * invLightScale);
								}
							}
						}
#endif //__FOLIAGE_VIBRANCY__
					}
				}
			}
		}

		gl_FragColor.rgb = clamp(gl_FragColor.rgb + (clamp(addedLight, -1.0, 1.0) * lightScale), 0.0, 1.0);
	}

#if defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__CURVE__)
	vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
	float to_light_dist = length(to_light);
	vec3 to_light_norm = (to_light / to_light_dist);

//#define rd (to_light_norm + surfaceToCamera)
#define rd reflect(surfaceToCamera, N)
#endif //defined(__AMBIENT_OCCLUSION__) || defined(__ENVMAP__) || defined(__CURVE__)

#ifdef __AMBIENT_OCCLUSION__
	if (u_Local1.b > 0.0)
	{
		float ao = calculateAO(to_light_norm, N * 10000.0);
		
		ao = clamp(ao, 0.0, 1.0);

		gl_FragColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __FOLIAGE_VIBRANCY__
	if (position.a == 1.0 || position.a == 2.0 || position.a == 19.0 || position.a == 20.0 || position.a == 5.0 || position.a == 6.0)
	{
		vec4 color = gl_FragColor;

		// Contrast...
//#define glowLower ( 16.0 / 255.0 )
//#define glowUpper (255.0 / 250.0 )
//		color.rgb = clamp((clamp(color.rgb - glowLower, 0.0, 1.0)) * glowUpper, 0.0, 1.0);

#define FOLIAGE_VIBRANCY 0.4

		// Vibrancy...
		vec3	lumCoeff = vec3(0.212656, 0.715158, 0.072186);  				//Calculate luma with these values
		float	max_color = max(color.r, max(color.g, color.b)); 				//Find the strongest color
		float	min_color = min(color.r, min(color.g, color.b)); 				//Find the weakest color
		float	color_saturation = max_color - min_color; 						//Saturation is the difference between min and max
		float	luma = dot(lumCoeff, color.rgb); 								//Calculate luma (grey)
		color.rgb = mix(vec3(luma), color.rgb, (1.0 + (FOLIAGE_VIBRANCY * (1.0 - (sign(FOLIAGE_VIBRANCY) * color_saturation))))); 	//Extrapolate between luma and original by 1 + (1-saturation) - current
		gl_FragColor.rgb = color.rgb;
	}
#endif //__FOLIAGE_VIBRANCY__

#ifdef __ENVMAP__
	if (u_Local1.a > 0.0)
	{
		lightScale = clamp((1.0 - max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b)), 0.0, 1.0);
		float invLightScale = clamp((1.0 - lightScale), 0.2, 1.0);

		vec3 env = envMap(rd, 0.6 /* warmth */);//.5;
		gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + ((env * (norm.a * 0.5) * invLightScale) * lightScale), (norm.a * 0.5) * lightScale);
	}
#endif //__ENVMAP__

#ifdef __CURVE__
	if (u_Local3.r > 0.0)
	{
		// Curvature.
		float crv = clamp(curve((position.xyz / 512000.0)/*rd*//*to_light_norm*/ * u_Local3.b, 0.125)*0.5+0.5, .0, 1.);
	    
    	// Darkening the crevices. Otherse known as cheap, scientifically-incorrect shadowing.	
		float shading =  crv*0.5+0.5; //smoothstep(-.05, .1, cellTile(to_light_norm));//
		shading *= smoothstep(-.1, .15, cellTile(to_light_norm));
			
		if (u_Local3.g > 0.0)
			gl_FragColor.rgb = vec3(shading);
		else
			gl_FragColor.rgb *= shading;
	}
#endif //__CURVE__

	//gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
}


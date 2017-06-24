//#define __AMBIENT_OCCLUSION__

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

uniform vec4		u_Local1; // r_blinnPhong, SUN_PHONG_SCALE, 0, 0
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


#ifdef __AMBIENT_OCCLUSION__
float drawObject(in vec3 p){
    p = abs(fract(p)-.5);
    return dot(p, vec3(.5));
}

float cellTile(in vec3 p){
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

float map(vec3 p){
    float n = (.5-cellTile(p))*1.5;
    return p.y + dot(sin(p/2. + cos(p.yzx/2. + 3.14159/2.)), vec3(.5)) + n;
}

float calculateAO(in vec3 pos, in vec3 nor)
{
	float sca = 2.0, occ = 0.0;
    for( int i=0; i<5; i++ ){
    
        float hr = 0.01 + float(i)*0.5/4.0;        
        float dd = map(nor * hr + pos);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - occ, 0.0, 1.0 );    
}
#endif //__AMBIENT_OCCLUSION__


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

	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	vec3 N = norm.xyz;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);

	norm.a = clamp(length(gl_FragColor.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	norm.a = clamp((clamp(norm.a - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);


#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0)
	{
		float shadowValue = textureLod(u_ShadowMap, texCoords, 0.0).r;
		shadowValue = pow(shadowValue, 1.5/*u_Local3.r*/);

#define sm_cont_1 ( 64.0 / 255.0)
#define sm_cont_2 (255.0 / 200.0)
		shadowValue = clamp((clamp(shadowValue - sm_cont_1, 0.0, 1.0)) * sm_cont_2, 0.0, 1.0);

		gl_FragColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a);
	}
#endif //defined(USE_SHADOWMAP)

	float lightScale = clamp((1.0 - max(max(color.r, color.g), color.b)) - 0.2, 0.0, 1.0);

	vec3 surfaceToCamera = normalize(u_ViewOrigin.xyz - position.xyz);

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	float lambertian2 = dot(PrimaryLightDir.xyz, N);
	float spec2 = 0.0;
	bool noSunPhong = false;
	float phongFactor = u_Local1.r;

	if (phongFactor < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
		noSunPhong = true;
		phongFactor = 0.0;
	}

	float normStrength = (norm.a * 0.5 + 0.5) * 0.2;//u_Local3.r;

	vec4 occlusion = vec4(0.0);
	vec3 norm2 = vec3(0.0);
	bool useOcclusion = false;

	if (u_Local2.r == 1.0)
	{
		useOcclusion = true;
		occlusion = texture(u_HeightMap, texCoords);
		norm2 = texture(u_DetailMap, texCoords).xyz * 2.0 - 1.0;
	}

	if (!noSunPhong && lambertian2 > 0.0)
	{// this is blinn phong
		vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
		float specAngle = max(dot(halfDir2, N), 0.0);
		spec2 = pow(specAngle, 16.0);
		vec3 lightAdd = (vec3(clamp(spec2, 0.0, 1.0) * normStrength) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor * 8.0 * u_Local1.g) * lightScale;

		if (useOcclusion)
		{
			vec3 to_light = position.xyz - u_PrimaryLightOrigin.xyz;
			float to_light_dist = length(to_light);
			vec3 to_light_norm = (to_light / to_light_dist);
			float light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm*E, 1.0), occlusion), 0.0, 1.0);

			gl_FragColor.rgb = (gl_FragColor.rgb * (light_occlusion * 0.3 + 0.66666)) + (lightAdd * light_occlusion);
		}
		else
		{
			gl_FragColor.rgb += lightAdd;
		}
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
		phongFactor = -u_Local1.r;
	}

	if (u_lightCount > 0.0)
	{
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

			if (lightDist < lightMax)
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
						vec3 ambientLight = lightColor * lightStrength * lightScale * 0.5;
						vec3 ambient = ambientLight; // Always add some basic light...

						vec3 diffuseLight = lightColor * lightStrength * lightScale * gl_FragColor.rgb * 8.0;
						float diffuseCoefficient = max(0.0, dot(N, lightDir));
						vec3 diffuse = diffuseLight * diffuseCoefficient; // Always add some basic diffuse light...
						
						// Specular...
						float specularCoefficient = 0.0;
						if(diffuseCoefficient > 0.0)
							specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-lightDir, N))), 1.0/*materialShininess*/);
						vec3 specular = specularCoefficient * lightColor;
						specular = specular * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 0.5 * (norm.a * 0.5 + 0.5) * phongFactor * 8.0;

						//attenuation
						float distanceToLight = length(lightPos - position.xyz);
						float attenuation = 1.0 / (1.0 + (norm.a * 0.5 + 0.5) * pow(distanceToLight, 2.0));

						//linear color (color before gamma correction)
						vec3 linearColor = ambient + attenuation*(diffuse + specular);
    
						//final color (after gamma correction)
						vec3 gamma = vec3(1.0/2.2);
						vec3 finalColor = pow(linearColor, gamma);

						vec3 lightAdd = finalColor * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 48.0;

						if (useOcclusion)
						{
							vec3 to_light = position.xyz - lightPos;
							float to_light_dist = distanceToLight;
							vec3 to_light_norm = to_light / to_light_dist;
							float light_occlusion = 1.0 - clamp(dot(vec4(-to_light_norm+E, 1.0), occlusion), 0.0, 1.0);
							addedLight.rgb += lightAdd * (light_occlusion * 0.75 - 0.05);
						}
						else
						{
							addedLight += lightAdd;
						}
					}
				}
			}
		}

		gl_FragColor.rgb = clamp(gl_FragColor.rgb + clamp(addedLight, -1.0, 1.0) * lightScale, 0.0, 1.0);
	}

#ifdef __AMBIENT_OCCLUSION__
	//if (u_Local2.g >= 1.0)
	//if (position.a != 0.0 && position.a != 1024.0 && position.a != 1025.0)
	{
		float ao = calculateAO((position.xyz / 524288.0) * 0.5 + 0.5, -N.xyz);

		/*
		ao = clamp(ao * 0.1 + 0.9, 0.0, 1.0);
		float ao2 = clamp(ao + 0.95, 0.0, 1.0);
		ao = (ao + ao2) / 2.0;
		//ao *= ao;
		ao = pow(ao, 4.0);
		*/

		if (u_Local3.r > 0.0)
			gl_FragColor.rgb = vec3(ao);
		else
			gl_FragColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

	//gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
}


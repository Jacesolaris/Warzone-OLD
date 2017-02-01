uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif //defined(USE_SHADOWMAP)

uniform vec2		u_Dimensions;

uniform vec4		u_Local2;

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

#define MAX_LIGHTALL_DLIGHTS 16//24

uniform int			u_lightCount;
uniform vec2		u_lightPositions[MAX_LIGHTALL_DLIGHTS];
uniform vec3		u_lightPositions2[MAX_LIGHTALL_DLIGHTS];
uniform float		u_lightDistances[MAX_LIGHTALL_DLIGHTS];
uniform vec3		u_lightColors[MAX_LIGHTALL_DLIGHTS];

varying vec2		var_TexCoords;


#define LIGHT_THRESHOLD  0.01//0.001
#define LIGHT_COLOR_SEARCH

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

vec3 GlowAtPosition ( vec2 coord )
{// Since dlight positions are not accurate in JKA/Q3... Scan a little around it's origin for a better color...
	vec3 color = texture( u_GlowMap, coord ).rgb;
	float colLen = length(color);
	vec2 px = vec2(1.0) / u_Dimensions.xy;

	for (float x = -2.0; x <= 2.0; x += 1.0)
	{
		for (float y = -2.0; y <= 2.0; y += 1.0)
		{
			vec3 newcolor = texture( u_GlowMap, coord + (px * (4.0 * vec2(x, y))) ).rgb;
			float ncLen = length(newcolor);

			if (ncLen > colLen)
			{
				color = newcolor;
				colLen = ncLen;
			}
		}
	}

	return color;
}

vec4 ConvertToNormals ( vec4 color )
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	vec3 color2 = color.rgb;

	vec3 N = vec3(clamp(color2.r + color2.b, 0.0, 1.0), clamp(color2.g + color2.b, 0.0, 1.0), clamp(color2.r + color2.g, 0.0, 1.0));

	vec3 brightness = color2.rgb; //adaptation luminance
	brightness = (brightness/(brightness+1.0));
	brightness = vec3(max(brightness.x, max(brightness.y, brightness.z)));
	vec3 brightnessMult = (vec3(1.0) - brightness) * 0.5;

	color2 = pow(clamp(color2 + brightnessMult, 0.0, 1.0), vec3(2.0));

	N.xy = 1.0 - N.xy;
	N.xyz = N.xyz * 0.5 + 0.5;
	N.xyz = pow(N.xyz, vec3(2.0));
	N.xyz *= 0.8;

	//float displacement = brightness.r;
	float displacement = clamp(length(color.rgb), 0.0, 1.0);
	//float displacement = clamp(length(color2.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	displacement = clamp((clamp(displacement - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

	vec4 norm = vec4(N, displacement);

	if (norm.g > norm.b)
	{// Switch them for screen space fakes...
		norm.gb = norm.bg;
	}

	return norm;
}

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
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);
	gl_FragColor = vec4(color.rgb, 1.0);

	/*{
		vec3 lightColor = texture( u_GlowMap, var_TexCoords ).rgb;
		gl_FragColor.rgb = lightColor;
		return;
	}*/

	/*{
		vec4 position = texture2D(u_PositionMap, var_TexCoords);
		float dist = distance(position.xyz, u_ViewOrigin.xyz);
		dist = clamp(dist / 4096.0, 0.0, 1.0);
		gl_FragColor.rgb = vec3(dist);
		return;
	}*/

#if defined(USE_SHADOWMAP)
	float shadowValue = texture(u_ShadowMap, var_TexCoords/*gl_FragCoord.xy * r_FBufScale*/).r;
	//gl_FragColor.rgb *= clamp(shadowValue, 0.4, 1.0);
	gl_FragColor.rgb *= clamp(shadowValue, 0.85, 1.0);
#endif //defined(USE_SHADOWMAP)

	vec3 viewOrg = u_ViewOrigin.xyz;
	vec4 position = texture2D(u_PositionMap, var_TexCoords);

	if (position.a == 1024.0 || position.a == 1025.0)
	{// Skybox... Skip...
		//gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
		return;
	}

	/*if (position.a == 0.0 && position.xyz == vec3(0.0))
	{// Unknown... Skip...
		//gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), 1.0);
		return;
	}*/

	vec4 norm = texture2D(u_NormalMap, var_TexCoords);

	/*if (position.a != 0.0 && position.a != 1024.0 && position.a != 1025.0 && norm.a == 0.05)
	{// Generic GLSL. Probably a glow or something, ignore the lighting...
		return;
	}*/

	if (norm.a < 0.05 || length(norm.xyz) <= 0.05)
	{
		norm = ConvertToNormals(color);
	}

	norm.a = norm.a * 0.5 + 0.5;
	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	vec3 N = norm.xyz;

	//gl_FragColor = vec4(N.xyz * 0.5 + 0.5, 1.0);
	//return;

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	float lambertian2 = dot(PrimaryLightDir.xyz, N);
	float spec2 = 0.0;
	bool noSunPhong = false;
	float phongFactor = u_Local2.r;

	if (phongFactor < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
		noSunPhong = true;
		phongFactor = 0.0;
	}


	if (!noSunPhong && lambertian2 > 0.0)
	{// this is blinn phong
		vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
		float specAngle = max(dot(halfDir2, N), 0.0);
		spec2 = pow(specAngle, 16.0);
		gl_FragColor.rgb += vec3(spec2 * (1.0 - norm.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor;
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
		phongFactor = -u_Local2.r;
	}

	if (u_lightCount > 0.0)
	{
		vec3 addedLight = vec3(0.0);

		for (int li = 0; li < u_lightCount; li++)
		{
			/*{
				if (distance(u_lightPositions[li], var_TexCoords) < 0.1)
				{
					//gl_FragColor.rgb = vec3(0.0, 0.0, 1.0);
					//return;
					vec3 lightColor = GlowAtPosition(u_lightPositions[li]);
					addedLight += lightColor;
					continue;
				}
			}*/

			vec3 lightPos = u_lightPositions2[li].xyz;

			float lightDist = distance(lightPos, position.xyz);
			float lightMax = u_lightDistances[li];

			if (lightDist < lightMax)
			{
				float lightStrength = clamp(1.0 - (lightDist / lightMax), 0.0, 1.0);
				lightStrength = pow(lightStrength, 2.0) * 0.1;

				if (lightStrength > LIGHT_THRESHOLD)
				{
#ifndef LIGHT_COLOR_SEARCH
					vec3 lightColor = u_lightColors[li].rgb;
#else //LIGHT_COLOR_SEARCH
					vec3 lightColor = GlowAtPosition(u_lightPositions[li]);
#endif //LIGHT_COLOR_SEARCH
					float lightColorLength = length(lightColor);

					if (lightColorLength > LIGHT_THRESHOLD)
					{
						// Try to maximize light strengths...
						float mult = 3.0 / lightColorLength;
						lightColor += lightColor * mult;

						// Add some basic light...
						addedLight += lightColor * lightStrength; // Always add some basic light...

						vec3 lightDir = normalize(lightPos - position.xyz);
						float lambertian3 = dot(lightDir.xyz, N);

						if (lambertian3 > 0.0)
						{// this is blinn phong
							// Diffuse...
							addedLight += (lightColor * lightStrength) * gl_FragColor.rgb * lambertian3;

							// Specular...
							vec3 halfDir3 = normalize(lightDir.xyz + E);
							float specAngle3 = max(dot(halfDir3, N), 0.0);
							float spec3 = pow(specAngle3, 16.0);

							float strength = ((1.0 - spec3) * (1.0 - norm.a)) * lightStrength * 5.0;//phongFactor;
							addedLight +=  lightColor * strength * 0.5;
						}
					}
				}
			}
		}

		if (length(addedLight) > 0.0)
		{
			gl_FragColor.rgb += clamp(addedLight * 0.22/*u_Local2.g*//*0.1*/, 0.0, 1.0);
			gl_FragColor.rgb = clamp(gl_FragColor.rgb, 0.0, 1.0);
		}
	}

	//if (u_Local2.g >= 1.0)
	//if (position.a != 0.0 && position.a != 1024.0 && position.a != 1025.0)
	{
		float ao = calculateAO(position.xyz / 524288.0, -N.xyz);

		ao = clamp(ao * 0.1 + 0.9, 0.0, 1.0);
		float ao2 = clamp(ao + 0.95, 0.0, 1.0);
		ao = (ao + ao2) / 2.0;
		//ao *= ao;
		ao = pow(ao, 4.0);

		gl_FragColor.rgb *= ao;
	}

	//gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
}


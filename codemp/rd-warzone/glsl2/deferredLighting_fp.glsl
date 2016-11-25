uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_DeluxeMap;

uniform vec2		u_Dimensions;

uniform vec4		u_Local2;

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform int			u_lightCount;
uniform vec3		u_lightPositions2[16];
uniform float		u_lightDistances[16];
uniform vec3		u_lightColors[16];

varying vec2		var_TexCoords;

#define FULL_LIGHTING


#define unOpenGlIsFuckedUpify(x) ( x / 524288.0 )

#if defined(FULL_LIGHTING)

//
// Full lighting... Blinn phong and basic lighting as well...
//

vec4 ConvertToNormals(vec4 color)
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	vec3 color2 = color.rgb;

	vec3 N = vec3(clamp(color2.r + color2.b, 0.0, 1.0), clamp(color2.g + color2.b, 0.0, 1.0), clamp(color2.r + color2.g, 0.0, 1.0));

	vec3 brightness = color2.rgb; //adaptation luminance
	brightness = (brightness / (brightness + 1.0));
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
	return norm;
}

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

	// GLSL distance() can't work with large numbers?!?!??!?!?!!??
	highp vec3 viewOrg = unOpenGlIsFuckedUpify(abs(u_ViewOrigin.xyz));
	highp vec4 position = abs(texture2D(u_PositionMap, var_TexCoords));

	if (position.a == 1024.0 || position.a == 1025.0)
	{// Skybox... Skip...
		//gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
		return;
	}

	if (position.a == 0.0 && position.xyz == vec3(0.0))
	{// Unknown... Skip...
		//gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), 1.0);
		return;
	}

	vec4 norm = texture2D(u_NormalMap, var_TexCoords);

#if 0
	float normPower = length(norm.rgb);

	//if (normPower < 0.5 || normPower > 2.5)
	if (normPower == 0.0)
	{// Fallback... Generate some fake normals...
		norm = ConvertToNormals(color);
		/*norm.rgb = norm.rgb * 0.5 + 0.5;
		float t = norm.b;
		norm.b = norm.g;
		norm.g = t;
		norm.rgb = normalize(norm.rgb);*/
	}
#endif

	/*if (u_Local2.a == 1.0)
	{
		gl_FragColor = vec4(normalize(norm.rgb), 1.0);
		return;
	}*/

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
		gl_FragColor.rgb += vec3(spec2 * (1.0 - norm.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor * 0.3;
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
		phongFactor = -u_Local2.r;
	}

	if (u_lightCount > 0.0)
	{
		float addedStrength = 1.0;
		vec3 addedLight = vec3(0.0);

		for (int li = 0; li < u_lightCount /*min(u_lightCount, 1)*/; li++)
		{
			vec3 lightPos = unOpenGlIsFuckedUpify(abs(u_lightPositions2[li].xyz));

			float lightDist = distance(lightPos, position.xyz);
			float lightMax = unOpenGlIsFuckedUpify(u_lightDistances[li]) * 1.5;

			if (lightDist < lightMax)
			{
				highp float lightStrength = 1.0 - (lightDist / lightMax);
				lightStrength = clamp(pow(lightStrength * 0.9, 3.0), 0.0, 1.0);

				if (lightStrength > 0.0)
				{
					highp float lightBrightness = length(u_lightColors[li].rgb);
					//if (lightBrightness > 2.0) lightStrength /= 3.0;
					//else if (lightBrightness > 1.0) lightStrength /= 2.0;

					highp float strength = lightStrength;// *u_Local2.g;
					addedStrength += strength;
					addedLight += u_lightColors[li].rgb * strength; // Always add some basic light...

					vec3 lightDir = normalize(lightPos - position.xyz);
					float lambertian3 = dot(lightDir.xyz, N);

					if (lambertian3 > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, N), 0.0);
						float spec3 = pow(specAngle3, 16.0);

						strength = ((1.0 - spec3) * (1.0 - norm.a)) * lightStrength * phongFactor;
						addedStrength += strength;
						addedLight += strength * u_lightColors[li].rgb;
					}
				}
				//else
				//	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), 1.0);
			}
			//else
			//	gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
		}

		if (addedStrength > 1.0)
		{
			highp vec3 power = (addedLight / addedStrength) * 2.3;// u_Local2.g;
			gl_FragColor.rgb += (power * 0.1) + (power * 0.9 * gl_FragColor.rgb);
		}
	}

	//gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
}

#else //!defined(FULL_LIGHTING)

//
// Fast lighting... No blinn phong (or sun) lighting...
//

void main(void)
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);
	gl_FragColor = vec4(color.rgb, 1.0);

	// GLSL distance() can't work with large numbers?!?!??!?!?!!??
	highp vec3 viewOrg = unOpenGlIsFuckedUpify(abs(u_ViewOrigin.xyz));
	highp vec4 position = abs(texture2D(u_PositionMap, var_TexCoords));
	//position.xyz = unOpenGlIsFuckedUpify(position.xyz);

	//gl_FragColor = vec4(clamp(vec3(distance(viewOrg, position.xyz) / (unOpenGlIsFuckedUpify(2048.0))), 0.0, 1.0), 1.0);
	//return;

	if (position.a == 1024.0)
	{// Skybox... Skip...
		//gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
		return;
	}

	if (position.a == 0.0 && position.xyz == vec3(0.0))
	{// Unknown... Skip...
		//gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), 1.0);
		return;
	}

	if (u_lightCount > 0.0)
	{
		float addedStrength = 1.0;
		vec3 addedLight = vec3(0.0);

		for (int li = 0; li < u_lightCount; li++)
		{
			float lightDist = distance(unOpenGlIsFuckedUpify(abs(u_lightPositions2[li].xyz)), position.xyz);
			float lightMax = unOpenGlIsFuckedUpify(u_lightDistances[li]) * 1.5;

			if (lightDist < lightMax)
			{
				highp float lightStrength = 1.0 - (lightDist / lightMax);
				lightStrength = clamp(pow(lightStrength * 0.9, 3.0), 0.0, 1.0);

				if (lightStrength > 0.0)
				{
					highp float lightBrightness = length(u_lightColors[li].rgb);
					if (lightBrightness > 2.0) lightStrength /= 3.0;
					else if (lightBrightness > 1.0) lightStrength /= 2.0;

					highp float strength = lightStrength;// *u_Local2.g;
					addedStrength += strength;
					addedLight += u_lightColors[li].rgb * strength; // Always add some basic light...
				}
			}
		}

		if (addedStrength > 1.0)
		{
			highp vec3 power = (addedLight / addedStrength) * 2.3;// u_Local2.g;
			gl_FragColor.rgb += (power * 0.1) + (power * 0.9 * gl_FragColor.rgb);
		}
	}
}

#endif //defined(FULL_LIGHTING)

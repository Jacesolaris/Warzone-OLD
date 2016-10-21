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

void main(void)
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);
	gl_FragColor = vec4(color.rgb, 1.0);

	vec4 position = texture2D(u_PositionMap, var_TexCoords);

	if (position.a == 1024.0)
	{// Skybox... Skip...
		return;
	}

	if (position.a == 0.0 && position.xyz == vec3(0.0))
	{// Unknown... Skip...
		return;
	}

	vec4 norm = texture2D(u_NormalMap, var_TexCoords);

	float normPower = length(norm.rgb);

	if (normPower < 0.5 || normPower > 2.5)
	{// Fallback... Generate some fake normals...
		vec4 norm = ConvertToNormals(color);
		norm.rgb = norm.rgb * 0.5 + 0.5;
		float t = norm.b;
		norm.b = norm.g;
		norm.g = t;
		norm.rgb = normalize(norm.rgb);
	}

	/*if (u_Local2.a == 1.0)
	{
		gl_FragColor = vec4(normalize(norm.rgb), 1.0);
		return;
	}*/

	norm.a = norm.a * 0.5 + 0.5;
	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	vec3 N = norm.xyz;

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
		for (int li = 0; li < u_lightCount; li++)
		{
			float lightDist = distance(u_lightPositions2[li], position.xyz);
			float lightMax = u_lightDistances[li] * 1.5;

			if (lightDist < lightMax)
			{
				float lightStrength = 1.0 - (lightDist / lightMax);
				lightStrength = pow(lightStrength * 0.9, 3.0);
				if (u_lightColors[li].r == u_lightColors[li].g && u_lightColors[li].r == u_lightColors[li].b) lightStrength *= 0.5; // Reduce true white strength...

				if (lightStrength > 0.0)
				{
					vec3 lightDir = normalize(u_lightPositions2[li] - position.xyz);
					float lambertian3 = dot(lightDir.xyz, N);

					gl_FragColor.rgb += u_lightColors[li].rgb * lightStrength * u_Local2.g; // Always add some basic light...

					if (lambertian3 > 0.0)
					{
						if (lightStrength > 0.0)
						{// this is blinn phong
							vec3 halfDir3 = normalize(lightDir.xyz + E);
							float specAngle3 = max(dot(halfDir3, N), 0.0);
							float spec3 = pow(specAngle3, 16.0);
							gl_FragColor.rgb += vec3((1.0 - spec3) * (1.0 - norm.a)) * u_lightColors[li].rgb * lightStrength * phongFactor;
						}
					}
				}
			}
		}
	}
}
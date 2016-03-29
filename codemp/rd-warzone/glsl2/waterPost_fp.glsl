uniform sampler2D	u_DiffuseMap; // Screen Image...
uniform sampler2D	u_SpecularMap; // Water Map...

uniform mat4		u_ModelViewProjectionMatrix;

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;

#define pw (1.0/u_Dimensions.x)
#define ph (1.0/u_Dimensions.y)

float linearize(float depth)
{
	return 1.0 / mix(u_ViewInfo.z, 1.0, depth);
}

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords.xy);
	vec4 aux = texture2D(u_SpecularMap, var_TexCoords.xy);

	if (aux.b <= 0.0)
	{
		gl_FragColor = clamp(color, 0.0, 1.0);
		return;
	}

	float upPos = var_TexCoords.y;
	float LAND_Y = var_TexCoords.y + ph * 3.0;

	for (float y = var_TexCoords.y; y < 1.0 && var_TexCoords.y + ((y - var_TexCoords.y) * 2.0) < 1.0; y += ph * 3.0)
	{
		float isWater = texture2D(u_SpecularMap, vec2(var_TexCoords.x, y)).b;

		if (isWater <= 0.0)
		{
			LAND_Y = y;
			break;
		}
	}

	upPos = clamp(var_TexCoords.y + ((LAND_Y - var_TexCoords.y) * 2.0), 0.0, 1.0);

	vec4 landColor = texture2D(u_DiffuseMap, vec2(var_TexCoords.x, upPos));
	landColor += texture2D(u_DiffuseMap, vec2(var_TexCoords.x + pw, upPos));
	landColor += texture2D(u_DiffuseMap, vec2(var_TexCoords.x - pw, upPos));
	landColor += texture2D(u_DiffuseMap, vec2(var_TexCoords.x, upPos + ph));
	landColor += texture2D(u_DiffuseMap, vec2(var_TexCoords.x, upPos - ph));
	landColor /= 5.0;

	color.rgb = mix(color.rgb, landColor.rgb, vec3(1.0 - upPos));

	gl_FragColor = clamp(color, 0.0, 1.0);
}

uniform sampler2D	u_DiffuseMap; // Screen Image...
uniform sampler2D	u_ScreenDepthMap; // Depth Map...
uniform sampler2D	u_NormalMap; // Water Map...

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
	//gl_FragColor = texture2D(u_NormalMap, var_TexCoords);
	//gl_FragColor = vec4(vec3(linearize(texture2D(u_ScreenDepthMap, var_TexCoords).r)), 1.0);
	//return;

	vec4 color = texture2D(u_DiffuseMap, var_TexCoords.xy);
	vec4 aux = texture2D(u_NormalMap, var_TexCoords.xy);

	if (aux.b <= 0.0)
	{
		gl_FragColor = color;
		return;
	}

	float upPos = var_TexCoords.y;
	float LAND_Y = 0.0;
	//float waterDepth = linearize(texture2D(u_ScreenDepthMap, var_TexCoords.xy).r);

	for (float y = var_TexCoords.y; y < 1.0 && var_TexCoords.y + ((y - var_TexCoords.y) * 2.0) < 1.0; y += ph)
	{
		float isWater = texture2D(u_NormalMap, vec2(var_TexCoords.x, y)).b;
		upPos = var_TexCoords.y + ((y - var_TexCoords.y) * 2.0);
		//float landDepth = linearize(texture2D(u_ScreenDepthMap, vec2(var_TexCoords.x, upPos)).r);

		if (isWater <= 0.0 && 1.0 - upPos > 0.0 /*&& waterDepth < landDepth*/)
		{
			LAND_Y = y;
			break;
		}
	}

	if (LAND_Y <= 0.0)
	{
		//gl_FragColor = color;
		//return;
		LAND_Y = var_TexCoords.y + ph;
	}

	upPos = var_TexCoords.y + ((LAND_Y - var_TexCoords.y) * 2.0);

	//float isWater = texture2D(u_NormalMap, vec2(var_TexCoords.x, upPos)).b;

	//if (isWater > 0.0)
	//{
	//	gl_FragColor = color;
	//	return;
	//}

	vec4 landColor = texture2D(u_DiffuseMap, vec2(var_TexCoords.x, upPos));

	color.rgb = mix(color.rgb, landColor.rgb, vec3(1.0 - upPos));
	//color.rgb = mix(color.rgb, landColor.rgb, vec3(1.0 - var_TexCoords.y));
	//color.rgb += landColor.rgb + (upPos - var_TexCoords.y);

	gl_FragColor = color;
}

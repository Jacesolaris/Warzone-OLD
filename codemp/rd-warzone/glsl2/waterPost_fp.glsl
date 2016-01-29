uniform sampler2D	u_TextureMap; // Screen Image...
uniform sampler2D	u_ScreenDepthMap; // Depth Map...
uniform sampler2D	u_NormalMap; // Water Map...

uniform vec2		u_Dimensions;
uniform float		u_Time;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;
varying vec3		var_ViewDir;
varying vec3		var_position;
varying vec3		var_viewOrg;


float pw = 1.0/ u_Dimensions.x;
float ph = 1.0/ u_Dimensions.y;

float linearize(float depth)
{
	//return -u_ViewInfo.y * u_ViewInfo.x / (depth * (u_ViewInfo.y - u_ViewInfo.x) - u_ViewInfo.y);
	//return depth / 4.0;
	return 1.0 / mix(u_ViewInfo.z, 1.0, depth);

	//return pow(depth, 255.0);
}

void main()
{
	vec4 color = texture2D(u_TextureMap, var_TexCoords.xy);
	vec3 aux = texture2D(u_NormalMap, var_TexCoords.xy).rgb;
	float upPos = 0.0;

	if (aux.b > 0.0) {
		float LAND_Y = 0.0;
		float waterDepth = linearize(texture2D(u_ScreenDepthMap, var_TexCoords.xy).r);

		for (float y = var_TexCoords.y; y < 1.0 && var_TexCoords.y + ((y - var_TexCoords.y) * 2.0) < 1.0; y += ph)
		{
			float isWater = texture2D(u_NormalMap, vec2(var_TexCoords.x, y)).b;

			if (isWater <= 0.0)
			{
				upPos = var_TexCoords.y + ((y - var_TexCoords.y) * 2.0);

				if (1.0 - upPos > 0.0)
				{
					float landDepth = linearize(texture2D(u_ScreenDepthMap, vec2(var_TexCoords.x, upPos)).r);

					if (waterDepth < landDepth)
					{
						LAND_Y = y;
						break;
					}
				}
			}
		}

		if (LAND_Y != 0.0)
		{
			float isWater = texture2D(u_NormalMap, vec2(var_TexCoords.x, upPos)).b;

			if (isWater <= 0.0)
			{
				vec4 landColor = texture2D(u_TextureMap, vec2(var_TexCoords.x, upPos));

				color.rgb = mix(color.rgb, landColor.rgb, vec3(1.0 - upPos));
				//color.rgb += landColor.rgb + (upPos - var_TexCoords.y);
			}
		}
	}

	gl_FragColor = color.rgba;
}

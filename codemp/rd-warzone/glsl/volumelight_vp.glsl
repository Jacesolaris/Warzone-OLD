attribute vec3				attr_Position;
attribute vec2				attr_TexCoord0;

uniform sampler2D			u_GlowMap;
uniform sampler2D			u_ScreenDepthMap;

#define						MAX_VOLUMETRIC_LIGHTS 2//16

uniform mat4				u_ModelViewProjectionMatrix;
uniform vec4				u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID
uniform vec4				u_Local0;
uniform vec2				u_Dimensions;

uniform int					u_lightCount;
uniform vec2				u_vlightPositions[MAX_VOLUMETRIC_LIGHTS];
uniform float				u_vlightDistances[MAX_VOLUMETRIC_LIGHTS];

flat varying vec4			var_LightColor[MAX_VOLUMETRIC_LIGHTS];
varying vec2				var_TexCoords;

#define VOLUMETRIC_THRESHOLD	0.0//u_Local0.a//0.001 //u_Local0.b

float linearize(float depth)
{
	return (1.0 / mix(u_ViewInfo.z, 1.0, depth));
}

void main()
{
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_TexCoords = attr_TexCoord0.st;

	int  SUN_ID = int(u_ViewInfo.a);

#if 0
	vec2 pix = vec2(1.0) / u_Dimensions.xy;

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++)
	{
		var_LightColor[i].a = 0.0;

		if (i >= u_lightCount) break;

		vec3 glowColor = vec3(0.0);

		if (i == SUN_ID) 
		{
			var_LightColor[i].a = 1.0;
		}
		
		float depth = linearize(textureLod( u_ScreenDepthMap, u_vlightPositions[i], 0.0 ).r);

		if (depth * u_ViewInfo.g >= (u_vlightDistances[i] * u_ViewInfo.g) - 16.0)
		{
			var_LightColor[i].a = depth;
		}
		
		for (float x = -8.0; x <= 8.0; x += 2.0)
		{
			for (float y = -8.0; y <= 8.0; y += 2.0)
			{
				glowColor += textureLod( u_GlowMap, u_vlightPositions[i] + (pix * vec2(pow(x, 1.75), pow(y, 1.75))), 0.0 ).rgb;

				if (var_LightColor[i].a == 0.0)
				{// Scan for the light...
					depth = linearize(textureLod( u_ScreenDepthMap, u_vlightPositions[i] + (pix * vec2(pow(x, 1.75), pow(y, 1.75))), 0.0 ).r);

					if (depth * u_ViewInfo.g >= (u_vlightDistances[i] * u_ViewInfo.g) - 16.0)
					{
						var_LightColor[i].a = depth;
					}
				}
			}
		}

		glowColor /= 64.0;
		
		var_LightColor[i].rgb = glowColor.rgb;

		if (length(var_LightColor[i].rgb) <= VOLUMETRIC_THRESHOLD)
		{
			var_LightColor[i].a = 0.0;
		}
	}
#else
	var_LightColor[SUN_ID].a = 1.0;
#endif
}

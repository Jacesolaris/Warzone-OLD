attribute vec3			attr_Position;
attribute vec2			attr_TexCoord0;
uniform sampler2D		u_GlowMap;

#define					MAX_VOLUMETRIC_LIGHTS 16//64

uniform mat4			u_ModelViewProjectionMatrix;
uniform vec4			u_ViewInfo; // zmin, zmax, zmax / zmin, SUN_ID
uniform vec4			u_Local0;
uniform vec2			u_Dimensions;

uniform int				u_lightCount;
uniform vec2			u_vlightPositions[MAX_VOLUMETRIC_LIGHTS];

varying vec2			var_TexCoords;
flat varying highp vec3		var_LightColor[MAX_VOLUMETRIC_LIGHTS];

void main()
{
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	var_TexCoords = attr_TexCoord0.st;

	vec2 pix = vec2(1.0) / u_Dimensions.xy;//u_Local0.xy;

	for (int i = 0; i < MAX_VOLUMETRIC_LIGHTS; i++)
	{
		if (i >= u_lightCount) break;

		highp vec3 glowColor = vec3(0.0);
		
		for (float x = -4.0; x <= 4.0; x += 2.0)
		{
			for (float y = -4.0; y <= 4.0; y += 2.0)
			{
				glowColor += textureLod( u_GlowMap, u_vlightPositions[i] + (pix * vec2(pow(x, 3.5), pow(y, 3.5))), 0.0 ).rgb;
				//glowColor = max(glowColor, textureLod( u_GlowMap, u_vlightPositions[i] + (pix * vec2(pow(x, 7.0), pow(y, 7.0))), 0.0 ).rgb);
			}
		}

		glowColor /= 81.0;
		

		//highp vec3 glowColor = textureLod( u_GlowMap, u_vlightPositions[i], 0.0).rgb;
		var_LightColor[i] = glowColor.rgb;
	}
}

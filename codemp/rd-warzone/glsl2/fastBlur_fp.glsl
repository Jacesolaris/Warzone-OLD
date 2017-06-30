uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;
uniform vec4		u_Local0;

varying vec2		var_TexCoords;

#if defined(SHADOW_HQ)
#define BLUR_RADIUS 6.0
#define BLUR_PIXELMULT 1.0
#elif defined(SHADOW_MQ)
#define BLUR_RADIUS 4.0
#define BLUR_PIXELMULT 1.0
#else // SHADOW_LQ
#define BLUR_RADIUS 3.0
#define BLUR_PIXELMULT 1.0
#endif

#define px (1.0/u_Dimensions.x)*BLUR_PIXELMULT
#define py (1.0/u_Dimensions.y)*BLUR_PIXELMULT

#define RADIUS_X (BLUR_RADIUS * px)
#define RADIUS_Y (BLUR_RADIUS * py)

float linearize(float depth)
{
	return 1.0 / mix(u_ViewInfo.z, 1.0, depth);
}

float DistFromDepth(float depth)
{
	return depth * (u_ViewInfo.y / u_ViewInfo.x);
}

vec4 FastBlur(void)
{
	vec4 color = texture(u_DiffuseMap, var_TexCoords.xy);
	float depth = linearize(texture(u_ScreenDepthMap, var_TexCoords.xy).r);
	float dist = DistFromDepth(depth);
	float invDepth = 1.0 + (1.0 - depth);

	int NUM_BLUR_PIXELS = 1;

	for (float x = -RADIUS_X * invDepth; x <= RADIUS_X * invDepth; x += px * invDepth)
	{
		for (float y = -RADIUS_Y * invDepth; y <= RADIUS_Y * invDepth; y += py * invDepth)
		{
			float thisDist = DistFromDepth(linearize(texture(u_ScreenDepthMap, vec2(var_TexCoords.x + x, var_TexCoords.y + y)).r));
			
			if (length(thisDist - dist) > 8.0) continue;

			color.rgb += texture(u_DiffuseMap, vec2(var_TexCoords.x + x, var_TexCoords.y + y)).rgb;
			NUM_BLUR_PIXELS++;
		}
	}

	color.rgb /= NUM_BLUR_PIXELS;
	return color;
}

void main()
{
	gl_FragColor = FastBlur();
}

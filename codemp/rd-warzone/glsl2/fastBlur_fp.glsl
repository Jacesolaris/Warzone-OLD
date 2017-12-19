uniform sampler2D		u_DiffuseMap;

uniform vec4			u_ViewInfo; // zmin, zmax, zmax / zmin
uniform float			u_ShadowZfar[5];
uniform vec2			u_Dimensions;
uniform vec4			u_Settings0;

uniform vec4			u_Local0;

varying vec2			var_TexCoords;

#define BLUR_RADIUS		u_Settings0.r
#define BLUR_PIXELMULT	u_Settings0.g

#if 0
#define px (1.0/u_Dimensions.x)*BLUR_PIXELMULT
#define py (1.0/u_Dimensions.y)*BLUR_PIXELMULT

#define RADIUS_X (BLUR_RADIUS * px)
#define RADIUS_Y (BLUR_RADIUS * py)
#else
#define px (1.0/u_Dimensions.x)
#define py (1.0/u_Dimensions.y)
#endif

float DistFromDepth(float depth)
{
	return depth * u_ViewInfo.y;//u_ViewInfo.z;
}

vec4 FastBlur(void)
{
	vec2 shadowInfo = texture(u_DiffuseMap, var_TexCoords.xy).xy;
	float color = shadowInfo.x;
	float depth = shadowInfo.y;
	float dist = DistFromDepth(depth);

#if 0
	float invDepth = 1.0 + (1.0 - depth);
	invDepth *= 0.666;

	float NUM_BLUR_PIXELS = 1.0;

	for (float x = -RADIUS_X * invDepth; x <= RADIUS_X * invDepth; x += px * invDepth)
	{
		for (float y = -RADIUS_Y * invDepth; y <= RADIUS_Y * invDepth; y += py * invDepth)
		{
			vec2 thisInfo = texture(u_DiffuseMap, var_TexCoords + vec2(x, y)).xy;

			float thisDist = DistFromDepth(thisInfo.y);
			
			if (length(thisDist - dist) > 16.0/*8.0*/) continue;

			color += thisInfo.r;
			NUM_BLUR_PIXELS += 1.0;
		}
	}

	color /= NUM_BLUR_PIXELS;
	return vec4(color, depth, 0.0, 1.0);
#else
	float BLUR_DEPTH_MULT = depth;
	BLUR_DEPTH_MULT = clamp(pow(BLUR_DEPTH_MULT, 0.125/*u_Local0.r*/), 0.0, 1.0);

	//if (u_Local0.g > 0.0)
	//	return vec4(BLUR_DEPTH_MULT, BLUR_DEPTH_MULT, 0.0, 1.0);

	if (BLUR_DEPTH_MULT <= 0.0)
	{// No point... This would be less then 1 pixel...
		return vec4(color, depth, 0.0, 1.0);
	}

	float NUM_BLUR_PIXELS = 1.0;

	for (float x = -BLUR_RADIUS; x <= BLUR_RADIUS; x += BLUR_PIXELMULT)
	{
		for (float y = -BLUR_RADIUS; y <= BLUR_RADIUS; y += BLUR_PIXELMULT)
		{
			vec2 offset = vec2(x * px, y * py);
			vec2 xy = vec2(var_TexCoords + offset);
			float weight = clamp(1.0 / ((length(vec2(x, y)) + 1.0) * 0.666), 0.2, 1.0);

			vec2 thisInfo = texture(u_DiffuseMap, xy).xy;

			float thisDist = DistFromDepth(thisInfo.y);
			
			if (length(thisDist - dist) > 16.0) continue;

			//color += thisInfo.r;
			//NUM_BLUR_PIXELS += 1.0;

			color += (thisInfo.r * weight);
			NUM_BLUR_PIXELS += weight;
		}
	}

	color /= NUM_BLUR_PIXELS;
	color = mix(shadowInfo.x, color, clamp(BLUR_DEPTH_MULT * 2.0, 0.0, 1.0));
	return vec4(color, depth, 0.0, 1.0);
#endif
}

void main()
{
	gl_FragColor = FastBlur();
}

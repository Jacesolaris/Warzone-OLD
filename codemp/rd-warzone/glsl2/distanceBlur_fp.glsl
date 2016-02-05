uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

varying vec2		var_TexCoords;


//#define DOFCA
//#define MATSO_DOF_BOKEH



#define BLUR_DEPTH 0.55//0.28
#define BLUR_RADIUS 2.0

#define px (1.0/u_Dimensions.x)
#define py (1.0/u_Dimensions.y)

#define RADIUS_X (BLUR_RADIUS * px)
#define RADIUS_Y (BLUR_RADIUS * py)

#ifdef MATSO_DOF_BOKEH
#define fMatsoDOFBokehCurve		12.0	//[0.5 to 20.0] Bokeh curve.
#define fMatsoDOFBokehLight		1.5//0.512 	//[0.0 to 2.0] Bokeh brightening factor.
#endif //MATSO_DOF_BOKEH

#ifdef DOFCA
#define fMatsoDOFChromaPow		2.4//	//[0.2 to 3.0] Amount of chromatic abberation color shifting.

vec4 GetMatsoDOFCA(vec2 tex, float CoC)
{
	vec3 chroma;
	chroma.r = pow(0.5, fMatsoDOFChromaPow * CoC);
	chroma.g = pow(1.0, fMatsoDOFChromaPow * CoC);
	chroma.b = pow(1.5, fMatsoDOFChromaPow * CoC);

	vec2 tr = ((2.0 * tex - 1.0) * chroma.r) * 0.5 + 0.5;
	vec2 tg = ((2.0 * tex - 1.0) * chroma.g) * 0.5 + 0.5;
	vec2 tb = ((2.0 * tex - 1.0) * chroma.b) * 0.5 + 0.5;
	
	vec3 color = vec3(texture2D(u_DiffuseMap, tr).r, texture2D(u_DiffuseMap, tg).g, texture2D(u_DiffuseMap, tb).b) * (1.0 - CoC);
	
	return vec4(color, 1.0);
}
#endif //DOFCA

float linearize(float depth)
{
	return 1.0 / mix(u_ViewInfo.z, 1.0, depth);
}

vec4 DistantBlur(void)
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords.xy);
	float depth = linearize(texture2D(u_ScreenDepthMap, var_TexCoords.xy).r);

	if (depth < BLUR_DEPTH)
	{
		return color;
	}

	float BLUR_DEPTH_MULT = (1.0 - (BLUR_DEPTH / depth)) * BLUR_RADIUS;
	//BLUR_DEPTH_MULT = BLUR_DEPTH_MULT * 0.333 + 0.666;
	BLUR_DEPTH_MULT += 0.5;
	BLUR_DEPTH_MULT = pow(BLUR_DEPTH_MULT, 1.5);

	if (BLUR_DEPTH_MULT * RADIUS_X < px && BLUR_DEPTH_MULT * RADIUS_Y < py)
	{// No point...
		return color;
	}

#ifdef MATSO_DOF_BOKEH
	float wValue = (1.0 + pow(length(color.rgb) + 0.1, fMatsoDOFBokehCurve)) * (1.0 - fMatsoDOFBokehLight);	// special recipe from papa Matso ;)
#else //!MATSO_DOF_BOKEH
	float wValue = 1.0;
#endif //MATSO_DOF_BOKEH

	float NUM_BLUR_PIXELS = wValue;

	for (float x = -RADIUS_X * BLUR_DEPTH_MULT; x <= RADIUS_X * BLUR_DEPTH_MULT; x += px)
	{
		for (float y = -RADIUS_Y * BLUR_DEPTH_MULT; y <= RADIUS_Y * BLUR_DEPTH_MULT; y += py)
		{
			//if (x == 0.0 && y == 0.0) continue;

			//float depth2 = linearize(texture2D(u_ScreenDepthMap, vec2(var_TexCoords.x + x, var_TexCoords.y + y)).r);

			//if (depth2 < BLUR_DEPTH) continue;

#ifdef DOFCA
			vec3 color2 = GetMatsoDOFCA(vec2(var_TexCoords.x + x, var_TexCoords.y + y), x).rgb;
#else //!DOFCA
			vec3 color2 = texture2D(u_DiffuseMap, vec2(var_TexCoords.x + x, var_TexCoords.y + y)).rgb;
#endif //DOFCA

#ifndef MATSO_DOF_BOKEH
			float w = 1.0;	// weight blur for better effect
#else //MATSO_DOF_BOKEH
			// my own pseudo-bokeh weighting
			float b = dot(length(color2.rgb),0.333) + length(color2.rgb) + 0.1;
			float w = pow(b, fMatsoDOFBokehCurve) + abs(((x/px) + (y/py)) / 2.0);
#endif //MATSO_DOF_BOKEH

			color.rgb += color2.rgb * w;
			NUM_BLUR_PIXELS += w;
		}
	}

	color.rgb /= NUM_BLUR_PIXELS;
	return color;
}

void main()
{
	gl_FragColor = DistantBlur();
}

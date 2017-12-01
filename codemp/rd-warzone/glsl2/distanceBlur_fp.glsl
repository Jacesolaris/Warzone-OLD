#if defined(OLD_BLUR)

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin, MAP_WATER_LEVEL
uniform vec2		u_Dimensions;

varying vec2		var_TexCoords;

#define BLUR_DEPTH 0.55//0.28
#define BLUR_RADIUS 2.0

#define MAP_WATER_LEVEL u_ViewInfo.a

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
	
	vec3 color = vec3(textureLod(u_DiffuseMap, tr, 0.0).r, textureLod(u_DiffuseMap, tg, 0.0).g, textureLod(u_DiffuseMap, tb, 0.0).b) * (1.0 - CoC);
	
	return vec4(color, 1.0);
}
#endif //DOFCA

vec4 DistantBlur(void)
{
	vec4 color = textureLod(u_DiffuseMap, var_TexCoords.xy, 0.0);
	float depth = textureLod(u_ScreenDepthMap, var_TexCoords.xy, 0.0).r;

	if (depth < BLUR_DEPTH)
	{
		return color;
	}

	vec3 origColor = color.rgb;

	vec2 origMaterial = textureLod(u_PositionMap, var_TexCoords.xy, 0.0).za;

	bool isSky = false;

	if (origMaterial.y-1.0 == MATERIAL_SKY || origMaterial.y-1.0 == MATERIAL_SUN || origMaterial.y-1.0 <= MATERIAL_NONE)
	{// Skybox... 
		if (origMaterial.x >= MAP_WATER_LEVEL)
		{
			isSky = true;
		}
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
			bool pixelIsSky = false;

			vec2 xy = vec2(var_TexCoords.x + x, var_TexCoords.y + y);

			if (isSky)
			{// When original pixel is sky, check if this pixel is also sky. If so, skip the blur... If the new pixel is not sky, then add it to the blur...
				vec2 material = textureLod(u_PositionMap, xy, 0.0).za;

				if (material.y-1.0 == MATERIAL_SKY || material.y-1.0 == MATERIAL_SUN || origMaterial.y-1.0 <= MATERIAL_NONE)
				{// Skybox... Skip...
					if (material.x >= MAP_WATER_LEVEL)
					{
						pixelIsSky = true;
					}
				}
			}

			vec3 color2;

			if (!pixelIsSky)
			{
#ifdef DOFCA
				color2 = GetMatsoDOFCA(xy, x).rgb;
#else //!DOFCA
				color2 = textureLod(u_DiffuseMap, xy, 0.0).rgb;
#endif //DOFCA
			}
			else
			{
				color2 = origColor;
			}

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




#else //!OLD_BLUR


uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_GlowMap;

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo; // zfar / znear, zfar

uniform vec4		u_Local0; // dofValue, dynamicGlowEnabled, 0, direction
uniform vec4		u_Local1; // testvalues


varying vec2		var_TexCoords;


#define BLUR_DEPTH 0.55


vec2 sampleOffset = vec2(1.0/u_Dimensions);

#define PIOVER180 0.017453292

//MATSO DOF
#define fMatsoDOFChromaPow		2.4		// [0.2 to 3.0] Amount of chromatic abberation color shifting.
#define fMatsoDOFBokehCurve		6.0		// [0.5 to 20.0] Bokeh curve.
#define fMatsoDOFBokehLight		4.0//0.512 	// [0.0 to 2.0] Bokeh brightening factor.
#define fMatsoDOFBokehAngle		10		// [0 to 360] Rotation angle of bokeh shape.

#if defined(FAST_BLUR)

#define iMatsoDOFBokehQuality	3		// [1 to 10] Blur quality as control value over tap count.
#define DOF_BLURRADIUS 			10.0

#elif defined(MEDIUM_BLUR)

#define iMatsoDOFBokehQuality	6		// [1 to 10] Blur quality as control value over tap count.
#define DOF_BLURRADIUS 			10.0

#else //defined(HIGH_BLUR)

#define iMatsoDOFBokehQuality	8		// [1 to 10] Blur quality as control value over tap count.
#define DOF_BLURRADIUS 			10.0

#endif //defined(HIGH_BLUR)

const vec2 tdirs[4] = vec2[4]( vec2(-0.306, 0.739), vec2(0.306, 0.739), vec2(-0.739, 0.306), vec2(-0.739, -0.306) );

float ExpandDepth(float depth)
{
	return depth * 255.0;
}

float GetGlowStrength(vec2 coord)
{
	vec2 coord2 = coord;
	coord2.y = 1.0 - coord2.y;
	vec4 glow = textureLod(u_GlowMap, coord2.xy, 0.0);
	float glowStrength = clamp(length(glow.rgb), 0.0, 1.0);
	return glowStrength;
}

vec4 GetMatsoDOFCA(sampler2D col, vec2 tex, float CoC)
{
	vec3 chroma;
	chroma.r = pow(0.5, fMatsoDOFChromaPow * CoC);
	chroma.g = pow(1.0, fMatsoDOFChromaPow * CoC);
	chroma.b = pow(1.5, fMatsoDOFChromaPow * CoC);

	vec2 tr = ((2.0 * tex - 1.0) * chroma.r) * 0.5 + 0.5;
	vec2 tg = ((2.0 * tex - 1.0) * chroma.g) * 0.5 + 0.5;
	vec2 tb = ((2.0 * tex - 1.0) * chroma.b) * 0.5 + 0.5;
	
	vec3 color = vec3(textureLod(col, tr, 0.0).r, textureLod(col, tg, 0.0).g, textureLod(col, tb, 0.0).b) * (1.0 - CoC);

	return vec4(color, 1.0);
}

vec4 GetMatsoDOFBlur(int axis, vec2 coord, sampler2D SamplerHDRX)
{
	vec4 tcol = textureLod(SamplerHDRX, coord.xy, 0.0);

	/*float material = textureLod(u_PositionMap, var_TexCoords.xy, 0.0).a;

	if (material-1.0 == MATERIAL_SKY || material-1.0 == MATERIAL_SUN)
	{// Skybox... Skip...
		return tcol;
	}*/

	float depth = textureLod(u_ScreenDepthMap, coord.xy, 0.0).x;
	float lDepth = depth;

	if (lDepth < BLUR_DEPTH)
	{
		return tcol;
	}

	float depthDiff = (lDepth - BLUR_DEPTH) * 4.0;//u_Local1.r;
	vec2 discRadius = (depthDiff * float(DOF_BLURRADIUS)) * sampleOffset.xy * 0.5 / float(iMatsoDOFBokehQuality);
	float wValue = 1.0;
	
	if (depthDiff < 0.0)
	{// Close to camera pixels, blur much less so player model is not blury...
		discRadius *= 0.003;
	}

	discRadius *= 0.5;

#if !defined(FAST_BLUR)
	bool isGlow = false;

	if (u_Local0.g <= 0.0)
	{// No dynamic glow map to use. Apply bokeh as per standard matso DOF...
		wValue = (1.0 + pow(length(tcol.rgb) + 0.1, fMatsoDOFBokehCurve)) * (1.0 - fMatsoDOFBokehLight);	// special recipe from papa Matso ;)
	}
	else
	{// Have dynamic glow to use. Only apply brightening to glow areas...
		float glowStrength = GetGlowStrength(coord);
		isGlow = bool(glowStrength > 0.05);

		if (isGlow)
		{// Since we have a glow map for the screen, only brighten when this pixel is on the glow map... So we don't brighten non-glowing parts of the screen.
			wValue = (1.0 + pow(length(tcol.rgb) + 0.1, fMatsoDOFBokehCurve)) * (1.0 - (fMatsoDOFBokehLight*glowStrength));	// special recipe from papa Matso ;)
		}
	}
#endif

	for (int i = -iMatsoDOFBokehQuality; i < iMatsoDOFBokehQuality; i++)
	{
		vec2 taxis =  tdirs[axis];

		taxis.x = cos(fMatsoDOFBokehAngle*PIOVER180)*taxis.x-sin(fMatsoDOFBokehAngle*PIOVER180)*taxis.y;
		taxis.y = sin(fMatsoDOFBokehAngle*PIOVER180)*taxis.x+cos(fMatsoDOFBokehAngle*PIOVER180)*taxis.y;
		
		float fi = float(i);
		vec2 tcoord = coord.xy + (taxis * fi * discRadius).xy;

		vec4 ct;

#if !defined(FAST_BLUR)
		if (isGlow || u_Local0.g <= 0.0)
			ct = GetMatsoDOFCA(SamplerHDRX, tcoord.xy, discRadius.x);
		else
#endif
			ct = textureLod(SamplerHDRX, tcoord.xy, 0.0);

		// my own pseudo-bokeh weighting
		float b = dot(length(ct.rgb),0.333) + length(ct.rgb) + 0.1;
		float w = pow(b, fMatsoDOFBokehCurve) + abs(fi);

		tcol += ct * w;
		wValue += w;
	}

	tcol /= wValue;

	return vec4(tcol.rgb, 1.0);
}

void main ()
{
	gl_FragColor = GetMatsoDOFBlur(int(u_Local0.a), var_TexCoords, u_DiffuseMap);
}




#endif //!OLD_BLUR

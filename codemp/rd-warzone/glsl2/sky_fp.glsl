#define __HIGH_PASS_SHARPEN__

#define SCREEN_MAPS_ALPHA_THRESHOLD 0.666

uniform sampler2D					u_DiffuseMap;
uniform sampler2D					u_OverlayMap; // Night sky image... When doing sky...
uniform sampler2D					u_SplatMap1;
uniform sampler2D					u_SplatMap2;

uniform vec4						u_Settings0; // useTC, useDeform, useRGBA, isTextureClamped
uniform vec4						u_Settings1; // useVertexAnim, useSkeletalAnim, blendMode, is2D
uniform vec4						u_Settings2; // LIGHTDEF_USE_LIGHTMAP, LIGHTDEF_USE_GLOW_BUFFER, LIGHTDEF_USE_CUBEMAP, LIGHTDEF_USE_TRIPLANAR
uniform vec4						u_Settings3; // LIGHTDEF_USE_REGIONS, LIGHTDEF_IS_DETAIL, 0=DetailMapNormal 1=detailMapFromTC 2=detailMapFromWorld, 0.0

#define USE_TC						u_Settings0.r
#define USE_DEFORM					u_Settings0.g
#define USE_RGBA					u_Settings0.b
#define USE_TEXTURECLAMP			u_Settings0.a

#define USE_VERTEX_ANIM				u_Settings1.r
#define USE_SKELETAL_ANIM			u_Settings1.g
#define USE_BLEND					u_Settings1.b
#define USE_IS2D					u_Settings1.a

#define USE_LIGHTMAP				u_Settings2.r
#define USE_GLOW_BUFFER				u_Settings2.g
#define USE_CUBEMAP					u_Settings2.b
#define USE_TRIPLANAR				u_Settings2.a

#define USE_REGIONS					u_Settings3.r
#define USE_ISDETAIL				u_Settings3.g
#define USE_DETAIL_COORD			u_Settings3.b

uniform vec4						u_Local1; // MAP_SIZE, sway, overlaySway, materialType
uniform vec4						u_Local2; // hasSteepMap, hasWaterEdgeMap, haveNormalMap, WATER_LEVEL
uniform vec4						u_Local3; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4						u_Local4; // stageNum, glowStrength, r_showsplat, 0.0
uniform vec4						u_Local5; // dayNightEnabled, nightScale, skyDirection, auroraEnabled -- Sky draws only!
uniform vec4						u_Local9; // testvalue0, 1, 2, 3

#define SHADER_MAP_SIZE				u_Local1.r
#define SHADER_SWAY					u_Local1.g
#define SHADER_OVERLAY_SWAY			u_Local1.b
#define SHADER_MATERIAL_TYPE		u_Local1.a

#define SHADER_HAS_STEEPMAP			u_Local2.r
#define SHADER_HAS_WATEREDGEMAP		u_Local2.g
#define SHADER_HAS_NORMALMAP		u_Local2.b
#define SHADER_WATER_LEVEL			u_Local2.a

#define SHADER_HAS_SPLATMAP1		u_Local3.r
#define SHADER_HAS_SPLATMAP2		u_Local3.g
#define SHADER_HAS_SPLATMAP3		u_Local3.b
#define SHADER_HAS_SPLATMAP4		u_Local3.a

#define SHADER_STAGE_NUM			u_Local4.r
#define SHADER_GLOW_STRENGTH		u_Local4.g
#define SHADER_SHOW_SPLAT			u_Local4.b

#define SHADER_DAY_NIGHT_ENABLED	u_Local5.r
#define SHADER_NIGHT_SCALE			u_Local5.g
#define SHADER_SKY_DIRECTION		u_Local5.b
#define SHADER_AURORA_ENABLED		u_Local5.a


uniform vec2						u_Dimensions;
uniform vec3						u_ViewOrigin;
uniform vec4						u_PrimaryLightOrigin;
uniform vec3						u_PrimaryLightColor;
uniform float						u_Time;


varying vec2						var_TexCoords;
varying vec3						var_Position;
varying vec3						var_Normal;
varying vec4						var_Color;

out vec4							out_Glow;
out vec4							out_Position;
out vec4							out_Normal;
out vec4							out_NormalDetail;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

vec3 GetStars( vec2 coord )
{
	float prob = 0.998;
	float color = 0.0;
	float r = rand(coord.xy);

	if (r > prob)
	{
		color = r * (0.25 * rand(coord.xy * u_Time) + 0.75);
	}
	
	return vec3(color);
}

vec3 extra_cheap_atmosphere(vec3 raydir, vec3 sundir, vec3 suncolorIn) {
	sundir.y = abs(sundir.y);
	float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
	float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
	float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
	float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
	float mymie = sundt * special_trick * 0.2;
	vec3 suncolor = mix(vec3(1.0), max(vec3(0.0), vec3(1.0) - vec3(5.5, 13.0, 22.4) / 22.4), special_trick2);
	//suncolor *= suncolorIn;
	vec3 bluesky = vec3(5.5, 13.0, 22.4) / 22.4 * suncolor;
	vec3 bluesky2 = max(vec3(0.0), bluesky - vec3(5.5, 13.0, 22.4) * 0.004 * (special_trick + -6.0 * sundir.y * sundir.y));
	bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
	return bluesky2 + mymie * suncolor;
}

vec3 getatm(vec3 ray, vec3 sundir, vec3 suncolorIn) {
	return extra_cheap_atmosphere(ray, normalize(vec3(1.0, 0.1, 1.0)), suncolorIn) * 0.5;
}

#define PI M_PI
#define iSteps 16
#define jSteps 8

vec2 rsi(vec3 r0, vec3 rd, float sr) {
	// ray-sphere intersection that assumes
	// the sphere is centered at the origin.
	// No intersection when result.x > result.y
	float a = dot(rd, rd);
	float b = 2.0 * dot(rd, r0);
	float c = dot(r0, r0) - (sr * sr);
	float d = (b*b) - 4.0*a*c;
	if (d < 0.0) return vec2(1e5, -1e5);
	return vec2(
		(-b - sqrt(d)) / (2.0*a),
		(-b + sqrt(d)) / (2.0*a)
	);
}

vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
	// Normalize the sun and view directions.
	pSun = normalize(pSun);
	r = normalize(r);

	// Calculate the step size of the primary ray.
	vec2 p = rsi(r0, r, rAtmos);
	if (p.x > p.y) return vec3(0, 0, 0);
	p.y = min(p.y, rsi(r0, r, rPlanet).x);
	float iStepSize = (p.y - p.x) / float(iSteps);

	// Initialize the primary ray time.
	float iTime = 0.0;

	// Initialize accumulators for Rayleigh and Mie scattering.
	vec3 totalRlh = vec3(0, 0, 0);
	vec3 totalMie = vec3(0, 0, 0);

	// Initialize optical depth accumulators for the primary ray.
	float iOdRlh = 0.0;
	float iOdMie = 0.0;

	// Calculate the Rayleigh and Mie phases.
	float mu = dot(r, pSun);
	float mumu = mu * mu;
	float gg = g * g;
	float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
	float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

	// Sample the primary ray.
	for (int i = 0; i < iSteps; i++) {

		// Calculate the primary ray sample position.
		vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

		// Calculate the height of the sample.
		float iHeight = length(iPos) - rPlanet;

		// Calculate the optical depth of the Rayleigh and Mie scattering for this step.
		float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
		float odStepMie = exp(-iHeight / shMie) * iStepSize;

		// Accumulate optical depth.
		iOdRlh += odStepRlh;
		iOdMie += odStepMie;

		// Calculate the step size of the secondary ray.
		float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

		// Initialize the secondary ray time.
		float jTime = 0.0;

		// Initialize optical depth accumulators for the secondary ray.
		float jOdRlh = 0.0;
		float jOdMie = 0.0;

		// Sample the secondary ray.
		for (int j = 0; j < jSteps; j++) {

			// Calculate the secondary ray sample position.
			vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

			// Calculate the height of the sample.
			float jHeight = length(jPos) - rPlanet;

			// Accumulate the optical depth.
			jOdRlh += exp(-jHeight / shRlh) * jStepSize;
			jOdMie += exp(-jHeight / shMie) * jStepSize;

			// Increment the secondary ray time.
			jTime += jStepSize;
		}

		// Calculate attenuation.
		vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

		// Accumulate scattering.
		totalRlh += odStepRlh * attn;
		totalMie += odStepMie * attn;

		// Increment the primary ray time.
		iTime += iStepSize;

	}

	// Calculate and return the final color.
	return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

#if defined(__HIGH_PASS_SHARPEN__)
vec3 Enhance(in sampler2D tex, in vec2 uv, vec3 color, float level)
{
	vec3 blur = textureLod(tex, uv, level).xyz;
	vec3 col = ((color - blur)*0.5 + 0.5) * 1.0;
	col *= ((color - blur)*0.25 + 0.25) * 8.0;
	col = mix(color, col * color, 1.0);
	return col;
}
#endif //defined(__HIGH_PASS_SHARPEN__)

void main()
{
#if 0
	gl_FragColor = vec4(GetStars( var_TexCoords ) * u_Local9.g, 1.0);
#else
	if (USE_TRIPLANAR > 0.0 || USE_REGIONS > 0.0)
	{// Can skip nearly everything... These are always going to be solid color...
		gl_FragColor = vec4(1.0);
	}
	else
	{
		vec2 texCoords = var_TexCoords;

		gl_FragColor = texture(u_DiffuseMap, texCoords);
#ifdef __HIGH_PASS_SHARPEN__
		gl_FragColor.rgb = Enhance(u_DiffuseMap, texCoords, gl_FragColor.rgb, 1.0/*8.0*/);
#endif //__HIGH_PASS_SHARPEN__

#if 0
		// Experimenting with atmospheric scatter...
#if 1
		//vec3 skyViewDir = normalize(u_ViewOrigin.xzy - var_Position.xzy);
		//vec3 skySundir = normalize(u_ViewOrigin.xzy - u_PrimaryLightOrigin.xzy);
		//vec3 skyRaydir = reflect(skyViewDir, var_Normal.xzy);
		vec3 skyViewDir = normalize(u_ViewOrigin.xzy - var_Position.xzy);
		vec3 skySundir = normalize(u_ViewOrigin.xzy - u_PrimaryLightOrigin.xzy);
		vec3 atmos = getatm(skyRaydir, skySundir, u_PrimaryLightColor);
#else
		vec3 skyRaydir = normalize(u_ViewOrigin.xzy - var_Position.xzy);
		vec3 skySundir = normalize(u_ViewOrigin.xzy - u_PrimaryLightOrigin.xzy);

		vec3 atmos = atmosphere(
			skyRaydir,						// normalized ray direction
			u_ViewOrigin.xzy/*vec3(0, 6372e3, 0)*/,             // ray origin
			u_PrimaryLightOrigin.xzy/*uSunPos*/,                        // position of the sun
			22.0,                           // intensity of the sun
			6371e3,                         // radius of the planet in meters
			6471e3,                         // radius of the atmosphere in meters
			vec3(5.5e-6, 13.0e-6, 22.4e-6), // Rayleigh scattering coefficient
			21e-6,                          // Mie scattering coefficient
			8e3,                            // Rayleigh scale height
			1.2e3,                          // Mie scale height
			0.758                           // Mie preferred scattering direction
		);
#endif

		// Apply exposure.
		//atmos = 1.0 - exp(-1.0 * atmos);

		gl_FragColor.rgb += atmos * u_Local9.r;
#endif

		if (SHADER_MATERIAL_TYPE == 1024.0)
		{// This is sky, and aurora is enabled...
			if (SHADER_DAY_NIGHT_ENABLED > 0.0 && SHADER_NIGHT_SCALE > 0.0)
			{// Day/Night cycle is enabled, and some night sky contribution is required...
				vec3 nightDiffuse = texture(u_OverlayMap, texCoords).rgb;
#ifdef __HIGH_PASS_SHARPEN__
				nightDiffuse.rgb = Enhance(u_OverlayMap, texCoords, nightDiffuse.rgb, 1.0);
#endif //__HIGH_PASS_SHARPEN__
				//nightDiffuse += GetStars( texCoords ) * u_Local9.g;
				gl_FragColor.rgb = mix(gl_FragColor.rgb, nightDiffuse, SHADER_NIGHT_SCALE); // Mix in night sky with original sky from day -> night...
			}

			if (SHADER_SKY_DIRECTION != 4.0 && SHADER_SKY_DIRECTION != 5.0													/* Not up/down sky textures */
				&& SHADER_AURORA_ENABLED > 0.0																		/* Auroras Enabled */
				&& ((SHADER_DAY_NIGHT_ENABLED > 0.0 && SHADER_NIGHT_SCALE > 0.0) /* Night Aurora */ || SHADER_AURORA_ENABLED >= 2.0		/* Forced day Aurora */))
			{// Aurora is enabled, and this is not up/down sky textures, add a sexy aurora effect :)
				vec2 fragCoord = texCoords;

				if (SHADER_SKY_DIRECTION == 2.0 || SHADER_SKY_DIRECTION == 3.0)
				{// Forward or back sky textures, invert the X axis to make the aura seamless...
					fragCoord.x = 1.0 - fragCoord.x;
				}

				float auroraPower;

				if (SHADER_AURORA_ENABLED >= 2.0)
					auroraPower = 1.0; // Day enabled aurora - always full strength...
				else
					auroraPower = SHADER_NIGHT_SCALE;

				vec2 uv = fragCoord.xy;
				
				// Move aurora up a bit above horizon...
				uv *= 0.8;
				uv += 0.2;

				uv = clamp(uv, 0.0, 1.0);
   
#define TAU 6.2831853071
#define time u_Time * 0.5


				float o = texture(u_SplatMap1, uv * 0.25 + vec2(0.0, time * 0.025)).r;
				float d = (texture(u_SplatMap2, uv * 0.25 - vec2(0.0, time * 0.02 + o * 0.02)).r * 2.0 - 1.0);
    
				float v = uv.y + d * 0.1;
				v = 1.0 - abs(v * 2.0 - 1.0);
				v = pow(v, 2.0 + sin((time * 0.2 + d * 0.25) * TAU) * 0.5);
				v = clamp(v, 0.0, 1.0);
    
				vec3 color = vec3(0.0);
    
				float x = (1.0 - uv.x * 0.75);
				float y = 1.0 - abs(uv.y * 2.0 - 1.0);
				x = clamp(x, 0.0, 1.0);
				y = clamp(y, 0.0, 1.0);
				color += vec3(x * 0.5, y, x) * v;
    
				vec2 seed = fragCoord.xy;
				vec2 r;
				r.x = fract(sin((seed.x * 12.9898) + (seed.y * 78.2330)) * 43758.5453);
				r.y = fract(sin((seed.x * 53.7842) + (seed.y * 47.5134)) * 43758.5453);

				float s = mix(r.x, (sin((time * 2.5 + 60.0) * r.y) * 0.5 + 0.5) * ((r.y * r.y) * (r.y * r.y)), 0.04); 
				color += clamp(pow(s, 70.0) * (1.0 - v), 0.0, 1.0);
				float str = max(color.r, max(color.g, color.b));

				color *= 0.7;
				gl_FragColor.rgb = mix(gl_FragColor.rgb, gl_FragColor.rgb + color, auroraPower * str);
			}
		}

		gl_FragColor.a *= var_Color.a;
	}
#endif

	if (USE_BLEND > 0.0)
	{// Emulate RGB blending... Fuck I hate this crap...
		float colStr = clamp(max(gl_FragColor.r, max(gl_FragColor.g, gl_FragColor.b)), 0.0, 1.0);

		if (USE_BLEND == 3.0)
		{
			gl_FragColor.a *= colStr * 2.0;
			gl_FragColor.rgb *= 0.5;
		}
		else if (USE_BLEND == 2.0)
		{
			colStr = clamp(colStr + 0.1, 0.0, 1.0);
			gl_FragColor.a = 1.0 - colStr;
		}
		else
		{
			colStr = clamp(colStr - 0.1, 0.0, 1.0);
			gl_FragColor.a = colStr;
		}
	}
	
	if (gl_FragColor.a > SCREEN_MAPS_ALPHA_THRESHOLD)
	{
		if (SHADER_MATERIAL_TYPE == 1024.0 && SHADER_DAY_NIGHT_ENABLED > 0.0 && SHADER_NIGHT_SCALE > 0.7)
		{// Add night sky to glow map...
			out_Glow = gl_FragColor;
		
			// Scale by closeness to actual night...
			float mult = (SHADER_NIGHT_SCALE - 0.7) * 3.333;
			out_Glow *= mult;

			// And enhance contrast...
			out_Glow.rgb *= out_Glow.rgb;

			// And reduce over-all brightness because it's sky and not a close light...
			out_Glow.rgb *= 0.5;
		}
		else
		{
			out_Glow = vec4(0.0);
		}

		out_Position = vec4(var_Position.rgb, SHADER_MATERIAL_TYPE+1.0);
		out_Normal = vec4(EncodeNormal(var_Normal.rgb), 0.0, 1.0);
		out_NormalDetail = vec4(0.0);
	}
	else
	{
		out_Glow = vec4(0.0);
		out_Position = vec4(0.0);
		out_Normal = vec4(0.0);
		out_NormalDetail = vec4(0.0);
	}
}

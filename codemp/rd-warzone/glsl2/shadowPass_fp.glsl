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
uniform float						u_Time;


varying vec2						var_TexCoords;
varying vec3						var_Position;
varying vec3						var_Normal;
varying vec4						var_Color;

out vec4							out_Glow;
out vec4							out_Position;
out vec4							out_Normal;
out vec4							out_NormalDetail;

void main()
{
	if (USE_TRIPLANAR > 0.0 || USE_REGIONS > 0.0)
	{// Can skip nearly everything... These are always going to be solid color... When rendering cubes, though, we still need textures...
		gl_FragColor = vec4(1.0);
	}
	else
	{
		vec2 texCoords = var_TexCoords;

		if (SHADER_SWAY > 0.0)
		{// Sway...
			texCoords += vec2(SHADER_OVERLAY_SWAY * SHADER_SWAY * ((1.0 - texCoords.y) + 1.0), 0.0);
		}

		gl_FragColor = texture(u_DiffuseMap, texCoords);

		if (SHADER_MATERIAL_TYPE == 1024.0)
		{// This is sky, and aurora is enabled...
			if (SHADER_DAY_NIGHT_ENABLED > 0.0 && SHADER_NIGHT_SCALE > 0.0)
			{// Day/Night cycle is enabled, and some night sky contribution is required...
				vec3 nightDiffuse = texture(u_OverlayMap, texCoords).rgb;
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
		out_Normal = vec4(var_Normal.rgb * 0.5 + 0.5, 1.0);
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

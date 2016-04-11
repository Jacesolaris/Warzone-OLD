/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define tex2D(tex, coord) texture2D(tex, coord)
#define tex2Dlod(tex, coord) texture2D(tex, coord)
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4
#define frac fract

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_ScreenDepthMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;
uniform vec4		u_Local0; // range, powMult, 0, 0

varying vec2		var_TexCoords;

#if 0
vec4 PS_Reflection()
{
	float	px 			= 1.0 / var_Dimensions.x;
	float	py 			= 1.0f / var_Dimensions.y;
	vec2	OFFSET		= vec2(px, py);
  
	vec4	ColorInput = texture2D(u_DiffuseMap, var_TexCoords.xy);
	vec4	OrigColor = ColorInput;//vec4(0.6, 0.6, 1.0, 1.0);

	if (ColorInput.a <= 0.0)
	{// Reflective...
		vec2 reflectCoords = var_TexCoords.xy;

		for (float y = var_TexCoords.y; y < 1.0; y += (py*1.0))
		{
			float thisAlpha = texture2D(u_DiffuseMap, vec2(var_TexCoords.x,y)).a;
			
			if (thisAlpha >= 1.0)
			{// Found top of reflection area...
				float edgeLength = var_TexCoords.y - y;
				float reflectSpot = var_TexCoords.y - (edgeLength * 2.0);
				if (reflectSpot > 0.0 && reflectSpot < 1.0)
				{
					reflectCoords.y = reflectSpot;
				}

				break;
			}
		}

		// Blur???
		//ColorInput.rgb = (texture2D(u_DiffuseMap, reflectCoords.xy).rgb + texture2D(u_DiffuseMap, reflectCoords.xy + OFFSET).rgb + texture2D(u_DiffuseMap, reflectCoords.xy - OFFSET).rgb + texture2D(u_DiffuseMap, reflectCoords.xy + vec2(px, 0.0)).rgb + texture2D(u_DiffuseMap, reflectCoords.xy + vec2(0.0, py)).rgb) / 5.0;
		ColorInput.rgb = texture2D(u_DiffuseMap, reflectCoords.xy).rgb;// * distFromTop;
		ColorInput.rgb += OrigColor.rgb + OrigColor.rgb;
		ColorInput.rgb /= 3.0;
		ColorInput.a = 1.0;
	}

	return ColorInput;
}
#endif



uniform float u_Time;

vec3 up_vec = vec3(0.0, 1.0, 0.0);

float linearize(float depth)
{
	return 1.0 / mix(u_ViewInfo.z, 1.0, depth);
}

float blend ( float val, float val0, float val1, float res0, float res1 )
{
	if ( val <= val0 ) return res0;
	if ( val >= val1 ) return res1;
	//
	return res0 + (val - val0) * (res1 - res0) / (val1 - val0);
}

void main ( void )
{
	vec2 dpos = var_TexCoords;

    if (u_Local0.r >= 1.0)
	{
		dpos.y = 1.0 - dpos.y;
	}

	float depth = linearize(texture2D(u_ScreenDepthMap, dpos).r);
    vec4 backcolor = texture2D(u_DiffuseMap, var_TexCoords);

    bool isGrass =true;//backcolor.g > backcolor.r + 0.01 && backcolor.g > backcolor.b + 0.01;

    if (isGrass) {
        vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
        vec2 p = dpos;
        float d = blend(depth, 0.0, 500.0, 100.0, 500.0) * u_Local0.g;
        float dclose = blend(depth, 0.0, 20.0, 30.0, 1.0) * u_Local0.g;
        d *= dclose;
        p.y += p.x * 1009.0 + p.x * 1259.0 + p.x * 2713.0;
        p.y += u_Time * 0.004;
        // wind

		float yoffset = fract(p.y * d) / d;

		vec2 uvoffset, uvoffset_d;

        if (u_Local0.r >= 1.0)
		{
			uvoffset = var_TexCoords.xy + (up_vec.xy * yoffset);
			uvoffset_d = dpos.xy - (up_vec.xy * yoffset);
		}
        else
		{
			uvoffset = var_TexCoords.xy + (up_vec.xy * yoffset);
			uvoffset_d = dpos.xy + (up_vec.xy * yoffset);
        }

		color = texture2D(u_DiffuseMap, uvoffset);
        float depth2 = linearize(texture2D(u_ScreenDepthMap, uvoffset_d).r);
        
		if (depth2 < depth) {
			//gl_FragColor = backcolor;
			//gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
			return;
		}

        gl_FragColor = mix(backcolor, color, clamp(1.0 - yoffset * d / 3.8, 0.0, 1.0));
		//gl_FragColor = mix(backcolor, color, clamp(yoffset * d / 3.8, 0.0, 1.0));
		//gl_FragColor = vec4(clamp(1.0 - yoffset * d / 3.8, 0.0, 1.0), clamp(1.0 - yoffset * d / 3.8, 0.0, 1.0), clamp(1.0 - yoffset * d / 3.8, 0.0, 1.0), 1.0);
		//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		return;
    }


	//
	//gl_FragColor = backcolor;
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
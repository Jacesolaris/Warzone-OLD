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


#if 0
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
#endif




uniform mat4		u_ModelViewProjectionMatrix;

uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;


//==RAY MARCHING CONSTANTS=========================================================
#define EPSILON .0001
#define MAX_VIEW_STEPS 100
#define MAX_SHADOW_STEPS 64
#define OCCLUSION_SAMPLES 8.0
#define OCCLUSION_FACTOR .5
#define MAX_DEPTH 10.0
#define BUMP_FACTOR .04
#define TEX_SCALE_FACTOR .4

#define PEN_FACTOR 50.0


vec3 Matrixify(vec3 pos)
{
	return vec4(u_ModelViewProjectionMatrix * vec4(pos, 1.0)).xyz;
}

vec2 ScreenPositionToCoord( in vec3 pos ) 
{
	vec2 coord;
	vec2 hu_Dimensions=u_Dimensions/2.0;
	pos.x /= pos.z;
	coord.x = (pos.x * 0.5 + 0.5);
        
	pos.y /= -pos.z / (hu_Dimensions.x/hu_Dimensions.y);
	coord.y = -(pos.y * 0.5 - 0.5);
        
	return coord;// + 0.5/(u_Dimensions/2.0);
}

float getDist(vec3 pos, vec3 samplePos)
{
	vec2 coord;
	vec3 sPos = samplePos;

	coord = var_TexCoords + ((pos.xy - sPos.xy) * (1.0 / u_Dimensions));

	if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
	{// Off screen...
		return 4.0;
	}

	vec3 nPos = Matrixify(texture2D(u_PositionMap, coord).xyz);
	return min(length(nPos - pos) * 0.2, 4.0) * 128.0;
}

float calcOcclusion(vec3 pos, vec3 surfaceNormal)
{
	float result = 0.0;
	vec3 normalPos = vec3(pos);
	for(float i = 0.0; i < OCCLUSION_SAMPLES; i+=1.0)
	{
		normalPos += surfaceNormal * (1.0/OCCLUSION_SAMPLES);
		result += (1.0/exp2(i)) * (i/OCCLUSION_SAMPLES)-getDist(pos, normalPos);
	}
	return 1.0-(OCCLUSION_FACTOR*result);
}

float calcShadow( vec3 origin, vec3 lightDir, vec3 lightpos, float penumbraFactor, vec3 viewPos)
{
	float dist;
	float result = 1.0;
	float lightDist = length(lightpos-origin);
	
	vec3 pos = vec3(origin)+(lightDir*(EPSILON*15.0+BUMP_FACTOR));
	
	for(int i = 0; i < MAX_SHADOW_STEPS; i++)
	{
		dist = getDist(origin, pos);
		if(dist < EPSILON)
		{
			return 0.0;
		}
		if(length(pos-origin) > lightDist || length(pos-origin) > MAX_DEPTH)
		{
			return result;
		}
		pos+=lightDir*dist;
		if( length(pos-origin) < lightDist )
		{
			result = min( result, penumbraFactor*dist / length(pos-origin) );
		}
	}
	return result;
}

void main ( void )
{
	vec2 coord = var_TexCoords;
	vec3 normal = texture2D(u_NormalMap, coord).xyz * 2.0 - 1.0;

	if (u_Local0.a > 0.0)
	{// Just draw screen normals for debugging...
		gl_FragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}

	vec4 color = texture2D(u_DiffuseMap, coord);
	vec4 positionMap = texture2D(u_PositionMap, coord);

	if (positionMap.a == 0.0)
	{// No material info on this pixel, it is most likely sky or generic shader...
		gl_FragColor = color;
		return;
	}

	vec3 pos = Matrixify(positionMap.xyz);

	float occlusion = (1.0 - clamp(calcOcclusion(pos, normal) * 0.0002, 0.0, 1.0));
	color.rgb *= occlusion;

	gl_FragColor = color;
}

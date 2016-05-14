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

uniform mat4		u_ModelViewProjectionMatrix;

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;		// testshadervalue1, testshadervalue2, testshadervalue3, testshadervalue4
uniform vec4		u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], 0.0

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

varying vec2		var_TexCoords;


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

vec4 GetPosAndDepth(sampler2D tex, ivec2 coord, int num)
{
	vec4 pos = texelFetch(tex, coord, num);
	pos.w = length((pos.xyz - u_ViewOrigin) / u_MapInfo.xyz);
	return pos;
}

void main ( void )
{
	//vec4 p = GetPosAndDepth(u_PositionMap, ivec2(var_TexCoords.x*u_Dimensions.x, var_TexCoords.y*u_Dimensions.y), 0);
	//gl_FragColor = vec4(vec3(p.a), 1.0);
	//return;

	vec2 coord = var_TexCoords;
	vec3 normal = texture2D(u_NormalMap, coord).xyz * 2.0 - 1.0;

	/*if (u_Local0.a > 0.0)
	{// Just draw screen normals for debugging...
		gl_FragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}*/

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

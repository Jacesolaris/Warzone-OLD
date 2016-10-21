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
uniform mat4		u_ModelViewMatrix;

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;		// testshadervalue1, testshadervalue2, testshadervalue3, testshadervalue4
uniform vec4		u_Local2;		//
uniform vec4		u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform int			u_lightCount;
uniform vec3		u_lightPositions2[16];
uniform float		u_lightDistances[16];
uniform vec3		u_lightColors[16];

varying vec2		var_TexCoords;
varying vec3		var_vertPos;
varying vec3		var_viewOrg;
varying vec3		var_rayOrg;
varying vec3		var_sunOrg;
varying vec3		var_rayDir;
varying vec3		var_sunDir;

void main(void)
{
	//vec4 norm = texture2D(u_NormalMap, var_TexCoords);
	//gl_FragColor = vec4(norm.rgb * 0.5 + 0.5, 1.0);
	//return;

	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);
	gl_FragColor = vec4(color.rgb, 1.0);

	vec4 position = texture2D(u_PositionMap, var_TexCoords);

	if (position.a == 1024.0)
	{// Skybox... Skip...
		return;
	}

	if (position.a == 0.0 && position.xyz == vec3(0.0))
	{// Unknown... Skip...
		return;
	}

	vec4 norm = texture2D(u_NormalMap, var_TexCoords);
	norm.rgb = norm.rgb * 2.0 - 1.0;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	vec3 N = norm.xyz;

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	float lambertian2 = dot(PrimaryLightDir.xyz, N);
	float spec2 = 0.0;
	bool noSunPhong = false;
	float phongFactor = u_Local2.r;

	if (phongFactor < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
		noSunPhong = true;
		phongFactor = 0.0;
	}

	if (lambertian2 > 0.0)
	{// this is blinn phong
		vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
		float specAngle = max(dot(halfDir2, N), 0.0);
		spec2 = pow(specAngle, 16.0);
		gl_FragColor.rgb += vec3(spec2 * (1.0 - norm.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor * 0.3;
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
		phongFactor = -u_Local2.r;
	}

	if (u_lightCount > 0.0)
	{
		for (int li = 0; li < u_lightCount; li++)
		{
			vec3 lightDir = normalize(u_lightPositions2[li] - position.xyz);
			float lambertian3 = dot(lightDir.xyz, N);
			float spec3 = 0.0;

			if (lambertian3 > 0.0)
			{
				float lightDist = distance(u_lightPositions2[li], position.xyz);
				float lightMax = u_lightDistances[li] * 1.5;//u_Local9.r;

				if (lightDist < lightMax)
				{
					float lightStrength = 1.0 - (lightDist / lightMax);
					lightStrength = pow(lightStrength * 0.9/*u_Local9.g*/, 3.0/*u_Local9.b*/);

					if (lightStrength > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, N), 0.0);
						spec3 = pow(specAngle3, 16.0);
						gl_FragColor.rgb += vec3((1.0 - spec3) * (1.0 - norm.a)) * u_lightColors[li].rgb * lightStrength * phongFactor * 0.6;
					}
				}
			}
		}
	}
}

#if 0

vec4 GetPosAndDepth(vec2 coord)
{
	vec4 pos = texture2D(u_PositionMap, coord);
	pos.a = distance(pos.xyz / u_MapInfo.xyz, u_ViewOrigin / u_MapInfo.xyz);
	return pos;
}

vec2 WorldToScreen(vec3 worldPos)
{
	vec4 hpos = u_ModelViewProjectionMatrix * vec4(worldPos, 1.0);

	/*
	vec2 pos;

	// transform to UV coords
	hpos.w = 0.5 / hpos.z;

	pos.x = 0.5 + hpos.x * hpos.w;
	pos.y = 0.5 + hpos.y * hpos.w;
	*/
	hpos.xyz /= hpos.w;
	hpos.xy = hpos.xy * 0.5 + vec2(0.5);

	return hpos.xy;
}

vec2 mvpPosToScreen(vec4 mvpPos)
{
	vec4 hpos = mvpPos;
	vec2 pos;

	// transform to UV coords
	hpos.w = 0.5 / hpos.z;

	pos.x = 0.5 + hpos.x * hpos.w;
	pos.y = 0.5 + hpos.y * hpos.w;

	return pos;
}

float map(vec4 pos)
{
	//vec2 screenPos = WorldToScreen(pos);
	vec2 screenPos = mvpPosToScreen(pos);
	vec4 pPos = u_ModelViewProjectionMatrix * vec4(GetPosAndDepth(screenPos).xyz, 1.0);
	//vec3 pPos = GetPosAndDepth(screenPos).xyz;
	float d1 = pPos.y;
	float d2 = length(pPos.xyz - vec3(1.0, 0.5, 0.0)) - 0.5;
	return min(d1, d2);
}

float shadow(vec3 pPos, vec3 light)
{
	vec4 p = u_ModelViewProjectionMatrix * vec4(pPos, 1.0);
	vec4 l = u_ModelViewProjectionMatrix * vec4(light, 1.0);
	vec3 dr = normalize(p.xyz - l.xyz);
	float dst = 0.0;
	float res = 1.0;
	for (int i = 0; i < 100; ++i) {
		float dt = map(l);
		l.xyz += dr * dt * 0.8;
		dst += dt * 0.8;
		if (dt < 0.0001) {
			if (distance(l.xyz, p.xyz) < 0.001) {
				return res;
			}
			else {
				return 0.0;
			}
		}
		res = min(res, 4.0 * dt * dst / length(p.xyz - l.xyz));
	}
	return res;// * l.w;
}

vec4 positionMapAtCoord(vec2 coord)
{
	return texture2D(u_PositionMap, coord).xyza;
}

void main(void)
{
	gl_FragColor = texture2D(u_DiffuseMap, var_TexCoords);
}

#endif // NO TEST SHADER DEFINED
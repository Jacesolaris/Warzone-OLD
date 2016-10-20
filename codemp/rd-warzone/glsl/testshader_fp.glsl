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

float znear = u_ViewInfo.x; //camera clipping start
float zfar = u_ViewInfo.y; //camera clipping end

//#define		C_HBAO_ZFAR u_ViewInfo.y
//#define		C_HBAO_ZFAR 2048.0
#define		C_HBAO_ZFAR 256.0
//#define		C_HBAO_ZFAR 1.0
//#define		C_HBAO_ZFAR u_Local0.r

#define		fvTexelSize (vec2(1.0) / u_Dimensions.xy)

float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}

float linearizeDepth(float depth)
{
	float d = depth;
	d /= C_HBAO_ZFAR - depth * C_HBAO_ZFAR + depth;
	return clamp(d, 0.0, 1.0);
	//return linearize(depth);
}

vec3 normal_from_depth(float depth, vec2 texcoords, vec3 fakeNormals) {
	vec2 offset1 = vec2(0.0, fvTexelSize.y);
	vec2 offset2 = vec2(fvTexelSize.x, 0.0);

	float depth1 = linearizeDepth(texture2D(u_ScreenDepthMap, texcoords + offset1).r);
	float depth2 = linearizeDepth(texture2D(u_ScreenDepthMap, texcoords + offset2).r);

	vec3 p1 = vec3(offset1, depth1 - depth);
	vec3 p2 = vec3(offset2, depth2 - depth);

	vec3 normal = cross(p1, p2);
	if (u_Local0.g == 1.0)
		normal.y = -normal.y;

	normal.z = -normal.z;

	//normal *= fakeNormals;

	return normalize(normal);
}

vec4 ConvertToNormals(vec4 color)
{
	// This makes silly assumptions, but it adds variation to the output. Hopefully this will look ok without doing a crapload of texture lookups or
	// wasting vram on real normals.
	//
	// UPDATE: In my testing, this method looks just as good as real normal maps. I am now using this as default method unless r_normalmapping >= 2
	// for the very noticable FPS boost over texture lookups.

	//N = vec3((color.r + color.b) / 2.0, (color.g + color.b) / 2.0, (color.r + color.g) / 2.0);
	vec3 N = vec3(clamp(color.r + color.b, 0.0, 1.0), clamp(color.g + color.b, 0.0, 1.0), clamp(color.r + color.g, 0.0, 1.0));

	N.xy = pow(1.0 - N.xy, vec2(u_Local0.a));

	float displacement = clamp(length(color.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	displacement = clamp((clamp(displacement - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

	//vec4 norm = vec4(N, ((1.0 - (N.x * N.y * N.z)) + (1.0 - (length(N.xyz) / 3.0))) / 2.0);
	//vec4 norm = vec4(N, 1.0 - clamp(length(color.xyz), 0.0, 1.0));
	vec4 norm = vec4(N, displacement);
	return norm;
}

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the linear system
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
	return mat3(T * invmax, B * invmax, N);
}

void main(void)
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);
	//vec4 norm = texture2D(u_NormalMap, var_TexCoords);
	vec4 norm = ConvertToNormals(color);
	vec4 cNorm = norm;
	vec4 position = texture2D(u_PositionMap, var_TexCoords);

	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);
	//vec3 N = norm.xyz * E * u_Local1.a;// * 2.0 - 1.0;

	float depth = linearizeDepth(texture2D(u_ScreenDepthMap, var_TexCoords).r);
	norm.xyz = normal_from_depth(depth, var_TexCoords, vec3(1.0, 1.0, 1.0));
	//mat3 TBN = cotangent_frame(-norm.xyz, -E, var_TexCoords);
	//vec3 N = normalize(TBN * -cNorm.xyz * 0.004);
	vec3 N = norm.xyz * 0.004;

	//if (u_Local0.b == 1.0)
	N.z = sqrt(1.0 - dot(N.xy, N.xy));

	//float depth = linearizeDepth(texture2D(u_ScreenDepthMap, var_TexCoords).r);
	//vec3 N = (normal_from_depth(depth, var_TexCoords, norm.rgb) * u_Local1.a) * 2.0 - 1.0 /** 0.004*/;

	//gl_FragColor = vec4(norm.xyz * 0.5 + 0.5, 1.0);
	//gl_FragColor = vec4(N.rgb * 0.5 + 0.5, 1.0);
	//gl_FragColor = vec4(depth, depth, depth, 1.0);
	//return;

	gl_FragColor = vec4(color.rgb, 1.0);
	//return;

	float phongFactor = u_Local2.r;
	/*vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	float lambertian2 = dot(PrimaryLightDir.xyz,N);
	float spec2 = 0.0;
	bool noSunPhong = false;

	if (phongFactor < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
	noSunPhong = true;
	phongFactor = 0.0;
	}

	if(lambertian2 > 0.0)
	{// this is blinn phong
	vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
	float specAngle = max(dot(halfDir2, N), 0.0);
	spec2 = pow(specAngle, 16.0);
	gl_FragColor.rgb += vec3(spec2 * (1.0 - norm.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor;
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
	phongFactor = -u_Local2.r;
	}*/

	if (u_lightCount > 0)
	{
		for (int li = 0; li < u_lightCount; li++)
		{
			vec3 lightDir = normalize(u_lightPositions2[li] - position.xyz);
			float lambertian3 = dot(lightDir.xyz, N);
			float spec3 = 0.0;

			if (lambertian3 > 0.0)
			{
				float lightDist = distance(u_lightPositions2[li], position.xyz);
				float lightMax = u_lightDistances[li] * u_Local1.r;

				if (lightDist < lightMax)
				{
					float lightStrength = 1.0 - (lightDist / lightMax);
					lightStrength = pow(lightStrength * u_Local1.g, u_Local1.b);

					if (lightStrength > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, N), 0.0);
						spec3 = pow(specAngle3, 16.0);
						gl_FragColor.rgb += (clamp(gl_FragColor.rgb + vec3(0.3), 0.3, 0.7)) * u_lightColors[li].rgb * spec3 * lightStrength * phongFactor * u_Local0.r;
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
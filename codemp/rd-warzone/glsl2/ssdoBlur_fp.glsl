uniform sampler2D			u_DiffuseMap;			// Screen
uniform sampler2D			u_PositionMap;			// Positions
uniform sampler2D			u_NormalMap;			// Normals
uniform sampler2D			u_DeluxeMap;			// Occlusion

uniform vec2				u_Dimensions;
uniform vec3				u_ViewOrigin;
uniform vec4				u_PrimaryLightOrigin;

uniform vec4				u_Local0;

#define dir					u_Local0.rg

varying vec2   				var_TexCoords;

#define znear				u_ViewInfo.r			//camera clipping start
#define zfar				u_ViewInfo.g			//camera clipping end

//#define __ENCODE_NORMALS_RECONSTRUCT_Z__
#define __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
//#define __ENCODE_NORMALS_CRY_ENGINE__
//#define __ENCODE_NORMALS_EQUAL_AREA_PROJECTION__

#ifdef __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
vec2 EncodeNormal(vec3 n)
{
	float scale = 1.7777;
	vec2 enc = n.xy / (n.z + 1.0);
	enc /= scale;
	enc = enc * 0.5 + 0.5;
	return enc;
}
vec3 DecodeNormal(vec2 enc)
{
	vec3 enc2 = vec3(enc.xy, 0.0);
	float scale = 1.7777;
	vec3 nn =
		enc2.xyz*vec3(2.0 * scale, 2.0 * scale, 0.0) +
		vec3(-scale, -scale, 1.0);
	float g = 2.0 / dot(nn.xyz, nn.xyz);
	return vec3(g * nn.xy, g - 1.0);
}
#elif defined(__ENCODE_NORMALS_CRY_ENGINE__)
vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N * 4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);
	return vec3(encoded * g, 1.0 - f * 0.5);
}
vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}
#elif defined(__ENCODE_NORMALS_EQUAL_AREA_PROJECTION__)
vec2 EncodeNormal(vec3 n)
{
	float f = sqrt(8.0 * n.z + 8.0);
	return n.xy / f + 0.5;
}
vec3 DecodeNormal(vec2 enc)
{
	vec2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	vec3 n;
	n.xy = fenc*g;
	n.z = 1.0 - f / 2.0;
	return n;
}
#else //__ENCODE_NORMALS_RECONSTRUCT_Z__
vec3 DecodeNormal(in vec2 N)
{
	vec3 norm;
	norm.xy = N * 2.0 - 1.0;
	norm.z = sqrt(1.0 - dot(norm.xy, norm.xy));
	return norm;
}
vec2 EncodeNormal(vec3 n)
{
	return vec2(n.xy * 0.5 + 0.5);
}
#endif //__ENCODE_NORMALS_RECONSTRUCT_Z__

vec4 dssdo_blur(vec2 tex)
{
	float weights[9] = float[]
	(
		0.013519569015984728,
		0.047662179108871855,
		0.11723004402070096,
		0.20116755999375591,
		0.240841295721373,
		0.20116755999375591,
		0.11723004402070096,
		0.047662179108871855,
		0.013519569015984728
	);

	vec4 pMap  = texture(u_PositionMap, var_TexCoords);

	if (pMap.a-1.0 == 1024.0 || pMap.a-1.0 == 1025.0 /*|| pMap.a-1.0 == 21 || pMap.a-1.0 == 16 || pMap.a-1.0 == 30 || pMap.a-1.0 == 25*/)
	{// Skybox... Skip...
		return vec4(0.0, 0.0, 0.0, 1.0);
	}

	float indices[9] = float[](-4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0);

	vec2 step = dir/u_Dimensions.xy;

	int i;

	vec3 normal[9];

//#pragma unroll 9
	for (i = 0; i < 9; i++)
	{
		normal[i] = texture(u_NormalMap, tex + indices[i]*step).xyz;// * 2.0 - 1.0;
		normal[i] = DecodeNormal(normal[i].xy);
	}

	float total_weight = 1.0;
	float discard_threshold = 0.85;

//#pragma unroll 9
	for (i = 0; i < 9; i++)
	{
		if( dot(normal[i], normal[4]) < discard_threshold )
		{
			total_weight -= weights[i];
			weights[i] = 0;
		}
	}

	//

	vec4 res = vec4(0.0);

//#pragma unroll 9
	for (i = 0; i < 9; i++)
	{
		res += texture(u_DeluxeMap, tex + indices[i]*step) * weights[i];
	}

	res /= total_weight;

	return vec4(res.rgb, 1.0);
}

void main() 
{
	gl_FragColor = dssdo_blur(var_TexCoords);
}

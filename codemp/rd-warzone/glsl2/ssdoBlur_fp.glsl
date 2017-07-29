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

	if (pMap.a == 1024.0 || pMap.a == 1025.0 /*|| pMap.a == 21 || pMap.a == 16 || pMap.a == 30 || pMap.a == 25*/)
	{// Skybox... Skip...
		return vec4(0.0, 0.0, 0.0, 1.0);
	}

	float indices[9] = float[](-4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0);

	vec2 step = dir/u_Dimensions.xy;

	int i;

	vec3 normal[9];

#pragma unroll 9
	for (i = 0; i < 9; i++)
	{
		normal[i] = texture(u_NormalMap, tex + indices[i]*step).xyz * 2.0 - 1.0;
	}

	float total_weight = 1.0;
	float discard_threshold = 0.85;

#pragma unroll 9
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

#pragma unroll 9
	for (i = 0; i < 9; i++)
	{
		res += texture(u_DeluxeMap, tex + indices[i]*step) * weights[i];
	}

	res /= total_weight;

	return res;
}

void main() 
{
	gl_FragColor = dssdo_blur(var_TexCoords);
}

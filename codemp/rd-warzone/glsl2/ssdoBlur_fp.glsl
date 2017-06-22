uniform sampler2D			u_DiffuseMap;			// Screen
uniform sampler2D			u_PositionMap;			// Positions
uniform sampler2D			u_NormalMap;			// Normals
uniform sampler2D			u_DeluxeMap;			// Occlusion

uniform vec2				u_Dimensions;
uniform vec3				u_ViewOrigin;
uniform vec4				u_PrimaryLightOrigin;

uniform vec4				u_Local0;
uniform vec4				u_Local1;

#define dir					u_Local0.rg

varying vec2   				var_TexCoords;
varying vec3   				var_LightPos;

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

	float indices[9] = float[](-4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0);

	vec2 step = dir/u_Dimensions.xy;

	vec3 normal[9];

	normal[0] = texture(u_NormalMap, tex + indices[0]*step).xyz;
	normal[1] = texture(u_NormalMap, tex + indices[1]*step).xyz;
	normal[2] = texture(u_NormalMap, tex + indices[2]*step).xyz;
	normal[3] = texture(u_NormalMap, tex + indices[3]*step).xyz;
	normal[4] = texture(u_NormalMap, tex + indices[4]*step).xyz;
	normal[5] = texture(u_NormalMap, tex + indices[5]*step).xyz;
	normal[6] = texture(u_NormalMap, tex + indices[6]*step).xyz;
	normal[7] = texture(u_NormalMap, tex + indices[7]*step).xyz;
	normal[8] = texture(u_NormalMap, tex + indices[8]*step).xyz;

	float total_weight = 1.0;
	float discard_threshold = 0.85;

	int i;

	for( i=0; i<9; ++i )
	{
		if( dot(normal[i], normal[4]) < discard_threshold )
		{
			total_weight -= weights[i];
			weights[i] = 0;
		}
	}

	//

	vec4 res = 0;

	for( i=0; i<9; ++i )
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

uniform vec4		u_Local9;
uniform vec4		u_Local10;

varying vec2		var_TexCoords;
varying vec3		var_vertPos;
varying vec3		var_Normal;
flat varying float	var_IsWater;

out vec4 out_Glow;
out vec4 out_Normal;
out vec4 out_Position;
out vec4 out_NormalDetail;

// Maximum waves amplitude
#define maxAmplitude u_Local10.g

void main()
{
	out_Glow = vec4(0.0);
	out_Normal = vec4(var_Normal * 0.5 + 0.5, 1.0);
	out_NormalDetail = vec4(0.0);
	out_Color = vec4(0.0059, 0.3096, 0.445, 0.5);
	out_Position = vec4(var_vertPos.xyz, var_IsWater);
}

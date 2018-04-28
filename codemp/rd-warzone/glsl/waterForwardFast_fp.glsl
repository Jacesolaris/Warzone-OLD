uniform vec4		u_Local9; // testvalue1-3, MAP_WATER_LEVEL
uniform vec4		u_Local10;

varying vec3		var_vertPos;
varying vec3		var_Normal;

out vec4 out_Glow;
out vec4 out_Normal;
out vec4 out_Position;
#ifdef __USE_REAL_NORMALMAPS__
out vec4 out_NormalDetail;
#endif //__USE_REAL_NORMALMAPS__

#define MAP_WATER_LEVEL u_Local9.a

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

void main()
{
	out_Color = vec4(0.0059, 0.3096, 0.445, 1.0);
	out_Position = vec4(var_vertPos.xyz, 1.0);
	//out_Position = vec4(0.0, 0.0, MAP_WATER_LEVEL, 1.0);
	out_Glow = vec4(0.0);
	out_Normal = vec4(EncodeNormal(var_Normal), 0.0, 1.0);
#ifdef __USE_REAL_NORMALMAPS__
	out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
}

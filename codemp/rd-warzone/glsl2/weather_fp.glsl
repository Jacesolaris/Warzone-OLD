uniform sampler2D	u_DiffuseMap;

uniform vec4		u_Color;
uniform vec4		u_Local0; // normals, 0.0

out vec4			out_Glow;
out vec4			out_Position;
out vec4			out_Normal;
out vec4			out_NormalDetail;

varying vec2		var_Tex1;
varying vec3		var_Position;
//varying vec4		var_Color;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

void main()
{
	gl_FragColor = texture2D(u_DiffuseMap, var_Tex1);// * u_Color;//var_Color;
	out_Glow = vec4(0.0);
	out_Position = vec4(0.0);//vec4(var_Position.xyz, MATERIAL_GLASS+1.0);
	out_Normal = vec4(0.0);//vec4(EncodeNormal(u_Local0.xyz), 0.0, 1.0);
	out_NormalDetail = vec4(0.0);
}

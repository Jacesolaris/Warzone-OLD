uniform sampler2D	u_DiffuseMap;

uniform vec4		u_Color;
uniform vec4		u_Local0; // normals, 0.0

out vec4			out_Glow;
out vec4			out_Position;
out vec4			out_Normal;
#ifdef __USE_REAL_NORMALMAPS__
out vec4			out_NormalDetail;
#endif //__USE_REAL_NORMALMAPS__


varying vec2		var_Tex1;
varying vec3		var_Position;
//varying vec4		var_Color;

void main()
{
	gl_FragColor = texture2D(u_DiffuseMap, var_Tex1);
	gl_FragColor.a = clamp(gl_FragColor.a * 3.0, 0.0, 1.0);
	out_Glow = vec4(0.0);
	out_Position = vec4(0.0);
	out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
	out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
}

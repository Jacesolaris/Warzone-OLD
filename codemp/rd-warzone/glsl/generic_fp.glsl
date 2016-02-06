uniform sampler2D	u_DiffuseMap;

varying vec2		var_DiffuseTex;
varying vec4		var_Color;

out vec4 out_Glow;

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_DiffuseTex);

	gl_FragColor = color * var_Color;

#if defined(USE_GLOW_BUFFER)
	out_Glow = gl_FragColor;
#else
	out_Glow = vec4(0.0);
#endif
}

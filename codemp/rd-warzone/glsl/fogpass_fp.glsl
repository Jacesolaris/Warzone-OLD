uniform vec4  u_Color;

varying float var_Scale;

out vec4 out_Glow;
out vec4 out_Normal;

void main()
{
	gl_FragColor = u_Color;
	gl_FragColor.a = sqrt(clamp(var_Scale, 0.0, 1.0));

#if defined(USE_GLOW_BUFFER)
	out_Glow = gl_FragColor;
#else
	out_Glow = vec4(0.0);
#endif

	//out_Normal = vec4(0.0);
}

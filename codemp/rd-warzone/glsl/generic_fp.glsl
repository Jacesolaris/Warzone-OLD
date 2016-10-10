uniform sampler2D	u_DiffuseMap;

varying vec2		var_DiffuseTex;
varying vec4		var_Color;
varying vec3		var_Normal;
varying vec3		var_VertPos;

out vec4 out_Glow;
//out vec4 out_Position;
//out vec4 out_Normal;

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_DiffuseTex);
	color = texture2D(u_DiffuseMap, var_DiffuseTex) * var_Color;
	
	//if (color.a <= 0.0) discard;

	gl_FragColor = color;

	//gl_FragColor.rgb = vec3(1.0, 0.0, 0.0);

	#if defined(USE_GLOW_BUFFER)
		out_Glow = gl_FragColor;
	#else
		out_Glow = vec4(0.0);
	#endif

	//out_Normal = vec4(var_Normal.xyz * 0.5 + 0.5, 0.05);
	//out_Position = vec4(var_VertPos, 0.0);
}

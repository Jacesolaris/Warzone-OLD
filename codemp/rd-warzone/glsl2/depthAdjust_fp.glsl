uniform sampler2D	u_ScreenDepthMap;
//uniform sampler2D	u_PositionMap;

varying vec2		var_Tex;

void main()
{
	float depth = texture(u_ScreenDepthMap, var_Tex).r;
	
	if (depth >= 1.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		discard;
	}

	/*
	float material = texture(u_PositionMap, var_Tex).a;

	if (material-1.0 == MATERIAL_SKY || material-1.0 == MATERIAL_SUN)// || material-1.0 == MATERIAL_NONE)
	{// Skybox... Skip...
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		//gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		discard;
	}
	*/
}

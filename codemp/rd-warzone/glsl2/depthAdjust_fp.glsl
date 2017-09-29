//uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;

varying vec2		var_Tex;

void main()
{
	//float depth = texture2D(u_ScreenDepthMap, var_Tex).r;
	vec4 pMap = textureLod(u_PositionMap, var_Tex, 0.0);
	float material = pMap.a;
	
	//if (depth >= 0.98) depth = 0.0;

	gl_FragColor = vec4(pMap.xyz, 1.0);
	return;

	if (material-1.0 == MATERIAL_SKY || material-1.0 == MATERIAL_SUN)// || material-1.0 == MATERIAL_NONE)
	{// Skybox... Skip...
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		//gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		discard;
	}
}

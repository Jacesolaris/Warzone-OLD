uniform sampler2D u_DiffuseMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap


uniform vec2				u_AlphaTestValues;

#define ATEST_NONE	0
#define ATEST_LT	1
#define ATEST_GT	2
#define ATEST_GE	3


varying vec2	var_TexCoords;
varying vec3	var_Position;
varying vec3	var_Normal;
varying vec4	var_Color;

out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;

void main()
{
	vec2 texCoords = var_TexCoords;

	if (u_Local4.a > 0.0)
	{// Sway...
		texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - texCoords.y) + 1.0), 0.0);
	}

	gl_FragColor = texture(u_DiffuseMap, texCoords);
	gl_FragColor.a *= var_Color.a;

	if (u_AlphaTestValues.r > 0.0)
	{
		if (u_AlphaTestValues.r == ATEST_LT)
			if (gl_FragColor.a >= u_AlphaTestValues.g)
				discard;
		if (u_AlphaTestValues.r == ATEST_GT)
			if (gl_FragColor.a <= u_AlphaTestValues.g)
				discard;
		if (u_AlphaTestValues.r == ATEST_GE)
			if (gl_FragColor.a < u_AlphaTestValues.g)
				discard;
	}

	out_Glow = vec4(0.0);
	out_Position = vec4(var_Position.rgb, 1024.0);
	out_Normal = vec4(var_Normal.rgb * 0.5 + 0.5, 0.0);
}

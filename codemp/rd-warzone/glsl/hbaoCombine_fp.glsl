uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

uniform vec2		u_Dimensions;

varying vec2		var_ScreenTex;

void main()
{
	vec4 color = texture2D(u_DiffuseMap, var_ScreenTex);
	vec3 hbao = texture2D(u_NormalMap, var_ScreenTex).rgb;
	gl_FragColor = vec4(color.rgb * (hbao.rgb * 0.7), color.a);
}

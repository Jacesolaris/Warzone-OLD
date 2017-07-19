uniform sampler2D	u_TextureMap;

varying vec2		var_TexCoords;

void main()
{
	vec4 color = texture2D(u_TextureMap, var_TexCoords);

#define const_1 ( 12.0 / 255.0)
#define const_2 (255.0 / 229.0)
	gl_FragColor.rgb = clamp((clamp(color.rgb - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);
	gl_FragColor.a = 1.0;
}

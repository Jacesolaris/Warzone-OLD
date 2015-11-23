uniform sampler2D u_TextureMap;

varying vec2		var_TexCoords;
varying vec2		var_Dimensions;
varying vec2		var_ScreenTex;

uniform vec2		u_Dimensions;
uniform mat4		u_invEyeProjectionMatrix;
uniform vec4		u_Local0;

vec2 PixelSize = vec2(1.0f / var_Dimensions.x, 1.0f / var_Dimensions.y);

#define   MAGICDETAIL_STRENGTH u_Local0.x

vec3 GenerateDetail( vec2 fragCoord )
{
	float M =abs(length(texture2D(u_TextureMap, fragCoord + vec2(0., 0.)*PixelSize).rgb) / 3.0);
	float L =abs(length(texture2D(u_TextureMap, fragCoord + vec2(1.0, 0.)*PixelSize).rgb) / 3.0);
	float R =abs(length(texture2D(u_TextureMap, fragCoord + vec2(-1.0, 0.)*PixelSize).rgb) / 3.0);	
	float U =abs(length(texture2D(u_TextureMap, fragCoord + vec2(0., 1.0)*PixelSize).rgb) / 3.0);;
	float D =abs(length(texture2D(u_TextureMap, fragCoord + vec2(0., -1.0)*PixelSize).rgb) / 3.0);
	float X = ((R-M)+(M-L))*0.5;
	float Y = ((D-M)+(M-U))*0.5;
	
	vec4 N = vec4(normalize(vec3(X, Y, MAGICDETAIL_STRENGTH)), 1.0);

	vec4 col = vec4(N.xyz * 0.5 + 0.5, 1.0);
	return col.rgb;
}

void main()
{
	vec4 inColor = texture2D(u_TextureMap, var_TexCoords);
	vec4 mult = vec4(GenerateDetail(var_TexCoords.xy), 1.0);
	//gl_FragColor.rgb = mult.rgb;
	gl_FragColor.rgb = inColor.rgb * ((mult.r + mult.g + mult.b) / 3.0);
	gl_FragColor.rgb = gl_FragColor.rgb * ((mult.r + mult.g + mult.b) / 3.0);
	gl_FragColor.rgb = ((inColor.rgb * 2.0) + gl_FragColor.rgb) / 3.0;
	gl_FragColor.a = 1.0;
}

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;

varying vec2		var_TexCoords;

#define E_CC_PALETTE

void main (void)
{
	vec4 color = texture2D(u_DiffuseMap, var_TexCoords);

#ifdef E_CC_PALETTE
	//adaptation in time
	color.rgb = clamp(color.rgb, 0.0, 1.0);

	vec3 brightness = texture2D(u_GlowMap, vec2(0.5)).rgb;//adaptation luminance
	brightness = (brightness/(brightness+1.0));//new version
	brightness = vec3(max(brightness.x, max(brightness.y, brightness.z)));//new version

	vec3	palette;
	vec2	uvsrc = vec2(0.0);

	uvsrc.y = brightness.r;
	uvsrc.x = color.r;
	palette.r = texture(u_DeluxeMap, uvsrc).r;
	uvsrc.x = color.g;
	uvsrc.y = brightness.g;
	palette.g = texture2D(u_DeluxeMap, uvsrc).g;
	uvsrc.x = color.b;
	uvsrc.y = brightness.b;
	palette.b = texture2D(u_DeluxeMap, uvsrc).b;
	color.rgb = palette.rgb;
#endif //E_CC_PALETTE

	gl_FragColor = color;
}

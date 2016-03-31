uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_OverlayMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif

uniform vec4		u_Local9;

flat in int			bUnderwater;
smooth in vec2		vTexCoord;

out vec4			out_Glow;
out vec4			out_DetailedNormal;
//out vec4			out_PositionMap;
out vec4			out_FoliageMap;

void main() 
{
	vec4 diffuse;
	vec3 m_Normal = (vec3(clamp(length(1.0-diffuse.rgb) * 0.333, 0.0, 1.0)) * 2.0 - 1.0);

	if (bUnderwater == 1)
		diffuse = texture(u_OverlayMap, vTexCoord);
	else
		diffuse = texture(u_DiffuseMap, vTexCoord);

	diffuse.rgb *= clamp((1.0-vTexCoord.y) * 1.5, 0.3, 1.0);
	if (diffuse.a > 0.5) diffuse.a = 1.0;

	#if defined(USE_SHADOWMAP)

		vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
		float shadowValue = texture2D(u_ShadowMap, shadowTex).r;
		diffuse.rgb *= clamp(shadowValue, 0.4, 1.0);

	#endif //defined(USE_SHADOWMAP)

	gl_FragColor = diffuse;
	
	out_Glow = vec4(0.0);

	out_DetailedNormal = vec4(m_Normal.xyz, 0.1);
	//out_PositionMap = vec4(gl_FragCoord.xyz, 0.0);
	out_FoliageMap.rgba = vec4(1.0, 1.0, 0.0, 0.0);
}

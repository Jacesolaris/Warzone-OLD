uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_OverlayMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif

uniform mat4		u_ModelViewProjectionMatrix;
uniform mat4		u_ModelMatrix;
uniform mat4		u_invEyeProjectionMatrix;
uniform mat4		u_ModelViewMatrix;

uniform vec4		u_Local9;

flat in int			bUnderwater;
smooth in vec2		vTexCoord;
in vec3				vVertPosition;

out vec4			out_Glow;
out vec4			out_Normal;
out vec4			out_Position;

void main() 
{
	vec4 diffuse;
	vec3 m_Normal = (vec3(clamp(length(1.0-diffuse.rgb) * 0.333, 0.0, 1.0)) * 2.0 - 1.0);

	if (bUnderwater >= 1)
		diffuse = texture(u_OverlayMap, vTexCoord);
	else
		diffuse = texture(u_DiffuseMap, vTexCoord);

	diffuse.rgb *= clamp((1.0-vTexCoord.y) * 1.5, 0.3, 1.0);
	if (diffuse.a > 0.5) diffuse.a = 1.0;
	else diffuse.a = 0.0;

	#if defined(USE_SHADOWMAP)

		vec2 shadowTex = gl_FragCoord.xy * r_FBufScale;
		float shadowValue = texture2D(u_ShadowMap, shadowTex).r;
		diffuse.rgb *= clamp(shadowValue, 0.4, 1.0);

	#endif //defined(USE_SHADOWMAP)

	gl_FragColor = diffuse;

	if (diffuse.a <= 0.0) discard;
	
	out_Glow = vec4(0.0);
	out_Normal = vec4(m_Normal.xyz, 0.1);
	out_Position = vec4(vVertPosition, 0.1875); // 6.0 / MATERIAL_LAST (0.1875) is MATERIAL_LONGGRASS
}

uniform sampler2D	u_DiffuseMap;

uniform vec4		u_Local9;

smooth in vec2		vTexCoord;

out vec4			out_Glow;
out vec4			out_Normal;
//out vec4			out_Position;

void main() 
{
	vec4 diffuse = texture(u_DiffuseMap, vTexCoord);
	diffuse.rgb *= clamp((1.0-vTexCoord.y) * 1.5, 0.3, 1.0);
	if (diffuse.a > 0.5) diffuse.a = 1.0;
	gl_FragColor = diffuse;
	
	vec3 m_Normal = (vec3(clamp(length(1.0-diffuse.rgb) * 0.333, 0.0, 1.0)) * 2.0 - 1.0);

	out_Glow = vec4(0.0);

	out_Normal = vec4(m_Normal.xyz, 0.1);
	//out_Position = vec4(gl_FragCoord.xyz, 0.0);
}

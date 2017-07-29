attribute vec3		attr_Position;
attribute vec3		attr_Normal;
attribute vec2		attr_TexCoord0;

uniform vec4		u_PrimaryLightOrigin;

out vec3 v_g_normal;
out vec2 v_g_texCoord;
out vec3 v_g_PrimaryLightDir;

void main()
{
	v_g_normal = attr_Normal * 2.0 - 1.0;
	v_g_texCoord = attr_TexCoord0;
	v_g_PrimaryLightDir = u_PrimaryLightOrigin.xyz - (attr_Position.xyz * u_PrimaryLightOrigin.w);
	gl_Position = vec4(attr_Position.xyz, 1.0);
}

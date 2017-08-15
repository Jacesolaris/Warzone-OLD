layout(triangles) in;

layout(triangle_strip, max_vertices = 4) out;

uniform mat4			u_ModelViewProjectionMatrix;

uniform sampler2D		u_SplatControlMap;

uniform vec4			u_Local6;
uniform vec4			u_Local10;

#define FOLIAGE_LAYERS	4
#define FOLIAGE_HEIGHT	u_Local10.r

in vec3					v_g_normal[3];
in vec2					v_g_texCoord[3];
in vec3					v_g_PrimaryLightDir[3];
in vec3					v_g_ViewDir[3];

out vec4				v_position;
out vec3				v_normal;
out vec2				v_texCoord;
out vec3				v_PrimaryLightDir;
flat out int			v_foliageLayer;

vec4 GetControlMap(vec3 m_vertPos)
{
	float scale = 1.0 / u_Local6.b; /* control scale */
	float offset = (u_Local6.b / 2.0) * scale;
	vec4 xaxis = texture( u_SplatControlMap, (m_vertPos.yz * scale) + offset);
	vec4 yaxis = texture( u_SplatControlMap, (m_vertPos.xz * scale) + offset);
	vec4 zaxis = texture( u_SplatControlMap, (m_vertPos.xy * scale) + offset);
	vec4 control = xaxis * 0.333 + yaxis * 0.333 + zaxis * 0.333;
	control = clamp(control * u_Local10.b, 0.0, 1.0);
	return control;
}

vec4 GetGrassMap(vec3 m_vertPos)
{
	vec4 control = GetControlMap(m_vertPos);
	return control;
}

void main(void)
{
	vec3 normal;

	const float FOLIAGE_DELTA = 1.0 / float(FOLIAGE_LAYERS);
	
	float d = 0.0;

	vec3 Vert1 = gl_in[0].gl_Position.xyz;
	vec3 Vert2 = gl_in[1].gl_Position.xyz;
	vec3 Vert3 = gl_in[2].gl_Position.xyz;
	vec3 Pos = (Vert1 + Vert2 + Vert3) / 3.0;   //Center of the triangle - copy for later

	vec4 grassMap = GetGrassMap(Pos);

	int grassMap2[4];
	//grassMap2[0] = (grassMap.r >= 0.2 || grassMap.g >= 0.2 || grassMap.b >= 0.2) ? 1 : 0;
	grassMap2[0] = 0;
	grassMap2[1] = grassMap.r >= 0.3 ? 1 : 0;
	grassMap2[2] = grassMap.g >= 0.3 ? 1 : 0;
	grassMap2[3] = grassMap.b >= 0.3 ? 1 : 0;

	for (int foliageLayer = 0; foliageLayer < FOLIAGE_LAYERS; foliageLayer++)
	{
		if (grassMap2[foliageLayer] < 1)
			continue;

		d += FOLIAGE_DELTA;

		for(int i = 0; i < gl_in.length(); i++)
		{
			v_normal = normalize(v_g_normal[i]);
			v_texCoord = v_g_texCoord[i];
			v_position = (gl_in[i].gl_Position + vec4(v_normal * d * FOLIAGE_HEIGHT, 0.0));
			gl_Position = u_ModelViewProjectionMatrix * v_position;
			v_foliageLayer = foliageLayer;
	
			EmitVertex();
		}
		
		EndPrimitive();
	}
}

uniform sampler2D	u_DiffuseMap;

uniform vec4		u_Local10; // foliageLODdistance, foliageDensity, doSway, overlaySway

//#define USE_LIGHTING

uniform vec3		u_PrimaryLightColor;
uniform vec3		u_PrimaryLightAmbient;
varying vec4		var_PrimaryLightDir;
varying vec3		var_ViewDir;


smooth in vec2 vTexCoord;
//smooth in vec3 vWorldPos;
//smooth in vec4 vEyeSpacePos;



#define m_TexCoords vTexCoord
#define m_Normal vec3(1.0)
//#define m_vertPos vWorldPos
#define m_ViewDir var_ViewDir


out vec4 out_Glow;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;


void main() 
{
	//gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	//gl_FragColor = vec4(0.0, 0.0, m_TexCoords.y, 1.0);
	//gl_FragColor = texture(u_DiffuseMap, m_TexCoords);

	vec2 texCoords = m_TexCoords + vec2(u_Local10.b * u_Local10.a * ((1.0 - m_TexCoords.y) + 1.0), 0.0);
	gl_FragColor = texture(u_DiffuseMap, texCoords);
	

#if 0
#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		vec3 viewDir = vec3(0.0);

		mat3 tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, m_Normal.xyz);
		viewDir = vec3(var_Normal2, var_Tangent.w, var_Bitangent.w);

		vec3 E = normalize(viewDir);

		float lambertian2 = dot(var_PrimaryLightDir.xyz, m_Normal.xyz);
		float spec2 = 0.0;

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 halfDir2 = normalize(var_PrimaryLightDir.xyz + E);
			float specAngle = max(dot(halfDir2, m_Normal.xyz), 0.0);
			spec2 = pow(specAngle, 16.0);
			gl_FragColor.rgb += vec3(spec2 * 0.25) * gl_FragColor.rgb * u_PrimaryLightColor.rgb;
		}
#endif
#endif

	out_Glow = vec4(0.0);

	out_DetailedNormal = vec4(m_Normal.xyz, 0.1);

	out_FoliageMap.r = 1.0;
	out_FoliageMap.g = 1.0;
}

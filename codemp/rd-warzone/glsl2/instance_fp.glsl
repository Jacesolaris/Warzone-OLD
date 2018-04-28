uniform sampler2D	u_DiffuseMap;

uniform vec4		u_Settings0;		// materialType, 0.0, 0.0, 0.0

uniform float		u_Time;

varying vec2		var_Tex1;
varying vec3		var_VertPos;
varying vec3		var_Normal;

out vec4			out_Glow;
out vec4			out_Position;
out vec4			out_Normal;
#ifdef __USE_REAL_NORMALMAPS__
out vec4			out_NormalDetail;
#endif //__USE_REAL_NORMALMAPS__

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

const float							fBranchHardiness = 0.001;
const float							fBranchSize = 128.0;
const float							fWindStrength = 12.0;
const vec3							vWindDirection = normalize(vec3(1.0, 1.0, 0.0));

vec2 GetSway()
{
	// Wind calculation stuff...
	float fWindPower = 0.5f + sin(var_VertPos.x / fBranchSize + var_VertPos.z / fBranchSize + u_Time*(1.2f + fWindStrength / fBranchSize));

	if (fWindPower < 0.0f)
		fWindPower = fWindPower*0.2f;
	else
		fWindPower = fWindPower*0.3f;

	fWindPower *= fWindStrength;

	return vWindDirection.xy*fWindPower*fBranchHardiness;
}

void main()
{
	vec2 tc = var_Tex1;

	if (u_Settings0.r == MATERIAL_GREENLEAVES)
	{
		tc += GetSway();
	}

	gl_FragColor = texture2D(u_DiffuseMap, tc);
	
	out_Glow = vec4(0.0);

	if (gl_FragColor.a >= 0.001)
	{
		out_Position = vec4(var_VertPos.xyz, u_Settings0.r + 1.0);
		out_Normal = vec4(vec3(EncodeNormal(var_Normal.xyz), 0.0), 1.0);
#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
	}
	else
	{
		out_Position = vec4(0.0);
		out_Normal = vec4(0.0);
#ifdef __USE_REAL_NORMALMAPS__
		out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
	}
}

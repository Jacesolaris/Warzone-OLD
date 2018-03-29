uniform sampler2D u_ShadowMap;

uniform vec3      u_LightForward;
uniform vec3      u_LightUp;
uniform vec3      u_LightRight;
uniform vec4      u_LightOrigin;
uniform float     u_LightRadius;
uniform vec4      u_Local0;			// PSHADOWMAP_SIZE, testvalue0, testvalue1, testvalue2

varying vec3      var_Position;
varying vec3      var_Normal;

out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;
out vec4 out_NormalDetail;

void main()
{
	vec3 lightToPos = var_Position - u_LightOrigin.xyz;
	float lightDist = length(lightToPos);

	vec2 st = vec2(-dot(u_LightRight, lightToPos), dot(u_LightUp, lightToPos));

	st = st * 0.5 + vec2(0.5);

	float intensity = 1.0 - clamp(lightDist / min(u_LightRadius, 256.0), 0.0, 1.0);

	if (intensity > 0.0)
	{
#if defined(USE_PCF)
		float pcf = float(texture(u_ShadowMap, st + vec2(-1.0 / u_Local0.r, -1.0 / u_Local0.r)).r != 1.0);
		pcf += float(texture(u_ShadowMap, st + vec2(1.0 / u_Local0.r, -1.0 / u_Local0.r)).r != 1.0);
		pcf += float(texture(u_ShadowMap, st + vec2(-1.0 / u_Local0.r, 1.0 / u_Local0.r)).r != 1.0);
		pcf += float(texture(u_ShadowMap, st + vec2(1.0 / u_Local0.r, 1.0 / u_Local0.r)).r != 1.0);
		pcf /= 4.0;
#else
		float pcf = float(texture(u_ShadowMap, st).r != 1.0);
#endif

		intensity *= pcf;
	}

	out_Color.rgb = vec3(.0, .0, .0);
	out_Color.a = clamp(pow(intensity, 8.0), 0.0, 0.5);
	out_Glow = vec4(0.0);
	out_Position = vec4(0.0);
	out_Normal = vec4(0.0);
	out_NormalDetail = vec4(0.0);
}

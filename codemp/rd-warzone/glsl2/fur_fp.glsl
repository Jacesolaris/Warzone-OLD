uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_SplatControlMap;

uniform vec4		u_Local10;

// The higher the value, the bigger the contrast between the fur length.
#define FUR_STRENGTH_CONTRAST u_Local10.g //2.0

// The higher the value, the less fur.
#define FUR_STRENGTH_CAP u_Local10.b //0.3

in vec4				v_position;
in vec3				v_normal;
in vec2				v_texCoord;
in vec3				v_PrimaryLightDir;

in float			v_furStrength;

out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;

vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}

void main()
{
	vec3 normal = normalize(v_normal);

	// Orthogonal fur to light is still illumintated. So shift by one, that only fur targeting away from the light do get darkened. 
	float intensity = clamp(dot(normal, v_PrimaryLightDir) + 1.0, 0.0, 1.0);
	
	float power = texture(u_SplatControlMap, v_texCoord).r;
	float furStrength = clamp(v_furStrength * power * FUR_STRENGTH_CONTRAST - FUR_STRENGTH_CAP, 0.0, 1.0);

	float color = texture(u_DiffuseMap, v_texCoord / u_Local10.a).r;
	gl_FragColor = vec4(vec3(0.0, color, 0.0) * intensity, furStrength > 0.5 ? 1.0 : 0.0);

	out_Position = vec4(v_position.xyz, furStrength > 0.5 ? 1.0 : 0.0);
	out_Normal = vec4(EncodeNormal(v_normal.xyz), 0.0, furStrength > 0.5 ? 1.0 : 0.0);
	out_Glow = vec4(0.0);
}

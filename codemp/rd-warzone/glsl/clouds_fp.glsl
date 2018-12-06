uniform vec4						u_Local2; // PROCEDURAL_CLOUDS_ENABLED, PROCEDURAL_CLOUDS_CLOUDSCALE, PROCEDURAL_CLOUDS_SPEED, PROCEDURAL_CLOUDS_DARK
uniform vec4						u_Local3; // PROCEDURAL_CLOUDS_LIGHT, PROCEDURAL_CLOUDS_CLOUDCOVER, PROCEDURAL_CLOUDS_CLOUDALPHA, PROCEDURAL_CLOUDS_SKYTINT
uniform vec4						u_Local5; // dayNightEnabled, nightScale, MAP_CLOUD_LAYER_HEIGHT, 0.0
uniform vec4						u_Local9; // testvalues

#define CLOUDS_ENABLED				u_Local2.r
#define CLOUDS_CLOUDSCALE			u_Local2.g
#define CLOUDS_SPEED				u_Local2.b
#define CLOUDS_DARK					u_Local2.a

#define CLOUDS_LIGHT				u_Local3.r
#define CLOUDS_CLOUDCOVER			u_Local3.g
#define CLOUDS_CLOUDALPHA			u_Local3.b
#define CLOUDS_SKYTINT				u_Local3.a

#define SHADER_DAY_NIGHT_ENABLED	u_Local5.r
#define SHADER_NIGHT_SCALE			u_Local5.g

uniform vec3						u_ViewOrigin;
uniform float						u_Time;

uniform vec4						u_PrimaryLightOrigin;
uniform vec3						u_PrimaryLightColor;


varying vec2						var_TexCoords;
varying vec3						var_vertPos;
varying vec3						var_Normal;


out vec4 out_Glow;
out vec4 out_Normal;
out vec4 out_Position;
#ifdef __USE_REAL_NORMALMAPS__
out vec4 out_NormalDetail;
#endif //__USE_REAL_NORMALMAPS__


//#define __ENCODE_NORMALS_RECONSTRUCT_Z__
#define __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
//#define __ENCODE_NORMALS_CRY_ENGINE__
//#define __ENCODE_NORMALS_EQUAL_AREA_PROJECTION__

#ifdef __ENCODE_NORMALS_STEREOGRAPHIC_PROJECTION__
vec2 EncodeNormal(vec3 n)
{
	float scale = 1.7777;
	vec2 enc = n.xy / (n.z + 1.0);
	enc /= scale;
	enc = enc * 0.5 + 0.5;
	return enc;
}
vec3 DecodeNormal(vec2 enc)
{
	vec3 enc2 = vec3(enc.xy, 0.0);
	float scale = 1.7777;
	vec3 nn =
		enc2.xyz*vec3(2.0 * scale, 2.0 * scale, 0.0) +
		vec3(-scale, -scale, 1.0);
	float g = 2.0 / dot(nn.xyz, nn.xyz);
	return vec3(g * nn.xy, g - 1.0);
}
#elif defined(__ENCODE_NORMALS_CRY_ENGINE__)
vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N * 4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);
	return vec3(encoded * g, 1.0 - f * 0.5);
}
vec2 EncodeNormal(in vec3 N)
{
	float f = sqrt(8.0 * N.z + 8.0);
	return N.xy / f + 0.5;
}
#elif defined(__ENCODE_NORMALS_EQUAL_AREA_PROJECTION__)
vec2 EncodeNormal(vec3 n)
{
	float f = sqrt(8.0 * n.z + 8.0);
	return n.xy / f + 0.5;
}
vec3 DecodeNormal(vec2 enc)
{
	vec2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	vec3 n;
	n.xy = fenc*g;
	n.z = 1.0 - f / 2.0;
	return n;
}
#else //__ENCODE_NORMALS_RECONSTRUCT_Z__
vec3 DecodeNormal(in vec2 N)
{
	vec3 norm;
	norm.xy = N * 2.0 - 1.0;
	norm.z = sqrt(1.0 - dot(norm.xy, norm.xy));
	return norm;
}
vec2 EncodeNormal(vec3 n)
{
	return vec2(n.xy * 0.5 + 0.5);
}
#endif //__ENCODE_NORMALS_RECONSTRUCT_Z__


const mat2 mc = mat2(1.6, 1.2, -1.2, 1.6);

vec2 hash(vec2 p) {
	p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise(in vec2 p) {
	const float K1 = 0.366025404; // (sqrt(3)-1)/2;
	const float K2 = 0.211324865; // (3-sqrt(3))/6;
	vec2 i = floor(p + (p.x + p.y)*K1);
	vec2 a = p - i + (i.x + i.y)*K2;
	vec2 o = (a.x>a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0); //vec2 of = 0.5 + 0.5*vec2(sign(a.x-a.y), sign(a.y-a.x));
	vec2 b = a - o + K2;
	vec2 c = a - 1.0 + 2.0*K2;
	vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
	vec3 n = h*h*h*h*vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
	return dot(n, vec3(70.0));
}

float fbm(vec2 n) {
	float total = 0.0, amplitude = 0.1;
	for (int i = 0; i < 7; i++) {
		total += noise(n) * amplitude;
		n = mc * n;
		amplitude *= 0.4;
	}
	return total;
}

vec4 Clouds(in vec2 fragCoord)
{
	vec2 p = fragCoord.xy;
	vec2 uv = p;
	float time = u_Time * CLOUDS_SPEED;
	float q = fbm(uv * CLOUDS_CLOUDSCALE * 0.5);

	//ridged noise shape
	float r = 0.0;
	uv *= CLOUDS_CLOUDSCALE;
	uv -= q - time;
	float weight = 0.8;
	for (int i = 0; i<8; i++) {
		r += abs(weight*noise(uv));
		uv = mc*uv + time;
		weight *= 0.7;
	}

	//noise shape
	float f = 0.0;
	uv = p;
	uv *= CLOUDS_CLOUDSCALE;
	uv -= q - time;
	weight = 0.7;
	for (int i = 0; i<8; i++) {
		f += weight*noise(uv);
		uv = mc*uv + time;
		weight *= 0.6;
	}

	f *= r + f;

	//noise colour
	float c = 0.0;
	time = u_Time * CLOUDS_SPEED * 2.0;
	uv = p;
	uv *= CLOUDS_CLOUDSCALE*2.0;
	uv -= q - time;
	weight = 0.4;
	for (int i = 0; i<7; i++) {
		c += weight*noise(uv);
		uv = mc*uv + time;
		weight *= 0.6;
	}

	//noise ridge colour
	float c1 = 0.0;
	time = u_Time * CLOUDS_SPEED * 3.0;
	uv = p;
	uv *= CLOUDS_CLOUDSCALE*3.0;
	uv -= q - time;
	weight = 0.4;
	for (int i = 0; i<7; i++) {
		c1 += abs(weight*noise(uv));
		uv = mc*uv + time;
		weight *= 0.6;
	}

	c += c1;

	vec3 cloudcolour = /*vec3(1.1, 1.1, 0.9)*/vec3(1.0, 1.0, 1.0) * clamp((CLOUDS_DARK + CLOUDS_LIGHT*c), 0.0, 1.0);

	f = CLOUDS_CLOUDCOVER + CLOUDS_CLOUDALPHA*f*r;

	//vec3 result = mix(skycolour, clamp(CLOUDS_SKYTINT * skycolour + cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));
	vec3 result = cloudcolour;

	return vec4(result.rgb, clamp(f + c, 0.0, 1.0));
}

void main()
{
	vec3 pViewDir = normalize(u_ViewOrigin - var_vertPos.xyz);

	vec4 cloudColor = Clouds(pViewDir.xy * 0.5 + 0.5);
	cloudColor.a *= clamp(pow(-pViewDir.z, 1.75/*u_Local9.b*//*2.5*/), 0.0, 1.0);
	cloudColor.a *= 0.75;//u_Local9.g;

	if (SHADER_DAY_NIGHT_ENABLED > 0.0 && SHADER_NIGHT_SCALE > 0.0)
	{// Adjust cloud color at night...
		float nMult = clamp(1.25 - SHADER_NIGHT_SCALE, 0.0, 1.0);
		cloudColor *= nMult;
	}

	gl_FragColor = cloudColor;
	out_Glow = vec4(0.0);
	//out_Normal = vec4(EncodeNormal(var_Normal), 0.0, 1.0);
#ifdef __USE_REAL_NORMALMAPS__
	out_NormalDetail = vec4(0.0);
#endif //__USE_REAL_NORMALMAPS__
	//out_Position = vec4(var_vertPos.xyz, SHADER_MATERIAL_TYPE+1.0);

	out_Position = vec4(0.0);
	out_Normal = vec4(0.0);
}

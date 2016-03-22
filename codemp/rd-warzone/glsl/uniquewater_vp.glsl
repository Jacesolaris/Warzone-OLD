#extension GL_EXT_gpu_shader4 : enable

#define ORIGINAL_WATER
//#define SECOND_WATER

#define WAVE

#define USE_DEFORM_VERTEXES
#define USE_TCGEN
#define USE_TCMOD

attribute vec3 attr_Position;
attribute vec3 attr_Normal;

attribute vec4 attr_Color;
attribute vec2 attr_TexCoord0;

#if defined(USE_LIGHTMAP) || defined(USE_TCGEN)
attribute vec2 attr_TexCoord1;
#endif

uniform vec4   u_DiffuseTexMatrix;
uniform vec4   u_DiffuseTexOffTurb;

#if defined(USE_TCGEN)
uniform vec3   u_LocalViewOrigin;
#endif

#if defined(USE_TCGEN)
uniform int    u_TCGen0;
uniform vec3   u_TCGen0Vector0;
uniform vec3   u_TCGen0Vector1;
#endif

#if defined(USE_DEFORM_VERTEXES)
uniform int    u_DeformGen;
uniform float  u_DeformParams[5];
#endif

uniform mat4	u_ModelViewProjectionMatrix;
uniform mat4	u_ModelMatrix;
uniform mat4	u_invEyeProjectionMatrix;
uniform mat4	u_ModelViewMatrix;

varying vec2   var_DiffuseTex;
varying vec4   var_Color;

uniform vec2	u_Dimensions;
uniform vec4	u_ViewOrigin;
uniform float	u_Time;
uniform vec4	u_Local8; // grassLength, grassLayer, wavespeed, wavesize
uniform vec4	u_Local9;





#if defined(ORIGINAL_WATER)





#define m_Length	u_Local8.r
#define m_Layer		u_Local8.g

#ifdef WAVE
bool	m_Wave = true;
float	m_WaveSpeed = u_Local8.b;
float	m_WaveSize = u_Local8.a;
#endif

varying vec2	var_TexCoords;
varying vec2	var_Dimensions;
varying vec3	var_vertPos;
varying float	var_Time;
varying vec3	var_Normal;
varying vec3	var_ViewDir;
varying float	var_Fresnel;






#if defined(USE_DEFORM_VERTEXES)
vec3 DeformPosition(const vec3 pos, const vec3 normal, const vec2 st)
{
	float base =      u_DeformParams[0];
	float amplitude = u_DeformParams[1];
	float phase =     u_DeformParams[2];
	float frequency = u_DeformParams[3];
	float spread =    u_DeformParams[4];

	if (u_DeformGen == DGEN_BULGE)
	{
		phase *= st.x;
	}
	else // if (u_DeformGen <= DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		phase += dot(pos.xyz, vec3(spread));
	}

	float value = phase + (u_Time * frequency);
	float func;

	if (u_DeformGen == DGEN_WAVE_SIN)
	{
		func = sin(value * 2.0 * M_PI);
	}
	else if (u_DeformGen == DGEN_WAVE_SQUARE)
	{
		func = sign(fract(0.5 - value));
	}
	else if (u_DeformGen == DGEN_WAVE_TRIANGLE)
	{
		func = abs(fract(value + 0.75) - 0.5) * 4.0 - 1.0;
	}
	else if (u_DeformGen == DGEN_WAVE_SAWTOOTH)
	{
		func = fract(value);
	}
	else if (u_DeformGen == DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		func = (1.0 - fract(value));
	}
	else // if (u_DeformGen == DGEN_BULGE)
	{
		func = sin(value);
	}

	return pos + normal * (base + func * amplitude);
}
#endif

#if defined(USE_TCGEN)
vec2 GenTexCoords(int TCGen, vec3 position, vec3 normal, vec3 TCGenVector0, vec3 TCGenVector1)
{
	vec2 tex = attr_TexCoord0.st;

	if (TCGen == TCGEN_LIGHTMAP)
	{
		tex = attr_TexCoord1.st;
	}
	else if (TCGen == TCGEN_ENVIRONMENT_MAPPED)
	{
		vec3 viewer = normalize(u_LocalViewOrigin - position);
		vec2 ref = reflect(viewer, normal).yz;
		tex.s = ref.x * -0.5 + 0.5;
		tex.t = ref.y *  0.5 + 0.5;
	}
	else if (TCGen == TCGEN_VECTOR)
	{
		tex = vec2(dot(position, TCGenVector0), dot(position, TCGenVector1));
	}
	
	return tex;
}
#endif

#if defined(USE_TCMOD)
vec2 ModTexCoords(vec2 st, vec3 position, vec4 texMatrix, vec4 offTurb)
{
	float amplitude = offTurb.z;
	float phase = offTurb.w * 2.0 * M_PI;
	vec2 st2;
	st2.x = st.x * texMatrix.x + (st.y * texMatrix.z + offTurb.x);
	st2.y = st.x * texMatrix.y + (st.y * texMatrix.w + offTurb.y);

	vec2 offsetPos = vec2(position.x + position.z, position.y);
	
	vec2 texOffset = sin(offsetPos * (2.0 * M_PI / 1024.0) + vec2(phase));
	
	return st2 + texOffset * amplitude;	
}
#endif



int LFSR_Rand_Gen(in int n)
{
  // <<, ^ and & require GL_EXT_gpu_shader4.
  n = (n << 13) ^ n;
  return (n * (n*n*15731+789221) + 1376312589) & 0x7fffffff;
}

float LFSR_Rand_Gen_f(int n )
{
  return float(LFSR_Rand_Gen(n));
}


float noise(vec3 p) {
  ivec3 ip = ivec3(floor(p));
  vec3 u = fract(p);
  u = u*u*(3.0-2.0*u);

  int n = ip.x + ip.y*57 + ip.z*113;

  float res = mix(mix(mix(LFSR_Rand_Gen_f(n+(0+57*0+113*0)),
                          LFSR_Rand_Gen_f(n+(1+57*0+113*0)),u.x),
                      mix(LFSR_Rand_Gen_f(n+(0+57*1+113*0)),
                          LFSR_Rand_Gen_f(n+(1+57*1+113*0)),u.x),u.y),
                 mix(mix(LFSR_Rand_Gen_f(n+(0+57*0+113*1)),
                          LFSR_Rand_Gen_f(n+(1+57*0+113*1)),u.x),
                      mix(LFSR_Rand_Gen_f(n+(0+57*1+113*1)),
                          LFSR_Rand_Gen_f(n+(1+57*1+113*1)),u.x),u.y),u.z);

  return 1.0 - res*(1.0/1073741824.0);
}


void main()
{
	vec3 position  = attr_Position;
	vec3 normal    = attr_Normal * 2.0 - vec3(1.0);

	float time = u_Time + 0.5;
	vec4 v = vec4(attr_Position, 1.0);
	vec4 v2 = v;
#define waveHeight 3.0
#define waveWidth 0.6
	float baseHeight = sin(waveWidth * v2.x) * cos(waveWidth * v2.y) * waveHeight;
	float fresnel = baseHeight - (sin((waveWidth * v2.x) + time) * cos((waveWidth * v2.y) + time) * waveHeight);
	v.z += fresnel;
 	gl_Position = u_ModelViewProjectionMatrix * v;

#if defined(USE_DEFORM_VERTEXES)
	position = DeformPosition(v.xyz, normal, attr_TexCoord0.st);
#endif

#if defined(USE_TCGEN)
	vec2 tex = GenTexCoords(u_TCGen0, v.xyz, normal, u_TCGen0Vector0, u_TCGen0Vector1);
#else
	vec2 tex = attr_TexCoord0.st;
#endif

	var_Dimensions = u_Dimensions.st;
	var_vertPos = v.xyz;
	var_TexCoords = tex;
	var_Fresnel = (sin((waveWidth * v2.x) + time) * cos((waveWidth * v2.y) + time) * waveHeight);//fresnel;
	var_Time = u_Time;
	var_Normal = normal;
	var_ViewDir = u_ViewOrigin.xyz - v.xyz;
}






#elif defined(SECOND_WATER)






uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;
uniform vec3		u_PrimaryLightAmbient;
uniform float		u_PrimaryLightRadius;


const vec4 tangent = vec4(1.0, 0.0, 0.0, 0.0);
const vec4 norm = vec4(0.0, 1.0, 0.0, 0.0);
const vec4 binormal = vec4(0.0, 0.0, 1.0, 0.0);

//attribute vec3 attr_Normal;
attribute vec4 attr_Tangent;

const mat4 lightTransformation = mat4(tangent, binormal, norm, vec4(0.0));

#define time u_Time*0.2
#define time2 u_Time*0.3

varying vec3 var_vertPos;
//varying vec4 lightDir;
varying vec2 waterFlow;
varying vec2 waterRipple;
varying vec4 projCoords;
varying vec4 eyeDir;
//varying vec2 UV;

void main(void)
{
	//UV = attr_TexCoord0.xy;

	//vec3 viewDir = u_ViewOrigin.xyz - attr_Position.xyz;

	//vec4 norm = vec4(attr_Normal.xyz * 2.0 - 1.0, viewDir.x);
	//vec4 tangent = vec4(attr_Tangent.xyz, viewDir.y);
	//vec4 binormal = vec4(cross(norm.xyz, tangent.xyz) * (tangent.w * 2.0 - 1.0), viewDir.z);
	//mat3 tangentToWorld = mat3(tangent.xyz, binormal.xyz, norm.xyz);

	//vec4 temp = vec4(u_ViewOrigin.xyz - attr_Position.xyz, 1.0);//(u_ModelViewMatrix * vec4(attr_Position, 1.0));
	//eyeDir = temp * lightTransformation;
	//eyeDir = normalize(u_ModelViewMatrix * vec4(viewDir, 0.0));

	//vec4 temp = vec4(u_ViewOrigin.xyz - u_PrimaryLightOrigin.xyz, 1.0);; // Directional light from the sun
	//lightDir = temp * lightTransformation;
	//lightDir.xyz = u_PrimaryLightOrigin.xyz - (attr_Position.xyz * u_PrimaryLightOrigin.w);
	//lightDir.w = u_PrimaryLightRadius * u_PrimaryLightRadius;

	// texcoords for making the water flow
	waterFlow = attr_TexCoord0.st + vec2(0.0, time);

	// texcoords for making the water ripple
	waterRipple = attr_TexCoord0.st + vec2(0.0, time2);

	//gl_ClipVertex = u_ModelViewMatrix * vec4(attr_Position, 1.0);
	gl_Position = projCoords = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);

	var_vertPos = attr_Position.xyz;
}






#else //!defined(ORIGINAL_WATER)



// Source: http://trederia.blogspot.com/2014/09/water-in-opengl-and-gles-20-part-4.html

/*
uniform mat4	u_ModelViewProjectionMatrix;
uniform mat4	u_ModelMatrix;
uniform mat4	u_invEyeProjectionMatrix;
uniform mat4	u_ModelViewMatrix;
*/

/////////////////////////////
// Uniforms
#define u_worldMatrix u_ModelViewMatrix
#define u_worldViewProjectionMatrix u_ModelViewProjectionMatrix
#define u_worldViewProjectionReflectionMatrix u_ModelViewProjectionMatrix//u_invEyeProjectionMatrix

#define u_cameraPosition u_ViewOrigin.xyz

/////////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;
varying vec4 v_vertexReflectionPosition;

varying vec2 v_texCoord;

varying vec3 v_eyePosition;

varying vec3 var_Normal;

/////////////////////////////
void main()
{
	v_vertexRefractionPosition = u_worldViewProjectionMatrix * vec4(attr_Position, 1.0);
	v_vertexReflectionPosition = v_vertexRefractionPosition;//u_worldViewProjectionReflectionMatrix * vec4(attr_Position, 1.0);
	v_vertexReflectionPosition.y = -v_vertexReflectionPosition.y;
	
	gl_Position = v_vertexRefractionPosition;
	
	v_texCoord = attr_TexCoord0;
	
	v_eyePosition = u_cameraPosition - (u_worldMatrix * vec4(attr_Position, 1.0)).xyz;

	//vec4 diff = u_worldViewProjectionMatrix * vec4(u_cameraPosition.xyz - attr_Position.xyz, 1.0);
	//float diff = v_eyePosition.y - v_vertexRefractionPosition.y;
	//v_vertexReflectionPosition.y = v_eyePosition.y - (diff*2.0);

	var_Normal = attr_Normal * 2.0 - 1.0;
}



#endif //defined(ORIGINAL_WATER)

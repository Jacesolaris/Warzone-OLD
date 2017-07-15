attribute vec3	attr_Position;
attribute vec3	attr_Normal;
attribute vec2	attr_TexCoord0;

uniform mat4	u_ModelViewProjectionMatrix;

uniform vec4	u_Settings0; // useTC, useDeform, useRGBA, isTextureClamped

#define USE_DEFORM			u_Settings0.g

uniform vec4	u_Local10;

uniform float	u_Time;

uniform int		u_DeformGen;
uniform float	u_DeformParams[5];

varying vec2	var_TexCoords;
varying vec3	var_vertPos;
varying vec3	var_Normal;

// Maximum waves amplitude
#define maxAmplitude u_Local10.g

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


void main()
{
	vec3 position  = attr_Position.xyz;
	vec3 normal    = attr_Normal * 2.0 - vec3(1.0);

	if (USE_DEFORM == 1.0)
	{
		position = DeformPosition(position, normal, attr_TexCoord0.st);
	}

	var_vertPos = position.xyz;
	var_TexCoords = attr_TexCoord0.st;
	var_Normal = normal;

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);
}

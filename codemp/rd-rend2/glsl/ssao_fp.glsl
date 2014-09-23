uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;

varying vec2		var_ScreenTex;
varying vec2		var_Dimensions;
varying vec4		var_ViewInfo; // zfar / znear, zfar, znear, zfar

vec2 poissonDisc[9] = vec2[9](
vec2(-0.7055767, 0.196515),    vec2(0.3524343, -0.7791386),
vec2(0.2391056, 0.9189604),    vec2(-0.07580382, -0.09224417),
vec2(0.5784913, -0.002528916), vec2(0.192888, 0.4064181),
vec2(-0.6335801, -0.5247476),  vec2(-0.5579782, 0.7491854),
vec2(0.7320465, 0.6317794)
);

// Input: It uses texture coords as the random number seed.
// Output: Random number: [0,1), that is between 0.0 and 0.999999... inclusive.
// Author: Michael Pohoreski
// Copyright: Copyleft 2012 :-)
// Source: http://stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader

float random( const vec2 p )
{
  // We need irrationals for pseudo randomness.
  // Most (all?) known transcendental numbers will (generally) work.
  const vec2 r = vec2(
    23.1406926327792690,  // e^pi (Gelfond's constant)
     2.6651441426902251); // 2^sqrt(2) (Gelfond-Schneider constant)
  //return fract( cos( mod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );
  return mod( 123456789., 1e-7 + 256. * dot(p,r) );  
}

mat2 randomRotation( const vec2 p )
{
	float r = random(p);
	float sinr = sin(r);
	float cosr = cos(r);
	return mat2(cosr, sinr, -sinr, cosr);
}

float getLinearDepth(sampler2D depthMap, const vec2 tex, const float zFarDivZNear)
{
		float sampleZDivW = texture2D(depthMap, tex).r;
		return 1.0 / mix(zFarDivZNear, 1.0, sampleZDivW);
		//return -var_ViewInfo.a * var_ViewInfo.z / (sampleZDivW * (var_ViewInfo.a - var_ViewInfo.z) - var_ViewInfo.a);
}

float ambientOcclusion(sampler2D depthMap, const vec2 tex, const float zFarDivZNear, const float zFar)
{
	float result = 0;

	float sampleZ = zFar * getLinearDepth(depthMap, tex, zFarDivZNear);

	vec2 expectedSlope = vec2(dFdx(sampleZ), dFdy(sampleZ)) / vec2(dFdx(tex.x), dFdy(tex.y));
	
	if (length(expectedSlope) > 5000.0)
		return 1.0;
	
	vec2 offsetScale = vec2(3.0 / sampleZ);
	
	mat2 rmat = randomRotation(tex);
		
	int i;
	for (i = 0; i < 3; i++)
	{
		vec2 offset = rmat * poissonDisc[i] * offsetScale;
		float sampleZ2 = zFar * getLinearDepth(depthMap, tex + offset, zFarDivZNear);

		if (abs(sampleZ - sampleZ2) > 20.0)
			result += 1.0;
		else
		{
			float expectedZ = sampleZ + dot(expectedSlope, offset);
			result += step(expectedZ - 1.0, sampleZ2);
		}
	}
	
	//result *= 0.33333;
	result *= 0.5;
	
	return result;
}

void main()
{
	//float result = ambientOcclusion(u_ScreenDepthMap, var_ScreenTex, var_ViewInfo.x, var_ViewInfo.y);
	
	//gl_FragColor = vec4(vec3(result), 1.0);

	// UQ1: Blur!
	vec2 offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);
	vec3 ao = vec3(0.0);
	vec3 color = vec3(0.0);

	/*
	for (float x = -2.0; x <= 2.0; x++)
	{
		for (float y = -2.0; y <= 2.0; y++)
		{
			ao += ambientOcclusion(u_ScreenDepthMap, var_ScreenTex + vec2(offset.x*y, offset.x*y), var_ViewInfo.x, var_ViewInfo.y);
		}
	}

	ao.rgb /= (5*5);
	ao.rgb = clamp(ao.rgb, 0.85, 1.0);
	*/
	ao.rgb = vec3(ambientOcclusion(u_ScreenDepthMap, var_ScreenTex, var_ViewInfo.x, var_ViewInfo.y));
	ao.rgb = clamp(ao.rgb, 0.85, 1.0);

	color = texture2D(u_DiffuseMap, var_ScreenTex).rgb;

	gl_FragColor = vec4(color * ao, 1.0);
	//gl_FragColor = vec4(color, 1.0);
}

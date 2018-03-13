uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_OverlayMap; // Real normals. Alpha channel 1.0 means enabled...

uniform vec4		u_Settings0;
uniform vec2		u_Dimensions;

varying vec2		var_TexCoords;


#define __FAST_NORMAL_DETAIL__


vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

float getHeight(vec2 uv) {
  return length(texture(u_DiffuseMap, uv).rgb) / 3.0;
}

#ifdef __FAST_NORMAL_DETAIL__
vec4 normalVector(vec3 color) {
	vec4 normals = vec4(color.rgb, length(color.rgb) / 3.0);
	normals.rgb = vec3(length(normals.r - normals.a), length(normals.g - normals.a), length(normals.b - normals.a));

	// Contrast...
//#define normLower ( 128.0 / 255.0 )
//#define normUpper (255.0 / 192.0 )
#define normLower ( 32.0 / 255.0 )
#define normUpper (255.0 / 212.0 )
	vec3 N = clamp((clamp(normals.rgb - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

	return vec4(vec3(1.0) - (normalize(N) * 0.5 + 0.5), normals.a);
}
#else //!__FAST_NORMAL_DETAIL__
vec4 bumpFromDepth(vec2 uv, vec2 resolution, float scale) {
  vec2 step = 1. / resolution;
    
  float height = getHeight(uv);
    
  vec2 dxy = height - vec2(
      getHeight(uv + vec2(step.x, 0.)), 
      getHeight(uv + vec2(0., step.y))
  );

  vec3 N = vec3(dxy * scale / step, 1.);

// Contrast...
#define normLower ( 128.0/*48.0*/ / 255.0 )
#define normUpper (255.0 / 192.0/*128.0*/ )
  N = clamp((clamp(N - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

  return vec4(normalize(N) * 0.5 + 0.5, height);
}

vec4 normalVector(vec2 coord) {
	return bumpFromDepth(coord, u_Dimensions, 0.1 /*scale*/);
}
#endif //__FAST_NORMAL_DETAIL__


vec3 TangentFromNormal ( vec3 normal )
{
	vec3 tangent;
	vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0)); 
	vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0)); 

	if( length(c1) > length(c2) )
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	return normalize(tangent);
}

void main(void)
{
	vec4 norm = texture(u_NormalMap, var_TexCoords);
	//norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	//norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y
	//norm.z = sqrt(clamp((0.25 - norm.x * norm.x) - norm.y * norm.y, 0.0, 1.0));
	norm.xyz = DecodeNormal(norm.xy);

	if (u_Settings0.r > 0.0)
	{
		vec4 normalDetail = textureLod(u_OverlayMap, var_TexCoords, 0.0);

		if (normalDetail.a < 1.0)
		{// Don't have real normalmap, make normals for this pixel...
#ifdef __FAST_NORMAL_DETAIL__
			vec3 color = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
			normalDetail = normalVector(color);
#else //!__FAST_NORMAL_DETAIL__
			normalDetail = normalVector(var_TexCoords);
#endif //__FAST_NORMAL_DETAIL__
		}

		normalDetail.rgb = normalize(clamp(normalDetail.rgb, 0.0, 1.0) * 2.0 - 1.0);
		//normalDetail.rgb *= 0.25;//u_Settings0.g;
		//norm.rgb = normalize(norm.rgb + normalDetail.rgb);
		norm.rgb = normalize(mix(norm.rgb, normalDetail.rgb, 0.25 * (length(norm.rgb - normalDetail.rgb) / 3.0)));
	}

	gl_FragColor = vec4(norm.rgb * 0.5 + 0.5, 1.0);
}
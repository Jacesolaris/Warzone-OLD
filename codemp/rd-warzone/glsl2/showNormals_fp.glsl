uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;

uniform vec4		u_Settings0;
uniform vec2		u_Dimensions;

varying vec2		var_TexCoords;

const vec3 LUMA_COEFFICIENT = vec3(0.2126, 0.7152, 0.0722);

float lumaAtCoord(vec2 coord) {
  vec3 pixel = texture(u_DiffuseMap, coord).rgb;
  float luma = dot(pixel, LUMA_COEFFICIENT);
  return luma;
}

vec4 normalVector(vec2 coord) {
  float lumaU0 = lumaAtCoord(coord + vec2(-1.0,  0.0) / u_Dimensions);
  float lumaU1 = lumaAtCoord(coord + vec2( 1.0,  0.0) / u_Dimensions);
  float lumaV0 = lumaAtCoord(coord + vec2( 0.0, -1.0) / u_Dimensions);
  float lumaV1 = lumaAtCoord(coord + vec2( 0.0,  1.0) / u_Dimensions);

  vec2 slope = vec2(lumaU0 - lumaU1, lumaV0 - lumaV1) * 0.5 + 0.5;

// Contrast...
#define normLower ( 128.0/*48.0*/ / 255.0 )
#define normUpper (255.0 / 192.0/*128.0*/ )
  slope = clamp((clamp(slope - normLower, 0.0, 1.0)) * normUpper, 0.0, 1.0);

  return vec4(slope, 1.0, length(slope.rg / 2.0));
}

void main(void)
{
	vec4 norm = texture(u_NormalMap, var_TexCoords);

	if (u_Settings0.r > 0.0)
	{
		norm.rgb = norm.rgb * 2.0 - 1.0;
		vec4 normalDetail = normalVector(var_TexCoords);
		normalDetail.rgb = normalDetail.rgb * 2.0 - 1.0;
		normalDetail.rgb *= 0.25;//u_Settings0.g;
		normalDetail.z = sqrt(clamp((0.25 - normalDetail.x * normalDetail.x) - normalDetail.y * normalDetail.y, 0.0, 1.0));
		norm.rgb = normalize(norm.rgb + normalDetail.rgb);
		norm.rgb = norm.rgb * 0.5 + 0.5;
	}

	gl_FragColor = vec4(norm.rgb, 1.0);
}
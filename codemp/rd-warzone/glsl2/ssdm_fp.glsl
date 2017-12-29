uniform sampler2D				u_DiffuseMap;
uniform sampler2D				u_PositionMap;
uniform sampler2D				u_NormalMap;
uniform sampler2D				u_ScreenDepthMap;

uniform vec2					u_Dimensions;

uniform vec4					u_Local1; // DISPLACEMENT_MAPPING_STRENGTH, r_testShaderValue1, r_testShaderValue2, r_testShaderValue3

uniform vec4					u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3					u_ViewOrigin;

varying vec2					var_TexCoords;

#define DISPLACEMENT_STRENGTH	u_Local1.r

vec2 px = vec2(1.0) / u_Dimensions.xy;

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

void main(void)
{
	vec2 texCoords = var_TexCoords;
	float invDepth = 1.0 - texture(u_ScreenDepthMap, texCoords).r;
	invDepth = clamp(length(invDepth * 1.75 - 0.75), 0.0, 1.0);
	
	vec3 dMap = texture(u_PositionMap, texCoords).rgb;
	vec3 norm = vec3(dMap.gb, 0.0) * 2.0 - 1.0;
	norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y
	//vec3 norm = DecodeNormal(dMap.gb);

	vec2 distFromCenter = vec2(length(texCoords.x - 0.5), length(texCoords.y - 0.5));
	float displacementStrengthMod = (DISPLACEMENT_STRENGTH / 18.0); // Default is 18.0. If using higher displacement, need more screen edge flattening, if less, less flattening.
	float screenEdgeScale = clamp(max(distFromCenter.x, distFromCenter.y) * 2.0, 0.0, 1.0);
	screenEdgeScale = 1.0 - pow(screenEdgeScale, 16.0/displacementStrengthMod);

	texCoords += norm.xy * vec2(-DISPLACEMENT_STRENGTH * px) * invDepth * screenEdgeScale * dMap.r;

	gl_FragColor = vec4(texture(u_DiffuseMap, texCoords).rgb, 1.0);
}

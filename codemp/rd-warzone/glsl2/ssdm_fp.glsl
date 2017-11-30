uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_ScreenDepthMap;

uniform vec2		u_Dimensions;

uniform vec4		u_Local1; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov
uniform vec3		u_ViewOrigin;

varying vec2		var_TexCoords;

vec2 px = vec2(1.0) / u_Dimensions.xy;

void main(void)
{
	vec2 texCoords = var_TexCoords;
	float invDepth = 1.0 - texture(u_ScreenDepthMap, texCoords).r;
	invDepth = clamp(length(invDepth * 1.75 - 0.75), 0.0, 1.0);
	
	vec3 dMap = texture(u_PositionMap, texCoords).rgb;
	vec3 norm = vec3(dMap.gb, 0.0) * 2.0 - 1.0;
	//norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y

	vec2 distFromCenter = vec2(length(texCoords.x - 0.5), length(texCoords.y - 0.5));
	float screenEdgeScale = clamp(max(distFromCenter.x, distFromCenter.y) * 2.0, 0.0, 1.0);
	screenEdgeScale = 1.0 - pow(screenEdgeScale, 16.0);

	texCoords += norm.xy * vec2(-18.0 * px) * invDepth * screenEdgeScale * dMap.r;

	gl_FragColor = vec4(texture(u_DiffuseMap, texCoords).rgb, 1.0);
}

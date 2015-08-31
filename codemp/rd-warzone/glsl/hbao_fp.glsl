uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_ScreenDepthMap;

uniform mat4		u_invEyeProjectionMatrix;

varying vec2		var_ScreenTex;
varying vec2		u_Dimensions;
varying vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

// Sampling radius is in view space...
#define SAMPLING_RADIUS 0.5
#define NUM_SAMPLING_DIRECTIONS 8
// sampling step is in texture space
#define SAMPLING_STEP 0.004
#define NUM_SAMPLING_STEPS 4
#define TANGENT_BIAS 0.2

// Select a method...
//#define ATTENUATION_METHOD_1
#define ATTENUATION_METHOD_2

// Use real screen normal map? Otherwise calculate from depth...
#define USE_NORMAL_MAP

// Filter result?
//#define FILTER_RESULT

#if defined(FILTER_RESULT)
float filter(float x)
{
    return max(0, 1.0 - x*x);
}
#endif //FILTER_RESULT

const float depthMult = 1.0;

vec2 offset1 = vec2(0.0, 1.0 / u_Dimensions.y);
vec2 offset2 = vec2(1.0 / u_Dimensions.x, 0.0);

vec3 normal_from_depth(float depth, vec2 texcoords) {
  float depth1 = texture2D(u_ScreenDepthMap, texcoords + offset1).r * depthMult;
  float depth2 = texture2D(u_ScreenDepthMap, texcoords + offset2).r * depthMult;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

vec3 SampleNormals(sampler2D normalMap, in vec2 coord)  
{
#if 1
	 return texture2D(normalMap, coord).rgb;
#else
	 float depth = texture2D(u_ScreenDepthMap, coord).r * depthMult;
	 return normal_from_depth(depth, coord);
#endif
}

void main()
{
	//gl_FragColor = vec4(texture2D(u_NormalMap, var_ScreenTex).rgb, 1.0);
	//return;

    float start_Z = texture2D(u_ScreenDepthMap, var_ScreenTex).r; // returns value (z/w+1)/2
    vec3 start_Pos = vec3(var_ScreenTex, start_Z);
    vec3 ndc_Pos = (2.0 * start_Pos) - 1.0; // transform to normalized device coordinates xyz/w

    // reconstruct view space position
    vec4 unproject = u_invEyeProjectionMatrix * vec4(ndc_Pos, 1.0);
    vec3 viewPos = unproject.xyz / unproject.w; // 3d view space position P
    vec3 viewNorm = SampleNormals(u_NormalMap, var_ScreenTex).xyz; // 3d view space normal N

    float total = 0.0;
    float sample_direction_increment = 2 * M_PI / NUM_SAMPLING_DIRECTIONS;

    for (int i = 0; i < NUM_SAMPLING_DIRECTIONS; i++) {
        // no jittering or randomization of sampling direction just yet
        float sampling_angle = i * sample_direction_increment; // azimuth angle theta in the paper
        vec2 sampleDir = vec2(cos(sampling_angle), sin(sampling_angle));

        // we will now march along sampleDir and calculate the horizon
        // horizon starts with the tangent plane to the surface, whose angle we can get from the normal
        float tangentAngle = acos(dot(vec3(sampleDir, 0), viewNorm)) - (0.5 * M_PI) + TANGENT_BIAS;
        float horizonAngle = tangentAngle;
        vec3 lastDiff = vec3(0);

        for (int j = 0; j < NUM_SAMPLING_STEPS; j++) {
            // march along the sampling direction and see what the horizon is
            vec2 sampleOffset = float(j+1) * SAMPLING_STEP * sampleDir;
            vec2 offTex = var_ScreenTex + sampleOffset;

            float off_start_Z = texture2D(u_ScreenDepthMap, offTex.st).r;
            vec3 off_start_Pos = vec3(offTex, off_start_Z);
            vec3 off_ndc_Pos = (2.0 * off_start_Pos) - 1.0;
            vec4 off_unproject = u_invEyeProjectionMatrix * vec4(off_ndc_Pos, 1.0);
            vec3 off_viewPos = off_unproject.xyz / off_unproject.w;

            // we now have the view space position of the offset point
            vec3 diff = off_viewPos.xyz - viewPos.xyz;

            if (length(diff) < SAMPLING_RADIUS) {
                // skip samples which are outside of our local sampling radius
                lastDiff = diff;
                float elevationAngle = atan(diff.z / length(diff.xy));
                horizonAngle = max(horizonAngle, elevationAngle);
            }
        }

        // the paper uses this attenuation but I like the other way better
#if defined(ATTENUATION_METHOD_1)
        float normDiff = length(lastDiff) / SAMPLING_RADIUS;
        float attenuation = 1.0 - normDiff*normDiff;
#elif defined(ATTENUATION_METHOD_2)
        float attenuation = 1.0 / (1 + length(lastDiff));
#else // Just in case someone doesn't define a method...
		float normDiff = length(lastDiff) / SAMPLING_RADIUS;
        float attenuation = 1.0 - normDiff*normDiff;
#endif //ATTENUATION_METHODS

        // now compare horizon angle to tangent angle to get ambient occlusion
        float occlusion = clamp(attenuation * (sin(horizonAngle) - sin(tangentAngle)), 0.0, 1.0);
        total += 1.0 - occlusion;
    }

    total /= NUM_SAMPLING_DIRECTIONS;

#if defined(FILTER_RESULT)
	total = 1.0 - filter(total);
#endif //FILTER_RESULT

    gl_FragColor = vec4(total, total, total, 1.0);
}

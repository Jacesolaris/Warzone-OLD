uniform sampler2D			u_DiffuseMap;			// Screen
uniform sampler2D			u_PositionMap;			// Positions
uniform sampler2D			u_NormalMap;			// Normals
uniform sampler2D			u_ScreenDepthMap;		// Depth
uniform sampler2D			u_DeluxeMap;			// Noise

uniform vec3				u_SsdoKernel[32];
uniform vec4				u_ViewInfo;				// znear, zfar, zfar / znear, fov
uniform vec2				u_Dimensions;
uniform vec3				u_ViewOrigin;
uniform vec4				u_PrimaryLightOrigin;

uniform vec4				u_Local0;
uniform vec4				u_Local1;

#define HStep				u_Local0.r
#define VStep				u_Local0.g
#define baseRadius			u_Local0.b
#define maxOcclusionDist	u_Local0.a

varying vec2   				var_TexCoords;
varying vec3   				var_LightPos;

#define znear				u_ViewInfo.r			//camera clipping start
#define zfar				u_ViewInfo.g			//camera clipping end

#if 0
float linearize(float zdepth)
{
    return -zfar * znear / (zdepth * (zfar - znear) - zfar);
	//return 1.0 / mix(u_ViewInfo.x, 1.0, zdepth);
}

vec3 ComputeDOFactor(in vec3 origin, in vec3 normal, in vec3 noiseValue )
{
	float radius = baseRadius / linearize(textureLod(u_ScreenDepthMap, var_TexCoords, 0).r);

	float maxDistanceInv = 1.0f / maxOcclusionDist;
	float attenuationAngleThreshold = 0.1;
	
	vec4 occlusionSh2 = vec4(0.0f);
	const float fudgeFactorL0 = 2.0;
	const float fudgeFactorL1 = 10.0;
	const float sh2WeightL0 = fudgeFactorL0 * 0.28209; //0.5*sqrt(1.0/pi);
	const vec3 sh2WeightL1 = vec3(fudgeFactorL1) * 0.48860; //0.5*sqrt(3.0/pi);
	const vec4 sh2Weight = vec4(sh2WeightL1, sh2WeightL0) / 32;

	for(int i = 0; i < 32; ++i)
	{
		vec2 sampleTex = reflect(u_SsdoKernel[i].xy, noiseValue.xy) * radius + var_TexCoords;
		vec4 samplePos4 = textureLod(u_PositionMap, sampleTex, 0);
		vec3 samplePos = samplePos4.xyz / samplePos4.w;
	
		vec3 originToSample = samplePos - origin;
		float distToSample = length(originToSample);
		vec3 originToSampleNormalized = originToSample / distToSample;

		float attenuation = 1 - clamp(distToSample * maxDistanceInv, 0.0f, 1.0f );
		float dp = dot(normal, originToSampleNormalized);
		attenuation = attenuation * attenuation * step( attenuationAngleThreshold, dp );

		vec4 val = attenuation * sh2Weight * vec4(originToSampleNormalized,1);

		if (!isnan(val.x) && !isinf(val.x))
			occlusionSh2 += val;
	}

	return occlusionSh2.xyz;
}
void main()
{	
	vec3 DO = vec3(0.0);

	//----- Fragment Position -----//
	vec3 worldPos = textureLod(u_PositionMap, var_TexCoords, 0).xyz;
	
	//------ Fragment Normal ------//
	vec3 worldNormal = textureLod(u_NormalMap, var_TexCoords, 0).xyz;
	
	//----------- Noise -----------//
	vec3 noiseValue = textureLod(u_DeluxeMap, var_TexCoords * vec2(HStep, VStep), 0).rgb;
	
	if (worldNormal != vec3(0.0))
		DO = ComputeDOFactor(worldPos, worldNormal, noiseValue);
	else
		DO = worldPos;

	gl_FragColor = vec4(DO, 1.0);
}

#endif





#if 0

#define GI_SSAO

#define PI 3.14


// --------------------------------------------------
// Uniforms
// --------------------------------------------------
//uniform sampler2D u_DeluxeMap;  // random vectors
//uniform sampler2D u_NormalMap; // normal + depth
//uniform sampler2D u_DiffuseMap; // albedo

//uniform vec3 uLightPos;    // light pos in view space

//uniform vec2 uScreenSize;  // screen dimensions
//uniform vec2 uClipZ;       // clipping planes
//uniform vec2 uTanFovs;     // tangent of field of views
//uniform int uSampleCnt;    // number of samples
//uniform float uRadius;     // radius size
//uniform float uGiBoost;    // gi boost

//vec3 uSamples[SAMPLE_CNT]; // sampling directions

#define uLightPos var_LightPos
#define uScreenSize u_Dimensions
#define uClipZ vec2(zfar-znear, znear)
#define uTanFovs u_Local1.gr
#define uSampleCnt 32
#define uRadius baseRadius
#define uGiBoost 1.0

#define uSamples u_SsdoKernel

// --------------------------------------------------
// Functions
// --------------------------------------------------
// get the view position of a sample from its depth 
// and eye information

vec3 ndc_to_view(vec2 ndc,
                 float depth,
                 vec2 clipPlanes,
                 vec2 tanFov) {
	// go from [0,1] to [zNear, zFar]
	float z = depth * clipPlanes.x + clipPlanes.y;
	// view space position
	return vec3(ndc * tanFov, -1) * z;
}

vec2 view_to_ndc(vec3 view,
                 vec2 clipPlanes,
                 vec2 tanFov) {
	return -view.xy / (tanFov*view.z);
}

void main() 
{
	vec4 position = texture(u_PositionMap, var_TexCoords);

	const float ATTF = 1e-5; // attenuation factor
	vec2 st = var_TexCoords;
	vec4 t1 = texture(u_NormalMap,st);      // read normal + depth
	t1.a = texture(u_ScreenDepthMap,st).a;
	vec3 t2 = texture(u_DiffuseMap,st).rgb; // colour
	vec3 n = t1.rgb*2.0-1.0; // rebuild normal
	vec3 p = ndc_to_view(var_TexCoords*2.0-1.0, t1.a, uClipZ, uTanFovs); // get view pos
	
	vec3 l = uLightPos - p; // light vec
	//vec3 l = u_PrimaryLightOrigin.xyz - position.xyz;

	float att = 1.0+ATTF*length(l);
	float nDotL = max(0.0,dot(normalize(l),n));
	gl_FragColor.rgb = t2;//*nDotL/(att*att);
	gl_FragColor.a = 1.0;

	if (position.a == 1024.0 || position.a == 1025.0)
	{// Skybox... Skip...
		return;
	}

#if defined GI_SSAO
	float occ = 0.0;
	float occCnt = 0.0;
	vec3 rvec = normalize(texture(u_DeluxeMap, gl_FragCoord.xy/64.0).rgb*2.0-1.0);
	for(int i=0; i<uSampleCnt && t1.a < 1.0; ++i) {
		vec3 dir = reflect(uSamples[i].xyz,rvec); // a la Crysis
		dir -= 2.0*dir*step(dot(n,dir),0.0);      // a la Starcraft
		vec3 sp = p + (dir * uRadius) * (t1.a * 1e2); // scale radius with depth
		vec2 spNdc = view_to_ndc(sp, uClipZ, uTanFovs); // get sample ndc coords
		bvec4 outOfScreen = bvec4(false); // check if sample projects to screen
		outOfScreen.xy = lessThan(spNdc, vec2(-1));
		outOfScreen.zw = greaterThan(spNdc, vec2(1));
		if(any(outOfScreen)) continue;
		vec4 spNd = texture(u_NormalMap,(spNdc*0.5 + 0.5)); // get nd data
		spNd.a = texture(u_ScreenDepthMap,(spNdc*0.5 + 0.5)).a;
		vec3 occEye = -sp/sp.z*(spNd.a*uClipZ.x+uClipZ.y); // compute correct pos
		vec3 occVec = occEye - p; // vector
		float att2 = 1.0+ATTF*length(occVec); // quadratic attenuation
		occ += max(0.0,dot(normalize(occVec),n)-0.25) / (att2*att2);
		++occCnt;
	};
	gl_FragColor.rgb*= occCnt > 0.0 ? vec3(1.0-occ*uGiBoost/occCnt) : vec3(1);
#elif defined GI_SSDO
	vec3 gi = vec3(0.0);
	float giCnt = 0.0;
	vec3 rvec = normalize(texture(u_DeluxeMap, gl_FragCoord.xy/64.0).rgb*2.0-1.0);
	for(int i=0; i<uSampleCnt && t1.a < 1.0; ++i) {
		vec3 dir = reflect(uSamples[i].xyz,rvec); // a la Crysis
		dir -= 2.0*dir*step(dot(n,dir),0.0);      // a la Starcraft
		vec3 sp = p + (dir * uRadius) * (t1.a * 1e2); // scale radius with depth
		vec2 spNdc = view_to_ndc(sp, uClipZ, uTanFovs); // get sample ndc coords
		bvec4 outOfScreen = bvec4(false); // check if sample projects to screen
		outOfScreen.xy = lessThan(spNdc, vec2(-1));
		outOfScreen.zw = greaterThan(spNdc, vec2(1));
		if(any(outOfScreen)) continue;
		vec2 spSt = (spNdc*0.5 + 0.5);
		vec4 spNd = texture(u_NormalMap,spSt); // get nd data
		spNd.a = texture(u_ScreenDepthMap,spSt).a;
		vec3 occEye = -sp/sp.z*(spNd.a*uClipZ.x+uClipZ.y); // compute correct pos
		vec3 occVec = occEye - p; // vector
		float att2 = 1.0+ATTF*length(occVec); // quadratic attenuation
		
		vec3 spL = uLightPos - occEye; // sample light vec
		//vec4 position2 = texture(u_PositionMap, spSt);
		//vec3 spL = u_PrimaryLightOrigin.xyz - position2.xyz; // sample light vec
		
		vec3 spKa = texture(u_DiffuseMap, spSt).rgb; // sample albedo
		vec3 spN = spNd.rgb*2.0-1.0; // sample normal
		float spAtt = 1.0+ATTF*length(spL); // quadratic attenuation
		vec3 spE = spKa*max(0.0,dot(normalize(spL),spN))/(spAtt*spAtt); // can precomp.
		float v = 1.0-max(0.0,dot(n,spN));
		gi+= spE*v*max(0.0,dot(normalize(occVec),n))/(att2*att2);
		++giCnt;
	};
	gl_FragColor.rgb+= giCnt > 0.0 ? t2*gi*nDotL*uGiBoost/giCnt : vec3(0);
#endif // GI_SSAO
}

#endif




#if 1


#define g_occlusion_radius			baseRadius				// VALUE="0.279710" MIN="0.000100" MAX="0.000000"
#define g_occlusion_max_distance	maxOcclusionDist		// VALUE="0.639419" MIN="0.000100" MAX="0.000000"

#define g_resolution				u_Dimensions

#define smp_position				u_PositionMap
#define smp_normal					u_NormalMap
#define smp_noise					u_DeluxeMap

vec4 dssdo_accumulate(vec2 tex)
{
	vec3 points[32] = vec3[]
	(
		vec3(-0.134, 0.044, -0.825),
		vec3(0.045, -0.431, -0.529),
		vec3(-0.537, 0.195, -0.371),
		vec3(0.525, -0.397, 0.713),
		vec3(0.895, 0.302, 0.139),
		vec3(-0.613, -0.408, -0.141),
		vec3(0.307, 0.822, 0.169),
		vec3(-0.819, 0.037, -0.388),
		vec3(0.376, 0.009, 0.193),
		vec3(-0.006, -0.103, -0.035),
		vec3(0.098, 0.393, 0.019),
		vec3(0.542, -0.218, -0.593),
		vec3(0.526, -0.183, 0.424),
		vec3(-0.529, -0.178, 0.684),
		vec3(0.066, -0.657, -0.570),
		vec3(-0.214, 0.288, 0.188),
		vec3(-0.689, -0.222, -0.192),
		vec3(-0.008, -0.212, -0.721),
		vec3(0.053, -0.863, 0.054),
		vec3(0.639, -0.558, 0.289),
		vec3(-0.255, 0.958, 0.099),
		vec3(-0.488, 0.473, -0.381),
		vec3(-0.592, -0.332, 0.137),
		vec3(0.080, 0.756, -0.494),
		vec3(-0.638, 0.319, 0.686),
		vec3(-0.663, 0.230, -0.634),
		vec3(0.235, -0.547, 0.664),
		vec3(0.164, -0.710, 0.086),
		vec3(-0.009, 0.493, -0.038),
		vec3(-0.322, 0.147, -0.105),
		vec3(-0.554, -0.725, 0.289),
		vec3(0.534, 0.157, -0.250)
	);

	const int num_samples = 32;

	vec2 noise_texture_size = vec2(4096,4096);//vec2(4,4);
	vec3 center_pos  = texture(smp_position, tex).xyz;
	vec3 eye_pos = u_ViewOrigin.xyz;//g_mat_view_inv[3].xyz;

	float center_depth  = distance(eye_pos, center_pos);

	float radius = g_occlusion_radius / center_depth;
	float max_distance_inv = 1.f / g_occlusion_max_distance;
	float attenuation_angle_threshold = 0.1;

	vec3 noise = texture(smp_noise, tex*g_resolution.xy/noise_texture_size).xyz*2-1;

	//radius = min(radius, 0.1);

	vec3 center_normal = texture(smp_normal, tex).xyz;

	vec4 occlusion_sh2 = vec4(0.0);

	const float fudge_factor_l0 = 2.0;
	const float fudge_factor_l1 = 10.0;

	const float sh2_weight_l0 = fudge_factor_l0 * 0.28209; //0.5*sqrt(1.0/pi);
	const vec3 sh2_weight_l1 = vec3(fudge_factor_l1 * 0.48860); //0.5*sqrt(3.0/pi);

	const vec4 sh2_weight = vec4(sh2_weight_l1, sh2_weight_l0) / num_samples;

	//[unroll] // compiler wants to branch here by default and this makes it run nearly 2x slower on PC and 1.5x slower on 360!
	for( int i=0; i < num_samples; ++i )
	{
	    vec2 textureOffset = reflect( points[ i ].xy, noise.xy ).xy * radius;
		vec2 sample_tex = tex + textureOffset;
		vec3 sample_pos = textureLod(smp_position, vec2(sample_tex), 0.0).xyz;
		vec3 center_to_sample = sample_pos - center_pos;
		float dist = length(center_to_sample);
		vec3 center_to_sample_normalized = center_to_sample / dist;
		float attenuation = 1.0 - clamp(dist * max_distance_inv, 0.0, 1.0);
		float dp = dot(center_normal, center_to_sample_normalized);

		attenuation = attenuation*attenuation * step(attenuation_angle_threshold, dp);

		occlusion_sh2 += attenuation * sh2_weight*vec4(center_to_sample_normalized,1);
	}

	return occlusion_sh2;
}

void main() 
{
	gl_FragColor = dssdo_accumulate(var_TexCoords);
}

#endif

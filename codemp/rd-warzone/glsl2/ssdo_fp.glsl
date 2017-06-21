uniform sampler2D			u_DiffuseMap;			// Screen
uniform sampler2D			u_PositionMap;			// Positions
uniform sampler2D			u_NormalMap;			// Normals
uniform sampler2D			u_ScreenDepthMap;		// Depth
uniform sampler2D			u_DeluxeMap;			// Noise

uniform vec3				u_SsdoKernel[32];
uniform vec4				u_ViewInfo;				// znear, zfar, zfar / znear, fov
uniform vec2				u_Dimensions;
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
#else

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
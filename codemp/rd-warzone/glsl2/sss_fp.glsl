uniform sampler2D			u_DiffuseMap;			// Screen
uniform sampler2D			u_PositionMap;			// Positions
uniform sampler2D			u_NormalMap;			// Normals
uniform sampler2D			u_ScreenDepthMap;		// Depth
uniform sampler2D			u_DeluxeMap;			// Noise

uniform mat4				u_ModelViewProjectionMatrix;
uniform mat4				u_ModelViewMatrix;
uniform mat4				u_ProjectionMatrix;

uniform vec4				u_ViewInfo;				// znear, zfar, zfar / znear, fov
uniform vec2				u_Dimensions;
uniform vec3				u_ViewOrigin;
uniform vec4				u_PrimaryLightOrigin;

uniform vec4				u_Local0;
uniform vec4				u_Local1;

varying vec3   				var_Ray;
varying vec2   				var_TexCoords;

#define znear				u_ViewInfo.r			//camera clipping start
#define zfar				u_ViewInfo.g			//camera clipping end

#define RAY_LENGTH zfar//40.0 //maximum ray length.
#define STEP_COUNT 256  //maximum sample count.
#define PIXEL_STRIDE 4   //sample multiplier. it's recommend 16 or 8.
#define PIXEL_THICKNESS (0.04 * PIXEL_STRIDE)   //how thick is a pixel. correct value reduces noise.

vec2 texel = vec2(1.0) / u_Dimensions;

vec3 DecodeNormal(in vec2 N)
{
	vec2 encoded = N*4.0 - 2.0;
	float f = dot(encoded, encoded);
	float g = sqrt(1.0 - f * 0.25);

	return vec3(encoded * g, 1.0 - f * 0.5);
}

//convenient function.
bool RayIntersect(float raya, float rayb, vec2 sspt, float thickness) 
{
    if (raya > rayb) {
        float t = raya;
        raya = rayb;
        rayb = t;
    }

#if 1
	// by default we use fixed thickness.
    float screenPCameraDepth = -texture(u_ScreenDepthMap, vec2(sspt * 0.5 + 0.5)).r;
    return raya < screenPCameraDepth && rayb > screenPCameraDepth - thickness;
#else
    float backZ = textureLod(_BackfaceTex, vec3(sspt * 0.5 + 0.5, 0)).r;
    return raya < backZ && rayb > screenPCameraDepth;
#endif
}


bool traceRay(vec3 start
				, vec3 direction
				, float jitter
				, vec4 texelSize
				, float maxRayLength
				, float maxStepCount
				, float pixelStride
				, float pixelThickness
				, out vec2 hitPixel
				, out float marchPercent
				, out float hitZ
				, out float rayLength) 
{
    //clamp raylength to near clip plane.
    //rayLength = ((start.z + direction.z * maxRayLength) > -_ProjectionParams.y) ?
    //    (-_ProjectionParams.y - start.z) / direction.z : maxRayLength;
	//rayLength = maxRayLength;
	rayLength = ((start.z + direction.z * maxRayLength) > -znear) ? (-znear - start.z) / direction.z : maxRayLength;

    vec3 end = start + direction * rayLength;

    vec4 H0 = u_ProjectionMatrix * vec4(start, 1.0);
    vec4 H1 = u_ProjectionMatrix * vec4(end, 1.0);

    vec2 screenP0 = H0.xy / H0.w;
    vec2 screenP1 = H1.xy / H1.w; 

    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    float Q0 = start.z * k0;
    float Q1 = end.z * k1;

    if (abs(dot(screenP1 - screenP0, screenP1 - screenP0)) < 0.00001) {
        screenP1 += texelSize.xy;
    }

    vec2 deltaPixels = (screenP1 - screenP0) * texelSize.zw;
    float step; //the sample rate.
    step = min(1.0 / abs(deltaPixels.y), 1.0 / abs(deltaPixels.x)); //make at least one pixel is sampled every time.

    //make sample faster.
    step *= pixelStride;
    float sampleScaler = 1.0 - min(1.0, -start.z / 100.0); //sample is slower when far from the screen.
    step *= 1.0 + sampleScaler; 

    float interpolationCounter = step;  //by default we use step instead of 0. this avoids some glitch.

    vec4 pqk = vec4(screenP0, Q0, k0);
    vec4 dpqk = vec4(screenP1 - screenP0, Q1 - Q0, k1 - k0) * step;

    pqk += jitter * dpqk;

    float prevZMaxEstimate = start.z;

    bool intersected = false;
    
	//the logic here is a little different from PostProcessing or (casual-effect). but it's all about raymarching.
    for (int i = 1; i <= maxStepCount && interpolationCounter <= 1 && !intersected; i++, interpolationCounter += step) 
	{
        pqk += dpqk;
        float rayZMin = prevZMaxEstimate;
        float rayZMax = pqk.z / pqk.w;

		vec2 c = (pqk.xy - dpqk.xy) * 0.5;

        if (RayIntersect(rayZMin, rayZMax, c, pixelThickness))
		{
            hitPixel = c * 0.5 + 0.5;
            marchPercent = float(i) / maxStepCount;
            intersected = true;
        }
        else 
		{
            prevZMaxEstimate = rayZMax;
        }
    }

#if 1     //binary search
    if (intersected) 
	{
        pqk -= dpqk;    //one step back

        for (float gapSize = pixelStride; gapSize > 1.0; gapSize *= 0.5) 
		{
            dpqk *= 0.5;
            float rayZMin = prevZMaxEstimate;
            float rayZMax = pqk.z / pqk.w;

            if (RayIntersect(rayZMin, rayZMax, pqk.xy - dpqk.xy * 0.5, pixelThickness)) 
			{// hit, stay the same.(but ray length is halfed)

            }
            else 
			{// miss the hit. we should step forward
                pqk += dpqk;
                prevZMaxEstimate = rayZMax;
            }
        }

        hitPixel = (pqk.xy - dpqk.xy * 0.5) * 0.5 + 0.5;
    }
#endif

    hitZ = pqk.z / pqk.w;
    rayLength *= (hitZ - start.z) / (end.z - start.z);
    return intersected;
}

vec4 fragDentisyAndOccluder(vec2 coord)   //we return dentisy in R, distance in G
{
	vec4 position = textureLod(u_PositionMap, coord, 0.0).xzyw;

	if (position.a-1.0 == MATERIAL_SKY || position.a-1.0 == MATERIAL_SUN)
	{// Skybox... Skip...
		return vec4(0.0);
	}

    float decodedDepth = texture(u_ScreenDepthMap, coord).r;
    //vec3 csRayOrigin = decodedDepth * var_Ray;
	//vec3 csRayOrigin = position.xyz - u_ViewOrigin.xzy;//position.xyz;
	vec3 csRayOrigin = decodedDepth * (u_ViewOrigin.xzy - position.xzy);
    
	//vec3 csNormal = normalize(texture(u_NormalMap, coord).rgb * 2.0 - 1.0);
	vec4 norm = textureLod(u_NormalMap, coord, 0.0);
	//norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	//norm.z = sqrt(1.0-dot(norm.xy, norm.xy)); // reconstruct Z from X and Y
	norm.xyz = DecodeNormal(norm.xy);
	vec3 csNormal = norm.xyz;
	
	vec3 csLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
    
	vec2 hitPixel;
    float marchPercent;
    vec3 debugCol;

    float atten = 0;

    float hitZ;
    float rayBump = max(-0.010*csRayOrigin.z, 0.001);
    float rayLength;
    
	bool intersectd = traceRay(
        csRayOrigin + csNormal * rayBump,
        csLightDir,
        0,        //don't need jitter here.
        vec4(1.0 / 991.0, 1.0 / 529.0, 991.0, 529.0),    //texel size. 
		//vec4(texel, u_Dimensions),    //texel size. 
        RAY_LENGTH,
        STEP_COUNT,
        PIXEL_STRIDE,
        PIXEL_THICKNESS,
        hitPixel,
        marchPercent,
        hitZ,
        rayLength
    );

    return intersectd ? vec4(1.0 , rayLength, 0.0, 1.0) : vec4(0.0);
}

void main() 
{
	gl_FragColor = fragDentisyAndOccluder(var_TexCoords);
}


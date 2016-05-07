#define USE_WATERMAP				// Needs to also be enabled in rend2 tr_local.h
//#define SIMPLIFIED_FRESNEL		// Seems no faster... not as good?
#define REAL_WAVES					// You probably always want this turned on.
//#define USE_UNDERWATER			// TODO: Convert from HLSL when I can be bothered.
#define USE_REFLECTION				// Enable reflections on water.
//#define DEBUG_WATER_LOCATION		// DEBUG: Simply displays where water is on the screen in blue, and non water in red.

/*
heightMap – height-map used for waves generation as described in the section “Modifying existing geometry”
backBufferMap – current contents of the back buffer
positionMap – texture storing scene position vectors (material type is alpha)
waterPositionMap – texture storing scene water position vectors (alpha is 1.0 when water is at this pixel. 0.0 if not).
waterPositionMap2 – texture storing scene water position vectors (This one is at slightly increased height to include max wave height).
normalMap – texture storing normal vectors for normal mapping as described in the section “The computation of normal vectors”
foamMap – texture containing foam – in my case it is a photo of foam converted to greyscale
*/


uniform sampler2D	u_HeightMap;
uniform sampler2D	u_DiffuseMap;   // backBufferMap
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_OverlayMap;   // foamMap

#if defined(USE_WATERMAP)
uniform sampler2D	u_WaterPositionMap;
uniform sampler2D	u_WaterPositionMap2;
#endif //defined(USE_WATERMAP)

uniform vec4		u_Local0; // testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1; // MAP_WATER_LEVEL, USE_GLSL_REFLECTION
uniform vec4		u_Local10; // waveHeight

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;

uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

// Position of the camera
uniform vec3		u_ViewOrigin;
#define ViewOrigin	u_ViewOrigin.xzy

// Timer
uniform float		u_Time;
#define timer		(u_Time * 5000.0)
#define wfTimer		(u_Time * 0.5)


#define WATER_DEPTH_HACK_LEVEL 10.0//8192.0

// Over-all water clearness...
//const float waterClarity = 0.001;
const float waterClarity = 0.03;
//float waterClarity = u_Local0.r;

// How fast will colours fade out. You can also think about this
// values as how clear water is. Therefore use smaller values (eg. 0.05f)
// to have crystal clear water and bigger to achieve "muddy" water.
const float fadeSpeed = 0.15;

// Normals scaling factor
const float normalScale = 1.0;

// R0 is a constant related to the index of refraction (IOR).
// It should be computed on the CPU and passed to the shader.
const float R0 = 0.5;

// Maximum waves amplitude
//const float maxAmplitude = 6.0;//4.0;
#define maxAmplitude u_Local10.r

// Direction of the light
//vec3 lightDir = vec3(0.0, 1.0, 0.0);

// Colour of the sun
vec3 sunColor = u_PrimaryLightColor.rgb;//vec3(1.0, 1.0, 1.0);

// The smaller this value is, the more soft the transition between
// shore and water. If you want hard edges use very big value.
// Default is 1.0f.
const float shoreHardness = 0.2;//1.0;

// This value modifies current fresnel term. If you want to weaken
// reflections use bigger value. If you want to empasize them use
// value smaller then 0. Default is 0.0.
const float refractionStrength = 0.0;
//float refractionStrength = -0.3;

// Modifies 4 sampled normals. Increase first values to have more
// smaller "waves" or last to have more bigger "waves"
const vec4 normalModifier = vec4(1.0, 2.0, 4.0, 8.0);

// Strength of displacement along normal.
const float displace = 1.7;

// Describes at what depth foam starts to fade out and
// at what it is completely invisible. The third value is at
// what height foam for waves appear (+ waterLevel).
const vec3 foamExistence = vec3(1.5, 5.35, 2.3); //vec3(0.65, 1.35, 0.5);

const float sunScale = 3.0;

const float shininess = 0.7;
const float specularScale = 0.07;


// Colour of the water surface
const vec3 depthColour = vec3(0.0078, 0.5176, 0.7);

// Colour of the water depth
//const vec3 bigDepthColour = vec3(0.0059, 0.3096, 0.445);
const vec3 bigDepthColour = vec3(0.0059, 0.1276, 0.18);
//vec3 bigDepthColour = vec3(u_Local0.r, u_Local0.g, u_Local0.b);

const vec3 extinction = vec3(7.0, 30.0, 40.0);			// Horizontal

// Water transparency along eye vector.
const float visibility = 32.0;

// Increase this value to have more smaller waves.
const vec2 scale = vec2(0.002, 0.002);
const float refractionScale = 0.005;

// Wind force in x and z axes.
const vec2 wind = vec2(-0.3, 0.7);


mat3 compute_tangent_frame(in vec3 N, in vec3 P, in vec2 UV) {
    vec3 dp1 = dFdx(P);
    vec3 dp2 = dFdy(P);
    vec2 duv1 = dFdx(UV);
    vec2 duv2 = dFdy(UV);
    // solve the linear system
    vec3 dp1xdp2 = cross(dp1, dp2);
    mat2x3 inverseM = mat2x3(cross(dp2, dp1xdp2), cross(dp1xdp2, dp1));
    vec3 T = inverseM * vec2(duv1.x, duv2.x);
    vec3 B = inverseM * vec2(duv1.y, duv2.y);
    // construct tangent frame
    float maxLength = max(length(T), length(B));
    T = T / maxLength;
    B = B / maxLength;
    return mat3(T, B, N);
}

// Function calculating fresnel term.
// - normal - normalized normal vector
// - eyeVec - normalized eye vector
float fresnelTerm(vec3 normal, vec3 eyeVec)
{
#ifdef SIMPLIFIED_FRESNEL
		// Simplified
		return R0 + (1.0f - R0) * pow(1.0f - dot(eyeVec, normal), 5.0f);
#else //!SIMPLIFIED_FRESNEL
		float angle = 1.0f - clamp(dot(normal, eyeVec), 0.0, 1.0);
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return clamp(fresnel * (1.0 - clamp(R0, 0.0, 1.0)) + R0 - refractionStrength, 0.0, 1.0);
#endif //SIMPLIFIED_FRESNEL
}

vec4 waterMapAtCoord ( vec2 coord )
{
#if defined(USE_WATERMAP)
	return texture2D(u_WaterPositionMap, coord).xzya;
#else //!defined(USE_WATERMAP)
	return vec4(0.0, u_Local1.r, 0.0, 0.0);
#endif //defined(USE_WATERMAP)
}

#if defined(USE_WATERMAP)
vec4 waterMap2AtCoord ( vec2 coord )
{
	return texture2D(u_WaterPositionMap2, coord).xzya;
}
#endif //defined(USE_WATERMAP)

vec4 positionMapAtCoord ( vec2 coord )
{
	return texture2D(u_PositionMap, coord).xzya;
}

float pw = (1.0/u_Dimensions.x);
float ph = (1.0/u_Dimensions.y);

vec3 AddReflection(vec2 coord, vec3 positionMap, vec3 waterMap, vec3 inColor)
{
	if (positionMap.y > waterMap.y)
	{
		return inColor;
	}

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y < 1.0; y += ph * 5.0)
	{
		vec4 wMap = waterMapAtCoord(vec2(coord.x, y));
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));
		float isWater = wMap.a;

		if (isWater <= 0.0 && pMap.y >= waterMap.y)
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0)
	{// Found no non-water surfaces...
		return inColor;
	}
	
	QLAND_Y -= ph * 5.0;
	
	// Full scan from within 5 px for the real 1st pixel...
	float upPos = coord.y;
	float LAND_Y = 0.0;

	for (float y = QLAND_Y; y < 1.0; y += ph)
	{
		vec4 wMap = waterMapAtCoord(vec2(coord.x, y));
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));
		float isWater = wMap.a;

		if (isWater <= 0.0 && pMap.y >= waterMap.y)
		{
			LAND_Y = y;
			break;
		}
	}

	if (LAND_Y <= 0.0)
	{// Found no non-water surfaces...
		return inColor;
	}

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return inColor;
	}


	vec4 wMap = waterMapAtCoord(vec2(coord.x, upPos));

	if (wMap.a > 0.0 /*|| distance(wMap.xyz, ViewOrigin) > distance(waterMap.xyz, ViewOrigin)*/)
	{// This position is water, or it is closer then the reflection pixel...
		return inColor;
	}

	vec4 landColor = texture2D(u_DiffuseMap, vec2(coord.x, upPos));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x + pw, upPos));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x - pw, upPos));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x, upPos + ph));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x, upPos - ph));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x + pw, upPos + ph));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x - pw, upPos - ph));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x + pw, upPos - ph));
	landColor += texture2D(u_DiffuseMap, vec2(coord.x - pw, upPos + ph));
	landColor /= 9.0;

	return mix(inColor.rgb, landColor.rgb, vec3(1.0 - pow(upPos, 4.0)) * 0.28/*u_Local0.r*/);
}


void main ( void )
{
	bool pixelIsInWaterRange = false;
	bool pixelIsWaterfall = false;
	bool depthHacked = false;
	vec3 color2 = texture2D(u_DiffuseMap, var_TexCoords).rgb;
	vec3 color = color2;

	vec4 waterMap = waterMapAtCoord(var_TexCoords);
#if defined(USE_WATERMAP)
	vec4 waterMap2 = waterMap2AtCoord(var_TexCoords);
#endif //defined(USE_WATERMAP)
	vec3 position = positionMapAtCoord(var_TexCoords).xyz;

	if (waterMap.a > 0.0)
	{// Fix for wierd water bugs.
		if (position.y == 0.0)
		{
			position.xyz = waterMap.xyz;
			position.y -= WATER_DEPTH_HACK_LEVEL;
			depthHacked = true;
		}

		float dist = length(position.y - waterMap.y);

		if (dist == 0.0 || dist >= 10.0)
		{
			position.xyz = waterMap.xyz;
			position.y -= WATER_DEPTH_HACK_LEVEL;
			depthHacked = true;
		}
	}

#if defined(USE_WATERMAP)
	if (waterMap.a > 0.0 || waterMap2.a > 0.0 || (waterMap.y != 0.0 && position.y <= waterMap.y + maxAmplitude))
#else //!defined(USE_WATERMAP)
	if (waterMap.a > 0.0 || (waterMap.y != 0.0 && position.y <= waterMap.y + maxAmplitude))
#endif //defined(USE_WATERMAP)
	{
		pixelIsInWaterRange = true;
	}

#if defined(DEBUG_WATER_LOCATION)
	// Debug basic "is water here" map...
	if (waterMap.a > 0.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
	}
	else if (waterMap2.a > 0.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
		return;
	}
#endif //defined(DEBUG_WATER_LOCATION)
	
	if (!pixelIsInWaterRange)
	{// No water here. Skip calculations.
		gl_FragColor = vec4(color2, 1.0);
		return;
	}

	//vec3 lightDir = ViewOrigin.xyz - u_PrimaryLightOrigin.xzy;
	vec3 lightDir = waterMap2.xyz - u_PrimaryLightOrigin.xzy;
	//lightDir.xz = (-lightDir.xz + -lightDir.xz + normalize(-lightDir.xz)) / 3.0;
	lightDir.xz = -lightDir.xz;

#if defined(USE_WATERMAP)
	float waterLevel = waterMap2.y;
	float level = waterMap2.y;
#else //!defined(USE_WATERMAP)
	float waterLevel = waterMap.y;
	float level = waterMap.y;
#endif //defined(USE_WATERMAP)
	float depth = 0.0;

	if (waterMap.a >= 2.0 || waterMap2.a >= 2.0)
	{// Low horizontal normal, this is a waterfall...
		pixelIsWaterfall = true;
	}

	// If we are underwater let's leave out complex computations
	if (level >= ViewOrigin.y && !pixelIsWaterfall)
	{
#if defined(USE_UNDERWATER)
		depth = length(position - cameraPos);
		float depthN = depth * fadeSpeed;
		float depth2 = level - cameraPos.y;
		
		vec3 waterCol = saturate(length(sunColor) / sunScale);
		waterCol = waterCol * lerp(depthColour, bigDepthColour, saturate(depth2 / extinction));
			
		if (position.y <= level)
		{
			color2 = color2 - color2 * saturate(depth2 / extinction);
			color = lerp(color2, waterCol, saturate(depthN / visibility));
		}
		else
		{
			vec3 eyeVec = position - cameraPos;	
			vec3 eyeVecNorm = normalize(eyeVec);
			float t = (level - cameraPos.y) / eyeVecNorm.y;
			vec3 surfacePoint = cameraPos + eyeVecNorm * t;
			
			eyeVecNorm = normalize(eyeVecNorm);
			depth = length(surfacePoint - cameraPos);
			float depthN = depth * fadeSpeed;
			
			float depth2 = level - cameraPos.y;
			
			vec2 texCoord = 0;
			texCoord = IN.texCoord.xy;
			texCoord.x += sin(timer * 0.002f + 3.0f * abs(position.y)) * (refractionScale);
			color2 = tex2D(backBufferMap, texCoord).rgb;
			
			color2 = color2 - color2 * saturate(depth2 / extinction);
			color = lerp(color2, waterCol, saturate(depthN / visibility));
			
			vec3 myNormal = normalize(vec3(0.0f, 1.0f, 0.0f));
		
			texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
			vec3x3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
			vec3 normal0a = normalize(mul((2.0f * tex2D(normalMap, texCoord) - 1.0f).xyz, tangentFrame).xyz);
	
			texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
			tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
			vec3 normal1a = normalize(mul((2.0f * tex2D(normalMap, texCoord) - 1.0f).xyz, tangentFrame).xyz);
			
			texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
			tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
			vec3 normal2a = normalize(mul((2.0f * tex2D(normalMap, texCoord) - 1.0f).xyz, tangentFrame).xyz);
			
			texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
			tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
			vec3 normal3a = normalize(mul((2.0f * tex2D(normalMap, texCoord) - 1.0f).xyz, tangentFrame).xyz);
			
			vec3 normal = normalize(normal0a * normalModifier.x + normal1a * normalModifier.y +
									  normal2a * normalModifier.z + normal3a * normalModifier.w);
									  
			vec3 mirrorEye = (2.0f * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
			float dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5f + 0.5f);
			vec3 fresnel = 0;
			vec3 specular = (1.0f - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 512.0f)) * (shininess * 1.8f + 0.2f))* sunColor;
			specular += specular * 25 * saturate(shininess - 0.05f) * sunColor;
		}
		
		gl_FragColor = vec4(color, 1.0);
		return;
#else //!defined(USE_UNDERWATER)
		gl_FragColor = vec4(color2, 1.0);
		return;
#endif //defined(USE_UNDERWATER)
	}

	if (pixelIsWaterfall)
	{// How???
		vec2 texCoord = var_TexCoords.xy;
		texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * refractionScale;

		vec3 refraction = texture2D(u_DiffuseMap, texCoord).rgb;
		gl_FragColor = vec4(mix(depthColour, refraction, 0.7), 1.0);
		return;
	}

	if (pixelIsInWaterRange || pixelIsWaterfall)
	{
		vec3 eyeVec = position - ViewOrigin;
		float cameraDepth = ViewOrigin.y - position.y;
		
		// Find intersection with water surface
		vec3 eyeVecNorm = normalize(eyeVec);
		float t = (level - ViewOrigin.y) / eyeVecNorm.y;
		vec3 surfacePoint = ViewOrigin + eyeVecNorm * t;

		eyeVecNorm = normalize(eyeVecNorm);


		vec2 texCoord;

#ifdef REAL_WAVES
		for(int i = 0; i < 10; i++)
#endif //REAL_WAVES
		{
			texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * scale + timer * 0.000005 * wind;
			
			float bias = texture2D(u_HeightMap, texCoord).r;
	
			bias *= 0.1;
			level += bias * maxAmplitude;

			t = (level - ViewOrigin.y) / eyeVecNorm.y;
			surfacePoint = ViewOrigin + eyeVecNorm * t;
		}

		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;

		eyeVecNorm = normalize(ViewOrigin - surfacePoint);

		float normal1 = texture2D(u_HeightMap, (texCoord + (vec2(-1.0, 0.0) / 256.0))).r;
		float normal2 = texture2D(u_HeightMap, (texCoord + (vec2(1.0, 0.0) / 256.0))).r;
		float normal3 = texture2D(u_HeightMap, (texCoord + (vec2(0.0, -1.0) / 256.0))).r;
		float normal4 = texture2D(u_HeightMap, (texCoord + (vec2(0.0, 1.0) / 256.0))).r;
		
		vec3 myNormal = normalize(vec3((normal1 - normal2) * maxAmplitude,
										   normalScale,
										   (normal3 - normal4) * maxAmplitude));

		texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
		mat3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal0a = normalize(tangentFrame * (texture2D(u_NormalMap, texCoord).xyz * 2.0 - 1.0));

		texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal1a = normalize(tangentFrame * (texture2D(u_NormalMap, texCoord).xyz * 2.0 - 1.0));
		
		texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal2a = normalize(tangentFrame * (texture2D(u_NormalMap, texCoord).xyz * 2.0 - 1.0));
		
		texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal3a = normalize(tangentFrame * (texture2D(u_NormalMap, texCoord).xyz * 2.0 - 1.0));
		
		vec3 normal = normalize(normal0a * normalModifier.x + normal1a * normalModifier.y +
								  normal2a * normalModifier.z + normal3a * normalModifier.w);

		texCoord = var_TexCoords.xy;

		if (!depthHacked)
			texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * (refractionScale * min(depth2, 1.0));

		vec3 refraction = texture2D(u_DiffuseMap, texCoord).rgb;

		vec4 position2 = positionMapAtCoord(texCoord);
		vec4 waterMap3 = waterMapAtCoord(texCoord);

		if (waterMap3.a > 0.0)
		{// Fix for wierd water bugs.
			if (position.y == 0.0)
			{
				position2.xyz = waterMap3.xyz;
				position2.y -= WATER_DEPTH_HACK_LEVEL;
			}

			float dist = length(position2.y - waterMap3.y);

			if (dist == 0.0 || dist >= 10.0)
			{
				position2.xyz = waterMap3.xyz;
				position2.y -= WATER_DEPTH_HACK_LEVEL;
			}
		}

		if (position2.y > level)
		{
			refraction = color2;
		}

		float fresnel = fresnelTerm(normal, eyeVecNorm);
		
		float depthN = depth * fadeSpeed;
		float waterCol = clamp(length(sunColor) / sunScale, 0.0, 1.0);
		
		vec3 refraction1 = mix(refraction, depthColour * vec3(waterCol), clamp(vec3(depthN) / vec3(visibility), 0.0, 1.0));
		refraction = mix(refraction1, bigDepthColour * vec3(waterCol), clamp(vec3(depth2) / vec3(extinction), 0.0, 1.0));

		float foam = 0.0;

		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001 * wind + sin(timer * 0.001 + position.x) * 0.005;
		vec2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00002 * wind + sin(timer * 0.001 + position.z) * 0.005;
		
		// .r added to below lookups because the original had no swizzle... no idea if it is right...
		if (depth2 < foamExistence.x)
		{
			foam = (texture2D(u_OverlayMap, texCoord).r + texture2D(u_OverlayMap, texCoord2).r) * 0.5;
		}
		else if (depth2 < foamExistence.y)
		{
			foam = mix((texture2D(u_OverlayMap, texCoord).r + texture2D(u_OverlayMap, texCoord2).r) * 0.5, 0.0,
						 (depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x));
		}
		
		if (maxAmplitude - foamExistence.z > 0.0001)
		{
			foam += (texture2D(u_OverlayMap, texCoord).r + texture2D(u_OverlayMap, texCoord2).r) * 0.5 * 
				clamp((level - (waterLevel + foamExistence.z)) / (maxAmplitude - foamExistence.z), 0.0, 1.0);
		}


		vec3 specular = vec3(0.0);

		/*
		vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		float dotSpec = clamp(dot(mirrorEye.xyz, -lightDir) * 0.5 + 0.5, 0.0, 1.0);
		specular = vec3(1.0 - fresnel) * clamp(-lightDir.y, 0.0, 1.0) * ((pow(dotSpec, 512.0)) * (shininess * 1.8 + 0.2)) * sunColor * specularScale;
		specular += specular * 25.0 * clamp(shininess - 0.05, 0.0, 1.0) * sunColor * specularScale;
		*/

		
		float lambertian2 = dot(-lightDir.xyz, normal);
		float spec2 = 0.0;

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
			vec3 halfDir2 = normalize(-lightDir.xyz + mirrorEye);
			float specAngle = max(dot(halfDir2, normal), 0.0);
			spec2 = pow(specAngle, 16.0);
			specular = vec3(1.0 - fresnel) * (vec3(spec2 * shininess)) * sunColor * specularScale * 25.0;
		}
		

		/*
		vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		vec3 vRef = normalize(reflect(-lightDir, normal));
		float stemp = max(0.0, dot(mirrorEye, vRef) );
		stemp = pow(stemp, 64.0);//exponent);
		specular = vec3(stemp) * sunColor * specularScale;
		*/

#if defined(USE_REFLECTION)
		if (u_Local1.g >= 2.0)
		{
			color = mix(refraction, bigDepthColour, fresnel);
			color = AddReflection(var_TexCoords, position, vec3(waterMap3.x, level, waterMap3.z), color.rgb);
		}
		else
		{
			color = mix(refraction, bigDepthColour, fresnel);
		}
#else //!defined(USE_REFLECTION)
		color = mix(refraction, bigDepthColour, fresnel);
#endif //defined(USE_REFLECTION)
		color = clamp(color + max(specular, foam * sunColor), 0.0, 1.0);
		
		color = mix(refraction, color, clamp(depth * shoreHardness, 0.0, 1.0));

		color = mix(color, color2, 1.0 - clamp(waterClarity * depth, 0.8, 1.0));

#if defined(USE_WATERMAP)
		if (position.y > level && waterMap.a <= 0.0)
#else //!defined(USE_WATERMAP)
		if (position.y > level)
#endif //defined(USE_WATERMAP)
		{// Waves against shoreline. Pixel is above waterLevel + waveHeight... (but ignore anything marked as actual water - eg: not a shoreline)
			color = color2;
		}
	}

	gl_FragColor = vec4(color, 1.0);
}

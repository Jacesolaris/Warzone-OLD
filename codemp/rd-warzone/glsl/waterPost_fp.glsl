//#define SIMPLIFIED_FRESNEL		// Seems no faster... not as good?
#define REAL_WAVES					// You probably always want this turned on.
#define USE_UNDERWATER				// TODO: Convert from HLSL when I can be bothered.
#define USE_REFLECTION				// Enable reflections on water.
#define FIX_WATER_DEPTH_ISSUES		// Use basic depth value for sky hits...
//#define __TEST_WATER__				// Testing experimental water...

/*
heightMap – height-map used for waves generation as described in the section “Modifying existing geometry”
backBufferMap – current contents of the back buffer
positionMap – texture storing scene position vectors (material type is alpha)
waterPositionMap – texture storing scene water position vectors (alpha is 1.0 when water is at this pixel. 0.0 if not).
waterPositionMap2 – texture storing scene water position vectors (This one is at slightly increased height to include max wave height).
normalMap – texture storing normal vectors for normal mapping as described in the section “The computation of normal vectors”
foamMap – texture containing foam – in my case it is a photo of foam converted to greyscale
*/

uniform mat4		u_ModelViewProjectionMatrix;

uniform sampler2D	u_WaterHeightMap;
uniform sampler2D	u_DiffuseMap;			// backBufferMap
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_OverlayMap;			// foamMap
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_DeluxeMap;			// noise

uniform sampler2D	u_WaterPositionMap;
uniform sampler2D	u_WaterPositionMap2;

uniform vec4		u_Mins;					// MAP_MINS[0], MAP_MINS[1], MAP_MINS[2], 0.0
uniform vec4		u_Maxs;					// MAP_MAXS[0], MAP_MAXS[1], MAP_MAXS[2], 0.0
uniform vec4		u_MapInfo;				// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec4		u_Local0;				// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;				// MAP_WATER_LEVEL, USE_GLSL_REFLECTION, IS_UNDERWATER
uniform vec4		u_Local2;				// WATER_COLOR_SHALLOW_R, WATER_COLOR_SHALLOW_G, WATER_COLOR_SHALLOW_B
uniform vec4		u_Local3;				// WATER_COLOR_DEEP_R, WATER_COLOR_DEEP_G, WATER_COLOR_DEEP_B
uniform vec4		u_Local4;				// FOG_COLOR
uniform vec4		u_Local5;				// FOG_COLOR_SUN
uniform vec4		u_Local6;				// FOG_DENSITY
uniform vec4		u_Local10;				// waveHeight, waveDensity

uniform vec2		u_Dimensions;
uniform vec4		u_ViewInfo;				// zmin, zmax, zmax / zmin

varying vec2		var_TexCoords;

uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

#define MAX_DEFERRED_LIGHTS 128//64//16//24

uniform int			u_lightCount;
uniform vec3		u_lightPositions2[MAX_DEFERRED_LIGHTS];
uniform float		u_lightDistances[MAX_DEFERRED_LIGHTS];
uniform float		u_lightHeightScales[MAX_DEFERRED_LIGHTS];
uniform vec3		u_lightColors[MAX_DEFERRED_LIGHTS];

// Position of the camera
uniform vec3		u_ViewOrigin;
#define ViewOrigin	u_ViewOrigin.xzy

// Timer
uniform float		u_Time;
#define systemtimer		(u_Time * 5000.0)


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
//const float waveHeight = 6.0;//4.0;
#define waveHeight u_Local10.r
#define waveDensity u_Local10.g

// Colour of the sun
//vec3 sunColor = (u_PrimaryLightColor.rgb + vec3(1.0) + vec3(1.0) + vec3(1.0)) / 4.0; // 1/4 real sun color, 3/4 white...
vec3 sunColor = vec3(1.0);

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

// Describes at what depth foam starts to fade out and
// at what it is completely invisible. The third value is at
// what height foam for waves appear (+ waterLevel).
//const vec3 foamExistence = vec3(1.5, 5.35, 2.3); //vec3(0.65, 1.35, 0.5);
//const vec3 foamExistence = vec3(1.5, 50.0, waveHeight * 0.85/*5.0*/);
#define foamExistence vec3(1.5, 50.0, waveHeight * 0.85/*5.0*/)
//vec3 foamExistence = vec3(u_Local0.r, u_Local0.g, u_Local0.b);

const float sunScale = 3.0;

const float shininess = 0.7;
const float specularScale = 0.07;


// Colour of the water surface
//const vec3 waterColorShallow = vec3(0.0078, 0.5176, 0.7);
vec3 waterColorShallow = u_Local2.rgb;

// Colour of the water depth
//const vec3 waterColorDeep = vec3(0.0059, 0.1276, 0.18);
vec3 waterColorDeep = u_Local3.rgb;

//const vec3 extinction = vec3(7.0, 30.0, 40.0);			// Horizontal
//const vec3 extinction = vec3(7.0, 96.0, 128.0);			// Horizontal
const vec3 extinction = vec3(35.0, 480.0, 640.0);
//vec3 extinction = vec3(u_Local0.r, u_Local0.g, u_Local0.b);

// Water transparency along eye vector.
//const float visibility = 32.0;
const float visibility = 320.0;

// Increase this value to have more smaller waves.
const vec2 scale = vec2(0.002, 0.002);
const float refractionScale = 0.005;

// Wind force in x and z axes.
//const vec2 wind = vec2(-0.3, 0.7);


float linearize(float depth)
{
	return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

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

vec4 waterMapLowerAtCoord ( vec2 coord )
{
	vec4 wmap = textureLod(u_WaterPositionMap, coord, 0.0).xzyw;
	wmap.y -= waveHeight;
	return wmap;
}

vec4 waterMapUpperAtCoord ( vec2 coord )
{
	//return textureLod(u_WaterPositionMap2, coord, 0.0).xzyw;
	return textureLod(u_WaterPositionMap, coord, 0.0).xzyw;
}

vec4 positionMapAtCoord ( vec2 coord )
{
	return textureLod(u_PositionMap, coord, 0.0).xzyw;
}

float pw = (1.0/u_Dimensions.x);
float ph = (1.0/u_Dimensions.y);

vec3 AddReflection(vec2 coord, vec3 positionMap, vec3 waterMapLower, vec3 inColor)
{
	if (positionMap.y > waterMapLower.y)
	{
		return inColor;
	}

	vec4 wMapCheck = waterMapLowerAtCoord(vec2(coord.x, 1.0));
	if (wMapCheck.a > 0.0)
	{// Top of screen pixel is water, don't check...
		return inColor;
	}

	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y <= 1.0; y += ph * 5.0)
	{
		vec4 wMap = waterMapLowerAtCoord(vec2(coord.x, y));
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));
		float isWater = wMap.a;

		if (isWater <= 0.0 && (pMap.y >= waterMapLower.y || length(pMap.xyz) == 0.0))
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0 || QLAND_Y >= 1.0)
	{// Found no non-water surfaces...
		return inColor;
	}
	
	QLAND_Y -= ph * 5.0;
	
	// Full scan from within 5 px for the real 1st pixel...
	float upPos = coord.y;
	float LAND_Y = 0.0;

	for (float y = QLAND_Y; y <= 1.0; y += ph)
	{
		vec4 wMap = waterMapLowerAtCoord(vec2(coord.x, y));
		vec4 pMap = positionMapAtCoord(vec2(coord.x, y));
		float isWater = wMap.a;

		if (isWater <= 0.0 && (pMap.y >= waterMapLower.y || length(pMap.xyz) == 0.0))
		{
			LAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0 || QLAND_Y >= 1.0)
	{// Found no non-water surfaces...
		return inColor;
	}

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return inColor;
	}


	vec4 wMap = waterMapLowerAtCoord(vec2(coord.x, upPos));

	if (wMap.a > 0.0)
	{// This position is water, or it is closer then the reflection pixel...
		return inColor;
	}

	vec4 landColor = textureLod(u_DiffuseMap, vec2(coord.x, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos + ph), 0.0);
	landColor /= 9.0;

	return mix(inColor.rgb, landColor.rgb, vec3(1.0 - pow(upPos, 4.0)) * 0.28/*u_Local0.r*/);
}

// lighting
float getdiffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}
float getspecular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (3.1415 * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor) {
	vec3 halfLV = normalize(light + view);
	float spe = pow(max(dot(normal, halfLV), 0.0), 32.0);
	float dif = dot(normal, light) * 0.5 + 0.75;
	return dif*diffuseColor + spe*specularColor;
}

void main ( void )
{
	vec3 color2 = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
	vec4 waterMapUpper = waterMapUpperAtCoord(var_TexCoords);
	waterMapUpper.y += waveHeight;
	bool IS_UNDERWATER = false;

	if (u_Local1.b > 0.0) 
	{
		IS_UNDERWATER = true;
	}

	if (waterMapUpper.a <= 0.0)
	{// Should be safe to skip everything.
		gl_FragColor = vec4(color2, 1.0);
		return;
	}

	bool pixelIsInWaterRange = false;
	bool pixelIsUnderWater = false;
	bool pixelIsWaterfall = false;
	vec3 color = color2;

	vec4 waterMapLower = waterMapUpper;//waterMapLowerAtCoord(var_TexCoords);
	waterMapLower.y -= waveHeight;

	vec4 positionMap = positionMapAtCoord(var_TexCoords);
	vec3 position = positionMap.xyz;

#if defined(FIX_WATER_DEPTH_ISSUES)
	if (positionMap.a == 1024.0)
	{
		position.xyz = waterMapLower.xyz;
		position.y -= 1024.0;
	}
#endif //defined(FIX_WATER_DEPTH_ISSUES)

	if (waterMapLower.a >= 2.0 || waterMapUpper.a >= 2.0)
	{// Low horizontal normal, this is a waterfall...
		pixelIsWaterfall = true;
	}
	else if (IS_UNDERWATER || waterMapLower.y > ViewOrigin.y)
	{
		pixelIsUnderWater = true;
	}
	else if (waterMapUpper.y >= position.y)
	{
		if (waterMapLower.y < position.y)
		{
			waterMapLower.y = position.y;
			waterMapLower.a = 0.0;
		}

		if (waterMapUpper.y >= position.y)
		{
			pixelIsInWaterRange = true;
		}
	}

	if (!pixelIsInWaterRange && !pixelIsWaterfall && !pixelIsUnderWater)
	{// No water here. Skip calculations.
		gl_FragColor = vec4(color2, 1.0);
		return;
	}

	float timer = systemtimer * (waveHeight / 16.0);

	if (pixelIsWaterfall)
	{// How???
		vec3 eyeVecNorm = normalize(ViewOrigin - waterMapUpper.xyz);
		vec3 pixelDir = eyeVecNorm;
		vec3 normal = normalize(pixelDir + (color.rgb * 0.5 - 0.25));
		vec3 specular = vec3(0.0);
		vec3 lightDir = normalize(ViewOrigin.xyz - u_PrimaryLightOrigin.xzy);
		float lambertian2 = dot(lightDir.xyz, normal);
		float spec2 = 0.0;
		float fresnel = clamp(1.0 - dot(normal, -eyeVecNorm), 0.0, 1.0);
		fresnel = pow(fresnel, 3.0) * 0.65;

		vec2 texCoord = var_TexCoords.xy;
		texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * refractionScale;

		vec3 refraction = textureLod(u_DiffuseMap, texCoord, 0.0).rgb;
		refraction = mix(waterColorShallow.rgb, refraction.rgb, 0.3);

		float fTime = u_Time * 2.0;
		vec3 foam = (texture(u_OverlayMap, vec2(waterMapUpper.x * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb + texture(u_OverlayMap, vec2(waterMapUpper.z * 0.03, (waterMapUpper.y * 0.03) + fTime)).rgb) * 0.5;

		color = mix(color + (foam * sunColor), refraction, fresnel * 0.8);

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
			vec3 halfDir2 = normalize(lightDir.xyz + mirrorEye);
			float specAngle = max(dot(halfDir2, normal), 0.0);
			spec2 = pow(specAngle, 16.0);
			specular = vec3(clamp(1.0 - fresnel, 0.4, 1.0)) * (vec3(spec2 * shininess)) * sunColor * specularScale * 25.0;
		}

		color = clamp(color + specular, 0.0, 1.0);
		color += vec3(getspecular(normal, lightDir, eyeVecNorm, 60.0));
		color = AddReflection(texCoord, position.xyz, waterMapUpper.xyz, color);

		gl_FragColor = vec4(color, 1.0);
		return;
	}

#ifdef __TEST_WATER__
	if (pixelIsInWaterRange || pixelIsUnderWater)
	{
		vec4 col2 = vec4(0.0);
		vec3 screenCenterOrg = positionMapAtCoord(vec2(0.0)).xzy;
		mainImage(col2, var_TexCoords, positionMap.xzyw, waterMapLower.xzyw, waterMapUpper.xzyw, screenCenterOrg);
		color.rgb = col2.rgb;
	}
#else //!__TEST_WATER__

	float waterLevel = waterMapLower.y;
	float level = waterMapLower.y;
	float depth = 0.0;

	if (pixelIsInWaterRange || pixelIsUnderWater)
	{
		vec2 wind = normalize(waterMapLower.xz); // Waves head toward center of map. Should suit current WZ maps...

		// Find intersection with water surface
		//vec3 eyeVecNorm = normalize(waterMapUpper.xyz - ViewOrigin);
		//vec3 eyeVecNorm = normalize(waterMapLower.xyz - ViewOrigin);
		vec3 eyeVecNorm = normalize(ViewOrigin - waterMapLower.xyz/*position*/);
		float t = ((level - ViewOrigin.y) / eyeVecNorm.y);
		vec3 surfacePoint = ViewOrigin + eyeVecNorm * t;
		//vec3 surfacePoint = waterMapLower.xyz;


		vec2 texCoord;
		//level = 0.0;

#ifdef REAL_WAVES
//#pragma unroll 10
		for(int i = 0; i < 10; i++)
#endif //REAL_WAVES
		{
			texCoord = ((surfacePoint.xz + eyeVecNorm.xz * 0.1) * scale + timer * 0.000005 * wind) * waveDensity;
			
			float bias = textureLod(u_WaterHeightMap, texCoord, 0.0).r;
	
			bias *= 0.1;
			level += bias * waveHeight;

			t = ((level - ViewOrigin.y) / eyeVecNorm.y);
			surfacePoint = ViewOrigin + eyeVecNorm * t;
		}

		//level = surfacePoint.y;

		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;
		float depthN = depth * fadeSpeed;

		float heightAboveGround = clamp(depth2 / waveHeight, 0.0, 1.0);

		if (pixelIsUnderWater)
		{
			//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			//return;

			depth = length(surfacePoint - position);
			depth2 = position.y - surfacePoint.y;
			depthN = depth * fadeSpeed;

			if (position.y <= level)
			{// This pixel is below the water line... Fast path...
				vec3 waterCol = clamp(length(sunColor) / vec3(sunScale), 0.0, 1.0);
				waterCol = waterCol * mix(waterColorShallow, waterColorDeep, clamp(depth2 / extinction, 0.0, 1.0));

				color2 = color2 - color2 * clamp(depth2 / extinction, 0.0, 1.0);
				color = mix(color2, waterCol, clamp(depthN / visibility, 0.0, 1.0));

				gl_FragColor = vec4(color, 1.0);
				return;
			}
		}
		/*else if (position.y > level && waterMapUpper.a <= 0.0)
		{// Waves against shoreline. Pixel is above waterLevel + waveHeight... (but ignore anything marked as actual water - eg: not a shoreline)
			gl_FragColor = vec4(color2, 1.0);
			return;
		}*/
		else if (depth2 < 0.0)
		{// Waves against shoreline. Pixel is above waterLevel + waveHeight... (but ignore anything marked as actual water - eg: not a shoreline)
			gl_FragColor = vec4(color2, 1.0);
			return;
		}

		//gl_FragColor = vec4(vec3(depth2 / waveHeight), 1.0);
		//return;

		eyeVecNorm = normalize(ViewOrigin - surfacePoint);
		//eyeVecNorm = normalize(ViewOrigin - waterMapUpper.xyz);

		float normal1 = textureLod(u_WaterHeightMap, (texCoord + (vec2(-1.0, 0.0) / 256.0)), 0.0).r;
		float normal2 = textureLod(u_WaterHeightMap, (texCoord + (vec2(1.0, 0.0) / 256.0)), 0.0).r;
		float normal3 = textureLod(u_WaterHeightMap, (texCoord + (vec2(0.0, -1.0) / 256.0)), 0.0).r;
		float normal4 = textureLod(u_WaterHeightMap, (texCoord + (vec2(0.0, 1.0) / 256.0)), 0.0).r;
		
		vec3 myNormal = normalize(vec3((normal1 - normal2) * waveHeight,
										   normalScale,
										   (normal3 - normal4) * waveHeight));

		texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
		mat3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal0a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));

		texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal1a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));
		
		texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal2a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));
		
		texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal3a = normalize(tangentFrame * (2.0 * textureLod(u_NormalMap, texCoord, 0.0).xyz - 1.0));
		
		vec3 normal = normalize(((normal0a * normalModifier.x) + (normal1a * normalModifier.y) +
						(normal2a * normalModifier.z) + (normal3a * normalModifier.w)));

		texCoord = var_TexCoords.xy;

		texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * (refractionScale * min(depth2, 1.0));

		vec3 refraction = textureLod(u_DiffuseMap, texCoord, 0.0).rgb;

		vec4 position2 = positionMapAtCoord(texCoord);
		vec4 waterMapLower3 = waterMapLowerAtCoord(texCoord);

#if defined(FIX_WATER_DEPTH_ISSUES)
		if (position2.a == 1024.0)
		{
			position2.xyz = waterMapLower3.xyz;
			position2.y -= 1024.0;
		}
#endif //defined(FIX_WATER_DEPTH_ISSUES)

		if (!pixelIsUnderWater && position2.y > level)
		{
			refraction = color2;
		}

		float waterCol = clamp(length(sunColor) / sunScale, 0.0, 1.0);
		
		vec3 refraction1 = mix(refraction, waterColorShallow * vec3(waterCol), clamp(vec3(depthN) / vec3(visibility), 0.0, 1.0));
		refraction = mix(refraction1, waterColorDeep * vec3(waterCol), clamp(vec3(depth2) / vec3(extinction), 0.0, 1.0));

		vec4 foam = vec4(0.0);

		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001 * wind + sin(timer * 0.001 + position.x) * 0.005;
		vec2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00002 * wind + sin(timer * 0.001 + position.z) * 0.005;
		
		
		if (depth2 < foamExistence.x)
		{
			foam = (texture(u_OverlayMap, texCoord) + texture(u_OverlayMap, texCoord2)) * 0.5;
		}
		else if (depth2 < foamExistence.y)
		{
			foam = mix((texture(u_OverlayMap, texCoord) + texture(u_OverlayMap, texCoord2)) * 0.5, vec4(0.0),
						 vec4((depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x)));
		}
		
		
		if (waveHeight - foamExistence.z > 0.0001)
		{
			foam += (texture(u_OverlayMap, texCoord) + texture(u_OverlayMap, texCoord2)) * 0.5 * 
				clamp((level - (waterLevel + foamExistence.z)) / (waveHeight - foamExistence.z), 0.0, 1.0);
		}
		

		vec3 specular = vec3(0.0);

		vec3 lightDir = normalize(ViewOrigin.xyz - u_PrimaryLightOrigin.xzy);
		//normal *= -myNormal.xyz;

		float lambertian2 = dot(lightDir.xyz, normal);
		float spec2 = 0.0;
		
		float fresnel = clamp(1.0 - dot(normal, -eyeVecNorm), 0.0, 1.0);
		fresnel = pow(fresnel, 3.0) * 0.65;

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
			vec3 halfDir2 = normalize(lightDir.xyz + mirrorEye);
			float specAngle = max(dot(halfDir2, normal), 0.0);
			spec2 = pow(specAngle, 16.0);
			specular = vec3(clamp(1.0 - fresnel, 0.4, 1.0)) * (vec3(spec2 * shininess)) * sunColor * specularScale * 25.0;
		}


#if 1
		/* TESTING */
		vec3 dist = -eyeVecNorm;

		color = mix(refraction, waterColorDeep, fresnel);

		float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
		color += waterColorShallow.rgb * (clamp(waveHeight - waterMapLower.y, 0.0, 1.0))* 0.18 * atten;

		color += vec3(getspecular(normal, lightDir, eyeVecNorm, 60.0));
		/* END - TESTING */

		// Also do dlights. Includes map glows and sabers and all that good stuff...
		if (u_lightCount > 0.0)
		{
#define LIGHT_COLOR_POWER			4.0

			vec3 addedLight = vec3(0.0);
			float power = clamp(length(color.rgb) / 3.0, 0.0, 1.0) * 0.5 + 0.5;
			power = pow(power, LIGHT_COLOR_POWER);
			power = power * 0.5 + 0.5;

			float maxStr = max(color.r, max(color.g, color.b)) * 0.9 + 0.1;

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightPos = u_lightPositions2[li].xyz;

				float lightDist = distance(lightPos, waterMapLower3.xzy/*position.xyz*/);

				if (u_lightHeightScales[li] > 0.0)
				{// ignore height differences, check later...
					lightDist -= length(lightPos.z - waterMapLower3.y/*position.z*/);
				}

				float lightDistMult = 1.0 - clamp((distance(lightPos.xyz, u_ViewOrigin.xyz) / 4096.0), 0.0, 1.0);
				float lightStrength = pow(1.0 - clamp(lightDist / u_lightDistances[li], 0.0, 1.0), 2.0);
				lightStrength *= lightDistMult;

				if (lightStrength > 0.0)
				{
					vec3 lightColor = (u_lightColors[li].rgb / length(u_lightColors[li].rgb)) * 4.0;//1.5; // Normalize.
					vec3 lightDir2 = normalize(lightPos - waterMapLower3.xzy/*position.xyz*/);
				
					lightColor = lightColor * lightStrength * power;

					lightColor *= maxStr;
					addedLight.rgb += (lightColor * vec3(getspecular(-normal, -lightDir2, eyeVecNorm, 60.0)));
				}
			}

			color.rgb = color.rgb + addedLight;
		}
#else
		color = mix(refraction, waterColorDeep, fresnel);
#endif

#if defined(USE_REFLECTION)
		if (!pixelIsUnderWater && u_Local1.g >= 2.0)
		{
			color = AddReflection(var_TexCoords, position, vec3(waterMapLower3.x, level, waterMapLower3.z), color.rgb);
		}
#endif //defined(USE_REFLECTION)

		color = clamp(color + max(specular, foam.rgb * sunColor), 0.0, 1.0);
		
		color = mix(refraction, color, clamp(depth * shoreHardness, 0.0, 1.0));

		color = mix(color, color2, 1.0 - clamp(waterClarity * depth, 0.8, 1.0));

		//if (position2.y/*position.y*/ > level && waterMapLower.a <= 0.0)
		//if (position2.z > level && position2.a != 1024.0/*level > waterMapLower.y*/)
		//if (position.y > level)// && position2.a != 1024.0)
		//{// Waves against shoreline. Pixel is above waterLevel + waveHeight... (but ignore anything marked as actual water - eg: not a shoreline)
		//	color = color2;
		//}

		if (!pixelIsUnderWater)
		{
			color.rgb = mix(color2.rgb, color.rgb, heightAboveGround);
			
			if (clamp(1.0 - (heightAboveGround * 0.65), 0.0, 1.0) > 0.5)
				color.rgb = color2.rgb;
		}
	}
#endif //__TEST_WATER__

	gl_FragColor = vec4(color, 1.0);
}

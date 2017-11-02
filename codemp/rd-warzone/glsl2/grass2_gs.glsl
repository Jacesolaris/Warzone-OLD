#define MAX_FOLIAGES				85

//#define THREE_WAY_GRASS_CLUMPS // otherwise uses 2 way X shape... 2 way probably gives better coverage...

#define GRASSMAP_SCONTROL_MIN		0.2

#if !defined(USE_400)
#extension GL_ARB_gpu_shader5 : enable
layout(triangles) in;
#else
layout(triangles, invocations = 8) in;
#endif
layout(triangle_strip, max_vertices = MAX_FOLIAGES) out;


uniform mat4						u_ModelViewProjectionMatrix;

uniform sampler2D					u_SplatControlMap;
uniform sampler2D					u_RoadsControlMap;

uniform vec4						u_Local1; // MAP_SIZE, sway, overlaySway, materialType
uniform vec4						u_Local2; // hasSteepMap, hasWaterEdgeMap, haveNormalMap, SHADER_WATER_LEVEL
uniform vec4						u_Local3; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4						u_Local8; // passnum, GRASS_DISTANCE_FROM_ROADS, GRASS_HEIGHT, 0.0
uniform vec4						u_Local9; // testvalue0, 1, 2, 3
uniform vec4						u_Local10; // foliageLODdistance, 0.0, 0.0, GRASS_TYPE_UNIFORMALITY

#define SHADER_MAP_SIZE				u_Local1.r
#define SHADER_SWAY					u_Local1.g
#define SHADER_OVERLAY_SWAY			u_Local1.b
#define SHADER_MATERIAL_TYPE		u_Local1.a

#define SHADER_HAS_STEEPMAP			u_Local2.r
#define SHADER_HAS_WATEREDGEMAP		u_Local2.g
#define SHADER_HAS_NORMALMAP		u_Local2.b
#define SHADER_WATER_LEVEL			u_Local2.a

#define SHADER_HAS_SPLATMAP1		u_Local3.r
#define SHADER_HAS_SPLATMAP2		u_Local3.g
#define SHADER_HAS_SPLATMAP3		u_Local3.b
#define SHADER_HAS_SPLATMAP4		u_Local3.a

#define PASS_NUMBER					u_Local8.r
#define GRASS_DISTANCE_FROM_ROADS	u_Local8.g
#define GRASS_HEIGHT				u_Local8.b

#define MAX_RANGE					u_Local10.r
#define GRASS_TYPE_UNIFORMALITY		u_Local10.a

#define MAP_WATER_LEVEL				SHADER_WATER_LEVEL // TODO: Use water map
#define GRASS_TYPE_UNIFORM_WATER	0.66

uniform vec3						u_ViewOrigin;
uniform float						u_Time;

uniform vec4						u_MapInfo; // MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], 0.0
uniform vec4						u_Mins;
uniform vec4						u_Maxs;

flat in	int							isSlope[];

smooth out vec2						vTexCoord;
smooth out vec3						vVertPosition;
//flat out float						vVertNormal;
smooth out vec2						vVertNormal;
flat out int						iGrassType;

//
// General Settings...
//

const float							fWindStrength = 12.0;
const vec3							vWindDirection = normalize(vec3(1.0, 1.0, 0.0));

float								controlScale = 1.0 / SHADER_MAP_SIZE;

vec3 vLocalSeed;

const float xdec = 1.0/255.0;
const float ydec = 1.0/65025.0;
const float zdec = 1.0/16581375.0;

float EncodeFloatRGBA( vec4 rgba ) {
  return dot( rgba, vec4(1.0, xdec, ydec, zdec) );
}

// This function returns random number from zero to one
float randZeroOne()
{
	uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
	n = n * (n * n * 15731u + 789221u);
	n = (n >> 9u) | 0x3F800000u;

	float fRes = 2.0 - uintBitsToFloat(n);
	vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
	return fRes;
}

int randomInt(int min, int max)
{
	float fRandomFloat = randZeroOne();
	return int(float(min) + fRandomFloat*float(max - min));
}

// Produce a psuedo random point that exists on the current triangle primitive.
vec4 randomBarycentricCoordinate() {
	float R = randZeroOne();
	float S = randZeroOne();

	if (R + S >= 1) {
		R = 1 - R;
		S = 1 - S;
	}

	return gl_in[0].gl_Position + (R * (gl_in[1].gl_Position - gl_in[0].gl_Position)) + (S * (gl_in[2].gl_Position - gl_in[0].gl_Position));
}

const vec2 roadPx = const vec2(1.0 / 2048.0);

vec4 GetControlMap(vec3 m_vertPos)
{
	vec4 xaxis = texture(u_SplatControlMap, (m_vertPos.yz * controlScale) * 0.5 + 0.5);
	vec4 yaxis = texture(u_SplatControlMap, (m_vertPos.xz * controlScale) * 0.5 + 0.5);
	vec4 zaxis = texture(u_SplatControlMap, (m_vertPos.xy * controlScale) * 0.5 + 0.5);

	if (SHADER_HAS_SPLATMAP4 > 0.0)
	{// Also grab the roads map, if we have one...
		vec2 mapSize = u_Maxs.xy - u_Mins.xy;
		vec2 pixel = (m_vertPos.xy - u_Mins.xy) / mapSize;
		
		float road = texture(u_RoadsControlMap, pixel).r;

		if (road > GRASS_DISTANCE_FROM_ROADS)
		{
			return vec4(0.0); // Force no grass near roads, or on black parts of the road map (obstacles)...
		}
		else if (road > 0.0)
		{
			float scale = 1.0 - (road / GRASS_DISTANCE_FROM_ROADS);
			xaxis.a = yaxis.a = zaxis.a = scale;
		}
		else
		{
			xaxis.a = yaxis.a = zaxis.a = 1.0;
		}
	}

	return xaxis * 0.333 + yaxis * 0.333 + zaxis * 0.333;
}

vec4 GetGrassMap(vec3 m_vertPos)
{
	vec4 control = GetControlMap(m_vertPos);
	control.rgb = clamp(clamp(clamp(control.rgb * 1024.0, 0.0, 1.0) - 0.05, 0.0, 1.0) * 0.5, 0.0, 1.0);
	return control;
}

mat4 rotationMatrix(vec3 axis, float angle) 
{ 
    axis = normalize(axis); 
    float s = sin(angle); 
    float c = cos(angle); 
    float oc = 1.0 - c; 
     
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0, 
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0, 
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0, 
                0.0,                                0.0,                                0.0,                                1.0); 
}

const float PIover180 = 3.1415/180.0; 
const vec3 vBaseDir[] = const vec3[] (
	vec3(1.0, 0.0, 0.0),
#ifdef THREE_WAY_GRASS_CLUMPS
	vec3(float(cos(45.0*PIover180)), float(sin(45.0*PIover180)), 0.0f),
	vec3(float(cos(-45.0*PIover180)), float(sin(-45.0*PIover180)), 0.0f)
#else //!THREE_WAY_GRASS_CLUMPS
	vec3(float(cos(90.0*PIover180)), float(sin(90.0*PIover180)), 0.0f)
#endif //THREE_WAY_GRASS_CLUMPS
);

void main()
{
	#if defined(USE_400)
		// invocations support...
		if (gl_InvocationID >= PASS_NUMBER)
		{// Hit number of specified invocations, skip this pass...
			return;
		}
	#endif

	if (isSlope[0] > 0 || isSlope[1] > 0 || isSlope[2] > 0)
	{
		return; // This slope is too steep for grass...
	}

	//float iGrassType = 0;
	iGrassType = 0;

	//face center------------------------
	vec3 Vert1 = gl_in[0].gl_Position.xyz;
	vec3 Vert2 = gl_in[1].gl_Position.xyz;
	vec3 Vert3 = gl_in[2].gl_Position.xyz;

	vec3 Pos = (Vert1 + Vert2 + Vert3) / 3.0;   //Center of the triangle - copy for later
	//-----------------------------------

	//if (Pos.z < MAP_WATER_LEVEL - 512.0)
	//{// Below map's water level... Early cull... (Maybe underwater plants later???)
	//	return;
	//}

	// UQ1: Checked and distance is faster
	float VertDist = distance(u_ViewOrigin, Pos);//(u_ModelViewProjectionMatrix*vec4(Pos, 1.0)).z;

	if (VertDist >= MAX_RANGE + 1024 // Too far from viewer...
		|| (VertDist >= 1024.0 && Pos.z < MAP_WATER_LEVEL && u_ViewOrigin.z >= MAP_WATER_LEVEL) // Underwater and distant and player is not...
		|| (VertDist >= 1024.0 && Pos.z >= MAP_WATER_LEVEL && u_ViewOrigin.z < MAP_WATER_LEVEL)) // Above water and player is below...
	{// Early cull...
		return;
	}

	float falloffStart2 = (MAX_RANGE + 1024) / 1.5;

	if (VertDist >= falloffStart2)
	{
		float falloffEnd = (MAX_RANGE + 1024)-falloffStart2;
		float pDist = clamp((VertDist-falloffStart2) / falloffEnd, 0.0, 1.0);
		float vertDistanceScale2 = 1.0 - pDist; // Scale down to zero size by distance...
			
		if (vertDistanceScale2 <= 0.05)
		{
			return;
		}
	}

	//vec3 normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz)); //calculate normal for this face

	//face info--------------------------
	//float VertSize = length(Vert1-Vert2) + length(Vert1-Vert3) + length(Vert2-Vert3);
	//-----------------------------------

	#if !defined(USE_400)
		// No invocations support...
		vLocalSeed = Pos*PASS_NUMBER;
	#else
		// invocations support...
		vLocalSeed = Pos*float(gl_InvocationID);
	#endif

#ifdef THREE_WAY_GRASS_CLUMPS
	for (int x = 0; x < MAX_FOLIAGES / 3; x++)
#else //!THREE_WAY_GRASS_CLUMPS
	for (int x = 0; x < MAX_FOLIAGES / 2; x++)
#endif //THREE_WAY_GRASS_CLUMPS
	{
		vec3 vGrassFieldPos = randomBarycentricCoordinate().xyz;

		float VertDist2 = distance(u_ViewOrigin, vGrassFieldPos);

		if (VertDist2 >= MAX_RANGE)
		{// Too far from viewer... Cull...
			continue;
		}

		float vertDistanceScale = 1.0;
		float falloffStart = MAX_RANGE / 1.5;

		if (VertDist2 >= falloffStart)
		{
			float falloffEnd = MAX_RANGE-falloffStart;
			float pDist = clamp((VertDist2-falloffStart) / falloffEnd, 0.0, 1.0);
			vertDistanceScale = 1.0 - pDist; // Scale down to zero size by distance...
			
			if (vertDistanceScale <= 0.05)
			{
				continue;
			}
		}

		vec4 controlMap = GetGrassMap(vGrassFieldPos);

		if (controlMap.a <= 0.0)
		{
			continue;
		}

		float controlMapScale = length(controlMap.rgb);
		controlMapScale *= controlMapScale;
		controlMapScale += 0.1;

		if (controlMapScale * controlMap.a < 0.2)
		{// Check if this area is on the grass map. If not, there is no grass here...
			continue;
		}

		float heightAboveWater = vGrassFieldPos.z - MAP_WATER_LEVEL;
		float heightAboveWaterLength = length(heightAboveWater);
		
		if (heightAboveWaterLength <= 128.0)
		{// Too close to water edge...
			continue;
		}

		float fSizeRandomness = randZeroOne() * 0.25 + 0.75;
		float sizeMult = 1.25;
		float fGrassPatchHeight = 1.0;

		if (heightAboveWater < 0.0)
		{
			iGrassType = 16;
			
			if (randZeroOne() > GRASS_TYPE_UNIFORM_WATER)
			{// Randomize...
				iGrassType = randomInt(16, 19);
			}

			if (heightAboveWaterLength <= 192.0)
			{// When near water edge, reduce the size of the grass...
				sizeMult *= clamp(heightAboveWaterLength / 192.0, 0.0, 1.0) * fSizeRandomness;
			}
			else
			{// Deep underwater plants draw larger...
				sizeMult *= clamp(1.0 + (heightAboveWaterLength / 192.0), 1.0, 16.0);
			}

			fGrassPatchHeight = clamp(controlMapScale * vertDistanceScale * 5.0, 0.0, 1.5) * fSizeRandomness;
		}
		else
		{
			if (controlMap.r >= GRASSMAP_SCONTROL_MIN && controlMap.g >= GRASSMAP_SCONTROL_MIN && controlMap.b >= GRASSMAP_SCONTROL_MIN)
			{// Any main grass...
				iGrassType = randomInt(0, 2);
			}
			else if (controlMap.r >= GRASSMAP_SCONTROL_MIN && controlMap.g >= GRASSMAP_SCONTROL_MIN)
			{// Either r or g grass...
				iGrassType = randomInt(0, 1);
			}
			else if (controlMap.r >= GRASSMAP_SCONTROL_MIN && controlMap.b >= GRASSMAP_SCONTROL_MIN)
			{// Either r or b grass...
				iGrassType = randomInt(0, 1);
				iGrassType += iGrassType; // so 0 or 2
			}
			else if (controlMap.g >= GRASSMAP_SCONTROL_MIN && controlMap.b >= GRASSMAP_SCONTROL_MIN)
			{// Either g or b grass...
				iGrassType = randomInt(1, 2);
			}
			else if (controlMap.r >= GRASSMAP_SCONTROL_MIN)
			{// Always r grass... (some randomcy applied at end)
				iGrassType = 0;
			}
			else if (controlMap.g >= GRASSMAP_SCONTROL_MIN)
			{// Always g grass... (some randomcy applied at end)
				iGrassType = 1;
			}
			else if (controlMap.b >= GRASSMAP_SCONTROL_MIN)
			{// b grass map forces randomization...
				iGrassType = randomInt(0, 15);
			}
			else
			{// Any grass at all...
				iGrassType = randomInt(0, 15);
			}

			if (randZeroOne() > GRASS_TYPE_UNIFORMALITY)
			{// Randomize...
				iGrassType = randomInt(2, 15);
			}

			if (heightAboveWaterLength <= 256.0)
			{// When near water edge, reduce the size of the grass...
				sizeMult *= clamp(heightAboveWaterLength / 192.0, 0.0, 1.0) * fSizeRandomness;
			}

			fGrassPatchHeight = clamp(controlMapScale * vertDistanceScale * 3.0, 0.0, 1.0);
		}

		if (fGrassPatchHeight <= 0.05)
		{
			continue;
		}

		float fGrassFinalSize = GRASS_HEIGHT * sizeMult * fSizeRandomness * controlMap.a; // controlMap.a is road edges multiplier...

		if (fGrassFinalSize <= 0.05)
		{
			continue;
		}

		if (iGrassType > 2 && iGrassType < 16)
		{// Rare randomized grasses (3 -> 9 - the plants) are a bit larger then the standard grass...
			fGrassPatchHeight *= 1.25;
		}

		// Wind calculation stuff...
		float fWindPower = 0.5f + sin(vGrassFieldPos.x / 30 + vGrassFieldPos.z / 30 + u_Time*(1.2f + fWindStrength / 20.0f));

		if (fWindPower < 0.0f)
			fWindPower = fWindPower*0.2f;
		else
			fWindPower = fWindPower*0.3f;

		fWindPower *= fWindStrength;

		float randDir = sin(randZeroOne()*0.7f)*0.1f;

#ifdef THREE_WAY_GRASS_CLUMPS
		for(int i = 0; i < 3; i++)
#else //!THREE_WAY_GRASS_CLUMPS
		for(int i = 0; i < 2; i++)
#endif //THREE_WAY_GRASS_CLUMPS
		{// Draw either 2 or 3 copies at each position at different angles...
			vec3 direction = (rotationMatrix(vec3(0, 1, 0), randDir)*vec4(vBaseDir[i], 1.0)).xyz;

			vec3 P = vGrassFieldPos.xyz;

			vec3 va = P - (direction * fGrassFinalSize);
			vec3 vb = P + (direction * fGrassFinalSize);
			vec3 vc = va + vec3(0.0, 0.0, fGrassFinalSize * fGrassPatchHeight);
			vec3 vd = vb + vec3(0.0, 0.0, fGrassFinalSize * fGrassPatchHeight);

			//float encodedGrassData = EncodeFloatRGBA(vec4(normalize(cross(vc - va, vb - va)), float(iGrassType) / 20.0));

			vVertPosition = va.xyz;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vVertPosition, 1.0);
			vTexCoord = vec2(0.0, 1.0);
			vVertNormal = normalize(cross(vc - va, vb - va)).xy;//encodedGrassData;
			EmitVertex();

			vVertPosition = vb.xyz;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vVertPosition, 1.0);
			vTexCoord = vec2(1.0, 1.0);
			EmitVertex();
		
			vVertPosition = vc.xyz + vWindDirection*fWindPower;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vVertPosition, 1.0);
			vTexCoord = vec2(0.0, 0.0);
			EmitVertex();

			vVertPosition = vd.xyz + vWindDirection*fWindPower;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vVertPosition, 1.0);
			vTexCoord = vec2(1.0, 0.0);
			EmitVertex();

			EndPrimitive();
		}
	}
}
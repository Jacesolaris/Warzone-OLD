#if !defined(USE_400)
#extension GL_ARB_gpu_shader5 : enable
//layout(triangles) in;
//#else
//layout(triangles, invocations = 6) in;
#endif

#define MAX_FOLIAGES		102

layout(triangles) in;
layout(triangle_strip, max_vertices = MAX_FOLIAGES) out;

uniform mat4				u_ModelViewProjectionMatrix;
uniform mat4				u_ModelMatrix;
uniform mat4				u_ModelViewMatrix;
uniform mat4				u_NormalMatrix;

uniform sampler2D			u_SplatControlMap;

uniform vec4				u_Local6; // useSunLightSpecular, hasSteepMap2, MAP_SIZE, WATER_LEVEL // -- only MAP_SIZE is used here
uniform vec4				u_Local7; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4				u_Local8; // passnum, 0, 0, 0
uniform vec4				u_Local9; // testvalue0, 1, 2, 3
uniform vec4				u_Local10; // foliageLODdistance, foliageDensity, MAP_WATER_LEVEL, 0.0

uniform vec3				u_ViewOrigin;
uniform float				u_Time;

flat in	int					isSlope[];

smooth out vec2				vTexCoord;
out vec3					vVertPosition;
flat out int				iGrassType;


#define GRASSMAP_MIN_TYPE_VALUE 0.2
#define SECONDARY_RANDOM_CHANCE 0.7

#define MAP_WATER_LEVEL			u_Local10.b // TODO: Use water map
#define PASS_NUMBER				u_Local8.r

//
// General Settings...
//

const float					fGrassPatchSize = 256.0;//u_Local9.b;//64.0;//48.0;//24.0;
const float					fWindStrength = 12.0;
const vec3					vWindDirection = normalize(vec3(1.0, 0.0, 1.0));

//#define					foliageDensity u_Local10.g
const float					foliageDensity = 40.0; // Changed to constant for more speed.

float						controlScale = 1.0 / u_Local6.b;

//
// LOD Range Settings...
//

#define MAX_RANGE			u_Local10.r

vec3 vLocalSeed;

// This function returns random number from zero to one
float randZeroOne()
{
    uint n = floatBitsToUint(vLocalSeed.y * 214013.0 + vLocalSeed.x * 2531011.0 + vLocalSeed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    vLocalSeed = vec3(vLocalSeed.x + 147158.0 * fRes, vLocalSeed.y*fRes  + 415161.0 * fRes, vLocalSeed.z + 324154.0*fRes);
    return fRes;
}

int randomInt(int min, int max)
{
	float fRandomFloat = randZeroOne();
	return int(float(min)+fRandomFloat*float(max-min));
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

vec4 GetControlMap( vec3 m_vertPos)
{
	vec4 xaxis = texture2D( u_SplatControlMap, (m_vertPos.yz * controlScale) * 0.5 + 0.5);
	vec4 yaxis = texture2D( u_SplatControlMap, (m_vertPos.xz * controlScale) * 0.5 + 0.5);
	vec4 zaxis = texture2D( u_SplatControlMap, (m_vertPos.xy * controlScale) * 0.5 + 0.5);

	return xaxis * 0.333 + yaxis * 0.333 + zaxis * 0.333;
}

vec4 GetGrassMap(vec3 m_vertPos)
{
	vec4 control = GetControlMap(m_vertPos);
	return clamp(pow(control, vec4(0.3)) * 0.5, 0.0, 1.0);
}

void main()
{
	if (isSlope[0] > 0 || isSlope[1] > 0 || isSlope[2] > 0)
	{
		return; // This slope is too steep for grass...
	}

	iGrassType = 0;

	//face center------------------------
    vec3 Vert1 = gl_in[0].gl_Position.xyz;
    vec3 Vert2 = gl_in[1].gl_Position.xyz;
    vec3 Vert3 = gl_in[2].gl_Position.xyz;

    vec3 Pos = (Vert1+Vert2+Vert3) / 3.0;   //Center of the triangle - copy for later
    //-----------------------------------

	//if (Pos.z < MAP_WATER_LEVEL - 512.0)
	//{// Below map's water level... Early cull... (Maybe underwater plants later???)
	//	return;
	//}

	// UQ1: Checked and distance is faster
	float VertDist = distance(u_ViewOrigin, Pos);//(u_ModelViewProjectionMatrix*vec4(Pos, 1.0)).z;

	if (VertDist >= MAX_RANGE + 1024) 
	{// Too far from viewer... Early cull...
		return;
	}
	
	vec3 normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz)); //calculate normal for this face

	//face info--------------------------
	//float VertSize = length(Vert1-Vert2) + length(Vert1-Vert3) + length(Vert2-Vert3);
	//int densityMax = int(VertSize / foliageDensity);
    //-----------------------------------

//#if !defined(USE_400)
	// No invocations support...
	vLocalSeed = Pos*PASS_NUMBER;
//#else
//	// invocations support...
//	vLocalSeed = Pos*float(gl_InvocationID);
//#endif
	
	float m = 1.0 - (VertDist / MAX_RANGE);
	int FOLIAGE_DENSITY = int(pow(float(MAX_FOLIAGES), m));

	if (Pos.z <= MAP_WATER_LEVEL - 256.0)
	{// Deep underwater plants draw at lower density, but larger...
		float densityMult = 3.0 + clamp(((MAP_WATER_LEVEL - 256.0) - Pos.z) / 64.0, 0.0, 6.0);
		FOLIAGE_DENSITY = int(float(FOLIAGE_DENSITY) / densityMult);
		if (FOLIAGE_DENSITY < 2) FOLIAGE_DENSITY = 2;
	}

	for(int x = 0; x < FOLIAGE_DENSITY; x++)
	{
		vec3 vGrassFieldPos = randomBarycentricCoordinate().xyz;

		vec4 controlMap = GetGrassMap(vGrassFieldPos);
		float controlMapScale = length(controlMap.rgb);

		if (controlMapScale < 0.2)//u_Local9.a)
		{// Check if this area is on the grass map. If not, there is no grass here...
			continue;
		}

		//if (controlMap.a > 0.0) // Testing large plants patches when controlMap alpha channel is active...
		//	controlMapScale += (controlMapScale * controlMap.a); // long grass patch

		// Fill in the smaller size grass around edges (since we just removed the smallest ones)...
		controlMapScale *= controlMapScale;
		controlMapScale += 0.1;

		float fGrassPatchWaterEdgeMod = randZeroOne();

		if (vGrassFieldPos.z < MAP_WATER_LEVEL + 64.0 + (fGrassPatchWaterEdgeMod * 96.0))
		{
			if (vGrassFieldPos.z < MAP_WATER_LEVEL - (64.0 + (fGrassPatchWaterEdgeMod * 96.0)) && vGrassFieldPos.z > MAP_WATER_LEVEL - 256.0)
				iGrassType = 3;
			else if (vGrassFieldPos.z <= MAP_WATER_LEVEL - 256.0)
				iGrassType = 4;
			else
				continue;
		}
		else
		{
			if (controlMap.r >= GRASSMAP_MIN_TYPE_VALUE && controlMap.g >= GRASSMAP_MIN_TYPE_VALUE && controlMap.b >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = randomInt(0, 2);
			}
			else if (controlMap.r >= GRASSMAP_MIN_TYPE_VALUE && controlMap.g >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = randomInt(0, 1);
				if (randZeroOne() > SECONDARY_RANDOM_CHANCE) iGrassType = 2; // Mix in occasional second random selection...
			}
			else if (controlMap.r >= GRASSMAP_MIN_TYPE_VALUE && controlMap.b >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = randomInt(0, 1);
				if (iGrassType == 1) iGrassType = 2;
				if (randZeroOne() > SECONDARY_RANDOM_CHANCE) iGrassType = 1; // Mix in occasional second random selection...
			}
			else if (controlMap.g >= GRASSMAP_MIN_TYPE_VALUE && controlMap.b >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = randomInt(1, 2);
				if (randZeroOne() > SECONDARY_RANDOM_CHANCE) iGrassType = 0; // Mix in occasional second random selection...
			}
			else if (controlMap.r >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = 0;
				if (randZeroOne() > SECONDARY_RANDOM_CHANCE) iGrassType = randomInt(1, 2);
			}
			else if (controlMap.g >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = 1;
				if (randZeroOne() > SECONDARY_RANDOM_CHANCE)  // Mix in occasional second random selection...
				{ 
					iGrassType = randomInt(0, 1); 
					if (iGrassType == 1) iGrassType = 2; 
				}
			}
			else if (controlMap.b >= GRASSMAP_MIN_TYPE_VALUE)
			{
				iGrassType = 2;
				if (randZeroOne() > SECONDARY_RANDOM_CHANCE) iGrassType = randomInt(0, 1); // Mix in occasional second random selection...
			}
		}

		// UQ1: Checked and distance is faster
		float VertDist2 = distance(u_ViewOrigin, vGrassFieldPos);//(u_ModelViewProjectionMatrix*vec4(vGrassFieldPos, 1.0)).z;

		if (VertDist2 >= MAX_RANGE) 
		{// Too far from viewer... Cull...
			continue;
		}

		float heightMult = 1.0;

		if (iGrassType < 3 && vGrassFieldPos.z < MAP_WATER_LEVEL + 160.0)
		{// When near water edge, reduce the size of the grass...
			heightMult = fGrassPatchWaterEdgeMod * 0.5 + 0.5;
		}
		else if (iGrassType >= 4 && vGrassFieldPos.z <= MAP_WATER_LEVEL - 256.0)
		{// Deep underwater plants draw larger but less of them...
			float heightMultMult = 1.0 + clamp(((MAP_WATER_LEVEL - 256.0) - vGrassFieldPos.z) / 128.0, 0.0, 4.0);
			heightMult = fGrassPatchWaterEdgeMod * heightMultMult;
		}
		else if (iGrassType >= 3 && vGrassFieldPos.z > MAP_WATER_LEVEL - 160.0)
		{// When near water edge, reduce the size of the grass...
			heightMult = fGrassPatchWaterEdgeMod * 0.5 + 0.5;
		}

		heightMult *= controlMapScale;

		//if (PASS_NUMBER > 6.0)
		//	heightMult *= 0.5;

		float fGrassPatchHeight = (fGrassPatchWaterEdgeMod * 0.25 + 0.75) * heightMult; // use fGrassPatchWaterEdgeMod random to save doing an extra random

		// Wind calculation stuff...
		float fWindPower = 0.5f+sin(vGrassFieldPos.x/30+vGrassFieldPos.z/30+u_Time*(1.2f+fWindStrength/20.0f));
		
		if(fWindPower < 0.0f)
			fWindPower = fWindPower*0.2f;
		else 
			fWindPower = fWindPower*0.3f;
		
		fWindPower *= fWindStrength;
		
		vec3 right = vec3(randZeroOne(), randZeroOne(), 0.0);
		vec3 up = vec3(0.0, 0.0, 0.5);
		vec3 normalOffset = (normal * vec3(right.x, right.y, 0.5));

		float size = fGrassPatchSize*fGrassPatchHeight;

		vec3 P = vGrassFieldPos.xyz + (up * (size*0.9));

		vec3 va = P - (right + normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
		vTexCoord = vec2(0.0, 1.0);
		vVertPosition = va.xyz;
		EmitVertex();  
  
		vec3 vb = P - (right - normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(0.0, 0.0);
		vVertPosition = vb.xyz;
		EmitVertex();  
 
		vec3 vd = P + (right - normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
		vTexCoord = vec2(1.0, 1.0);
		vVertPosition = vd.xyz;
		EmitVertex();  
 
		vec3 vc = P + (right + normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(1.0, 0.0);
		vVertPosition = vc.xyz;
		EmitVertex();  
  
		EndPrimitive();
	}
}


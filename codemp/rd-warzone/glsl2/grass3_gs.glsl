layout(triangles, invocations = 20) in;
layout(triangle_strip, max_vertices = 170) out;


uniform mat4			u_ModelViewProjectionMatrix;
uniform mat4			u_ModelMatrix;
uniform mat4			u_ModelViewMatrix;

uniform vec4			u_Local10; // foliageLODdistance

uniform float			u_Time;

#define screenScale		vec3(r_FBufScale.xy, 0.0)

#define MAX_RANGE		u_Local10.r

#define LOD0_RANGE		MAX_RANGE / 8.0
#define LOD1_RANGE		MAX_RANGE / 5.0
#define LOD2_RANGE		MAX_RANGE / 3.0

#define LOD0_MAX_FOLIAGES 42
#define LOD1_MAX_FOLIAGES 9
#define LOD2_MAX_FOLIAGES 2

smooth out vec2 vTexCoord;
//smooth out vec3 vWorldPos;
//smooth out vec4 vEyeSpacePos;


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


void main()
{
	float fGrassPatchSize = 96.0;

	//face center------------------------
    vec3 Vert1 = gl_in[0].gl_Position.xyz;
    vec3 Vert2 = gl_in[1].gl_Position.xyz;
    vec3 Vert3 = gl_in[2].gl_Position.xyz;

    vec3 Pos = (Vert1+Vert2+Vert3) / 3.0;   //Center of the triangle - copy for later
    //-----------------------------------

	float VertDist = (u_ModelViewProjectionMatrix*vec4(Pos, 1.0)).z;

	//------ LOD - # of grass objects to spawn per-face
    int FOLIAGE_DENSITY = (VertDist <= LOD0_RANGE)? LOD0_MAX_FOLIAGES: LOD1_MAX_FOLIAGES;
    FOLIAGE_DENSITY = (VertDist >= LOD1_RANGE)? LOD2_MAX_FOLIAGES: FOLIAGE_DENSITY;
    FOLIAGE_DENSITY = (VertDist >= LOD2_RANGE)? 0: FOLIAGE_DENSITY;

	vLocalSeed = Pos*float(gl_InvocationID);

	vec3 vBaseDir[4];
	vBaseDir[0] = vec3(0.0, 0.0, 1.0);
	vBaseDir[1] = vec3(0.0, 0.0, 1.0);
	vBaseDir[2] = vec3(0.0, 0.0, -1.0);
	vBaseDir[3] = vec3(0.0, 0.0, -1.0);

	//vec3 vBaseDirRotated[4];
	//vBaseDirRotated[0] = (rotationMatrix(vec3(0, 1, 0), sin(u_Time*0.7f)*0.1f)*vec4(vBaseDir[0], 1.0)).xyz;
	//vBaseDirRotated[1] = (rotationMatrix(vec3(0, 1, 0), sin(u_Time*0.7f)*0.1f)*vec4(vBaseDir[1], 1.0)).xyz;
	//vBaseDirRotated[2] = (rotationMatrix(vec3(0, 1, 0), sin(u_Time*0.7f)*0.1f)*vec4(vBaseDir[2], 1.0)).xyz;
	//vBaseDirRotated[2] = (rotationMatrix(vec3(0, 1, 0), sin(u_Time*0.7f)*0.1f)*vec4(vBaseDir[3], 1.0)).xyz;

	vec3 instanceRotAddition[4];
	instanceRotAddition[0] = vec3(0.0, 0.0, 0.0);
	instanceRotAddition[1] = vec3(1.0, 0.0, 0.0);
	instanceRotAddition[2] = vec3(1.0, 0.0, 0.0);
	instanceRotAddition[3] = vec3(0.0, 0.0, 0.0);

	//float fWindStrength = 4.0;
	
	//vec3 vWindDirection = normalize(vec3(1.0, 0.0, 1.0));

	for(int x = 0; x < FOLIAGE_DENSITY; x ++)
	{
		vec3 vGrassFieldPos = randomBarycentricCoordinate().xyz;
		vGrassFieldPos.xyz += vec3(10.0);
		float fGrassPatchHeight = randZeroOne() * 0.25 + 0.75;
		vec3 scaleMult = vec3(fGrassPatchSize*fGrassPatchHeight*0.5f) * (vec3(1.0) - screenScale);
		//int iGrassPatchType = randomInt(0, 3);

		/*
		// Wind calculation stuff...
		float fWindPower = 0.5f+sin(vGrassFieldPos.x/30+vGrassFieldPos.z/30+u_Time*(1.2f+fWindStrength/20.0f));
		
		if(fWindPower < 0.0f)
			fWindPower = fWindPower*0.2f;
		else 
			fWindPower = fWindPower*0.3f;
		
		fWindPower *= fWindStrength;
		*/

		for(int i = 0; i < 4; i++)
		{
			vec3 vGrassInstancePos = vGrassFieldPos;
			vec3 rotation = (instanceRotAddition[i]*scaleMult);

#if 0
			vec3 vTL = vGrassInstancePos - vBaseDirRotated[i]*fGrassPatchSize*0.5f + vWindDirection*fWindPower;
#else
			
			vec3 vTL = vGrassInstancePos - (vBaseDir[i]*scaleMult);
			vTL -= rotation;
			vTL.y -= scaleMult.y*2.0;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vTL, 1.0);
			vTexCoord = vec2(0.0, 1.0);
			vTexCoord.y *= vBaseDir[i].z;
			//vWorldPos = vTL;
			//vEyeSpacePos = u_ModelMatrix*vec4(vTL, 1.0);
			EmitVertex();

			vec3 vBL = vGrassInstancePos - (vBaseDir[i]*scaleMult);
			vBL += rotation;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vBL, 1.0);
			vTexCoord = vec2(1.0, 1.0);
			vTexCoord.y *= vBaseDir[i].z;
			//vWorldPos = vBL;
			//vEyeSpacePos = u_ModelMatrix*vec4(vBL, 1.0);
			EmitVertex();

			vec3 vTR = vGrassInstancePos + (vBaseDir[i]*scaleMult);
			vTR -= rotation;
			vTR.y -= scaleMult.y*2.0;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vTR, 1.0);
			vTexCoord = vec2(0.0, 0.0);
			vTexCoord.y *= vBaseDir[i].z;
			//vWorldPos = vTL;
			//vEyeSpacePos = u_ModelMatrix*vec4(vTR, 1.0);
			EmitVertex();

			vec3 vBR = vGrassInstancePos + (vBaseDir[i]*scaleMult);
			vBL += rotation;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vBR, 1.0);
			vTexCoord = vec2(1.0, 0.0);
			vTexCoord.y *= vBaseDir[i].z;
			//vWorldPos = vBR;
			//vEyeSpacePos = u_ModelMatrix*vec4(vBR, 1.0);
			EmitVertex();

			EndPrimitive();
#endif
		}		
	}
}


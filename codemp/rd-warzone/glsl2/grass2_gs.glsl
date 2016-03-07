#if !defined(USE_400)
#extension GL_ARB_gpu_shader5 : enable
#endif

layout(triangles, invocations = 24) in;
layout(triangle_strip, max_vertices = 113) out;


uniform mat4			u_ModelViewProjectionMatrix;
uniform mat4			u_ModelMatrix;
uniform mat4			u_ModelViewMatrix;
uniform mat4			u_NormalMatrix;

uniform vec4			u_Local9;
uniform vec4			u_Local10; // foliageLODdistance, foliageDensity, doSway, overlaySway

uniform float			u_Time;

varying vec3			var_Normal;


smooth out vec2			vTexCoord;

#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
smooth out vec3			vWorldPos;
#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 



#define screenScale		vec3(r_FBufScale.xy, 0.0)

#define MAX_RANGE		u_Local10.r

#define LOD0_RANGE		MAX_RANGE / 16.0
#define LOD1_RANGE		MAX_RANGE / 8.0
#define LOD2_RANGE		MAX_RANGE / 5.0
#define LOD3_RANGE		MAX_RANGE / 3.0

#define LOD0_MAX_FOLIAGES 28
#define LOD1_MAX_FOLIAGES 20
#define LOD2_MAX_FOLIAGES 5
#define LOD3_MAX_FOLIAGES 1

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

#define M_PI		3.14159265358979323846

vec3 vectoangles( in vec3 value1 ) {
	float	forward;
	float	yaw, pitch;
	vec3	angles;

	if ( value1.g == 0 && value1.r == 0 ) {
		yaw = 0;
		if ( value1.b > 0 ) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		if ( value1.r > 0 ) {
			yaw = ( atan ( value1.g, value1.r ) * 180 / M_PI );
		}
		else if ( value1.g > 0 ) {
			yaw = 90;
		}
		else {
			yaw = 270;
		}
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = sqrt ( value1.r*value1.r + value1.g*value1.g );
		pitch = ( atan(value1.b, forward) * 180 / M_PI );
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	angles.r = -pitch;
	angles.g = yaw;
	angles.b = 0.0;

	return angles;
}

void main()
{
	//vec3 normal = gl_NormalMatrix * normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz)); //calculate normal for this face
	//vec3 normal = (u_ModelViewProjectionMatrix * vec4(normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz)), 0.0)).xyz; //calculate normal for this face
	vec3 normal = (u_NormalMatrix * vec4(normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz)), 0.0)).xyz; //calculate normal for this face
	float pitch = vectoangles( normal.xyz /*var_Normal.xyz*/ ).r;
	
	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	if (pitch > 26.0/*u_Local9.r*/ || pitch < -26.0/*-u_Local9.r*/) // 26.0 to 32.0 looks about right
		return; // This slope is too great for grass...

	float fGrassPatchSize = 96.0;

	//face center------------------------
    vec3 Vert1 = gl_in[0].gl_Position.xyz;
    vec3 Vert2 = gl_in[1].gl_Position.xyz;
    vec3 Vert3 = gl_in[2].gl_Position.xyz;

    vec3 Pos = (Vert1+Vert2+Vert3) / 3.0;   //Center of the triangle - copy for later
	float VertSize = length(Vert1-Vert2) + length(Vert1-Vert3) + length(Vert2-Vert3);
	int densityMax = int(VertSize / u_Local10.g);
    //-----------------------------------

	float VertDist = (u_ModelViewProjectionMatrix*vec4(Pos, 1.0)).z;

	//------ LOD - # of grass objects to spawn per-face
    int FOLIAGE_DENSITY;
	FOLIAGE_DENSITY = (VertDist <= LOD0_RANGE)? LOD0_MAX_FOLIAGES: LOD1_MAX_FOLIAGES;
    FOLIAGE_DENSITY = (VertDist >= LOD1_RANGE)? LOD2_MAX_FOLIAGES: FOLIAGE_DENSITY;
	FOLIAGE_DENSITY = (VertDist >= LOD2_RANGE)? LOD3_MAX_FOLIAGES: FOLIAGE_DENSITY;
    FOLIAGE_DENSITY = (VertDist >= LOD3_RANGE)? 0: FOLIAGE_DENSITY;

	if ( FOLIAGE_DENSITY > densityMax) FOLIAGE_DENSITY = densityMax;

#if !defined(USE_400)
	vLocalSeed = Pos;//*float(gl_PrimitiveIDIn/*gl_InvocationID*/);
#else
	vLocalSeed = Pos*float(gl_InvocationID);
#endif

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
			#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
				vWorldPos = vTL;
			#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
			EmitVertex();

			vec3 vBL = vGrassInstancePos - (vBaseDir[i]*scaleMult);
			vBL += rotation;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vBL, 1.0);
			vTexCoord = vec2(1.0, 1.0);
			vTexCoord.y *= vBaseDir[i].z;
			#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
				vWorldPos = vBL;
			#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
			EmitVertex();

			vec3 vTR = vGrassInstancePos + (vBaseDir[i]*scaleMult);
			vTR -= rotation;
			vTR.y -= scaleMult.y*2.0;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vTR, 1.0);
			vTexCoord = vec2(0.0, 0.0);
			vTexCoord.y *= vBaseDir[i].z;
			#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
				vWorldPos = vTR;
			#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
			EmitVertex();

			vec3 vBR = vGrassInstancePos + (vBaseDir[i]*scaleMult);
			vBL += rotation;
			gl_Position = u_ModelViewProjectionMatrix*vec4(vBR, 1.0);
			vTexCoord = vec2(1.0, 0.0);
			vTexCoord.y *= vBaseDir[i].z;
			#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
				vWorldPos = vBL;
			#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
			EmitVertex();

			EndPrimitive();
#endif
		}		
	}
}


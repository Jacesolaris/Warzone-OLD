#if !defined(USE_400)
#extension GL_ARB_gpu_shader5 : enable
#endif

layout(triangles) in;
layout(triangle_strip, max_vertices = 170) out;


uniform mat4				u_ModelViewProjectionMatrix;
uniform mat4				u_ModelMatrix;
uniform mat4				u_ModelViewMatrix;
uniform mat4				u_NormalMatrix;

uniform vec4				u_Local8; // passnum, 0, 0, 0
uniform vec4				u_Local9;
uniform vec4				u_Local10; // foliageLODdistance, foliageDensity, MAP_WATER_LEVEL, 0.0

uniform float				u_Time;

smooth out vec2				vTexCoord;


#define M_PI				3.14159265358979323846
#define MAP_WATER_LEVEL		u_Local10.b

//
// General Settings...
//

#define						FAST_METHOD
//#define					ANGLE_BASED_DENSITY

const float					fGrassPatchSize = 48.0;//24.0;
const float					fWindStrength = 12.0;
const vec3					vWindDirection = normalize(vec3(1.0, 0.0, 1.0));

//
// LOD Range Settings...
//

#define MAX_RANGE			u_Local10.r

#define LOD0_RANGE			MAX_RANGE / 16.0
#define LOD1_RANGE			MAX_RANGE / 8.0
#define LOD2_RANGE			MAX_RANGE / 5.0
#define LOD3_RANGE			MAX_RANGE / 3.0


#if !defined(FAST_METHOD)
#define LOD0_MAX_FOLIAGES	85
#define LOD1_MAX_FOLIAGES	32
#define LOD2_MAX_FOLIAGES	4
#define LOD3_MAX_FOLIAGES	1
#else //defined(FAST_METHOD)
#define LOD0_MAX_FOLIAGES	170
#define LOD1_MAX_FOLIAGES	85//48
#define LOD2_MAX_FOLIAGES	32
#define LOD3_MAX_FOLIAGES	16
#endif //defined(FAST_METHOD)

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
	//face center------------------------
    vec3 Vert1 = gl_in[0].gl_Position.xyz;
    vec3 Vert2 = gl_in[1].gl_Position.xyz;
    vec3 Vert3 = gl_in[2].gl_Position.xyz;

    vec3 Pos = (Vert1+Vert2+Vert3) / 3.0;   //Center of the triangle - copy for later
    //-----------------------------------

	if (Pos.z < MAP_WATER_LEVEL)
	{// Below map's water level... Early cull... (Maybe underwater plants later???)
		return;
	}

	float VertDist = (u_ModelViewProjectionMatrix*vec4(Pos, 1.0)).z;

	if (VertDist >= MAX_RANGE) 
	{// Too far from viewer... Early cull...
		return;
	}

	vec3 normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz)); //calculate normal for this face
	float pitch = vectoangles( normal.xyz ).r;
	
	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	if (pitch < 0.0) pitch = -pitch;

	if (pitch > 46.0)
	{
		return; // This slope is too steep for grass...
	}

	//face info--------------------------
	float VertSize = length(Vert1-Vert2) + length(Vert1-Vert3) + length(Vert2-Vert3);
	int densityMax = int(VertSize / u_Local10.g);
    //-----------------------------------

	//------ LOD - # of grass objects to spawn per-face
    int FOLIAGE_DENSITY = LOD0_MAX_FOLIAGES;
	if (VertDist >= LOD3_RANGE) FOLIAGE_DENSITY = LOD3_MAX_FOLIAGES;
	else if (VertDist >= LOD2_RANGE) FOLIAGE_DENSITY = LOD2_MAX_FOLIAGES;
	else if (VertDist >= LOD1_RANGE) FOLIAGE_DENSITY = LOD1_MAX_FOLIAGES;

#if defined(ANGLE_BASED_DENSITY)
	FOLIAGE_DENSITY = int(float(FOLIAGE_DENSITY) / (pitch+1.0));
#endif //defined(ANGLE_BASED_DENSITY)

	if ( FOLIAGE_DENSITY > densityMax) FOLIAGE_DENSITY = densityMax;

	// No invocations support...
	vLocalSeed = Pos*u_Local8.r;

	for(int x = 0; x < FOLIAGE_DENSITY; x++)
	{
		vec3 vGrassFieldPos = randomBarycentricCoordinate().xyz;

		if (vGrassFieldPos.z < MAP_WATER_LEVEL)
		{
			continue;
		}

		float fGrassPatchHeight = randZeroOne() * 0.25 + 0.75;

		// Wind calculation stuff...
		float fWindPower = 0.5f+sin(vGrassFieldPos.x/30+vGrassFieldPos.z/30+u_Time*(1.2f+fWindStrength/20.0f));
		
		if(fWindPower < 0.0f)
			fWindPower = fWindPower*0.2f;
		else 
			fWindPower = fWindPower*0.3f;
		
		fWindPower *= fWindStrength;
		
#if !defined(FAST_METHOD)
		if (VertDist >= MAX_RANGE / 32.0)
		{// Distant stuff is a single billboard facing the player...
			vec3 right = vec3(u_ModelViewMatrix[0][0], 
                    u_ModelViewMatrix[1][0], 
                    u_ModelViewMatrix[2][0]);
 
			vec3 up = vec3(u_ModelViewMatrix[0][1], 
					u_ModelViewMatrix[1][1], 
					u_ModelViewMatrix[2][1]);

			float size = fGrassPatchSize*fGrassPatchHeight;
		  
			vec3 P = vGrassFieldPos.xyz + (up * (size*0.9));

			vec3 va = P - (right + up) * size;
			gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
			vTexCoord = vec2(0.0, 1.0);
			EmitVertex();  
  
			vec3 vb = P - (right - up) * size;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
			vTexCoord = vec2(0.0, 0.0);
			EmitVertex();  
 
			vec3 vd = P + (right - up) * size;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
			vTexCoord = vec2(1.0, 1.0);
			EmitVertex();  
 
			vec3 vc = P + (right + up) * size;
			gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
			vTexCoord = vec2(1.0, 0.0);
			EmitVertex();  
  
			EndPrimitive();
		}
		else
		{// Close stuff draws 2 boxes in an X pattern...
			{
				vec3 right = vec3(1.0, 0.0, 0.0);
				vec3 up = vec3(0.0, 0.0, 1.0);

				float size = fGrassPatchSize*fGrassPatchHeight;
				vec3 P = vGrassFieldPos.xyz + (up * (size*0.9));

				vec3 va = P - (right + up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
				vTexCoord = vec2(0.0, 1.0);
				EmitVertex();  
  
				vec3 vb = P - (right - up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
				vTexCoord = vec2(0.0, 0.0);
				EmitVertex();  
 
				vec3 vd = P + (right - up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
				vTexCoord = vec2(1.0, 1.0);
				EmitVertex();  
 
				vec3 vc = P + (right + up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
				vTexCoord = vec2(1.0, 0.0);
				EmitVertex();  
  
				EndPrimitive();
			}
		
			{
				vec3 right = vec3(0.0, 1.0, 0.0);
				vec3 up = vec3(0.0, 0.0, 1.0);

				float size = fGrassPatchSize*fGrassPatchHeight;
				vec3 P = vGrassFieldPos.xyz + (up * (size*0.8));

				vec3 va = P - (right + up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
				vTexCoord = vec2(0.0, 1.0);
				EmitVertex();  
  
				vec3 vb = P - (right - up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
				vTexCoord = vec2(0.0, 0.0);
				EmitVertex();  
 
				vec3 vd = P + (right - up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
				vTexCoord = vec2(1.0, 1.0);
				EmitVertex();  
 
				vec3 vc = P + (right + up) * size;
				gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
				vTexCoord = vec2(1.0, 0.0);
				EmitVertex();  
  
				EndPrimitive();
			}
		}
#else //defined(FAST_METHOD)
		vec3 right = vec3(randZeroOne(), randZeroOne(), 0.0);
		vec3 up = vec3(0.0, 0.0, 0.5);
		vec3 normalOffset = (normal * vec3(right.x, right.y, 0.5));

		float size = fGrassPatchSize*fGrassPatchHeight;

		vec3 P = vGrassFieldPos.xyz + (up * (size*0.9));

		vec3 va = P - (right + normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
		vTexCoord = vec2(0.0, 1.0);
		EmitVertex();  
  
		vec3 vb = P - (right - normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(0.0, 0.0);
		EmitVertex();  
 
		vec3 vd = P + (right - normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
		vTexCoord = vec2(1.0, 1.0);
		EmitVertex();  
 
		vec3 vc = P + (right + normalOffset) * size;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(1.0, 0.0);
		EmitVertex();  
  
		EndPrimitive();
#endif //defined(FAST_METHOD)
	}
}


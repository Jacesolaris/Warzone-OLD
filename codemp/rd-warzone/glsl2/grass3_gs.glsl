#if !defined(USE_400)
#extension GL_ARB_gpu_shader5 : enable
//layout(triangles) in;
//#else
//layout(triangles, invocations = 6) in;
#endif

#define MAX_FOLIAGES			100
#define MAX_FOLIAGES_LOOP		64

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

const float					fGrassPatchSize = 128.0;//256.0;//u_Local9.b;//64.0;//48.0;//24.0;
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

#define M_PI		3.14159265358979323846

vec3 VectorMA( vec3 vec1, vec3 scale, vec3 vec2 ) 
{
	vec3 vecOut;
	vecOut.x = vec1.x + scale.x*vec2.x;
	vecOut.y = vec1.y + scale.y*vec2.y;
	vecOut.z = vec1.z + scale.z*vec2.z;
	return vecOut;
}

#define DEG2RAD( deg ) ( ((deg)*M_PI) / 180.0f )

vec3 VectorRotate( vec3 invec, mat3x3 matrix )
{
	vec3 outvec;
	outvec[0] = dot( invec, matrix[0] );
	outvec[1] = dot( invec, matrix[1] );
	outvec[2] = dot( invec, matrix[2] );
	return outvec;
}

vec3 RotatePointAroundVector( vec3 dir, vec3 point, float degrees )
{
	vec3 dst;
	mat3x3  m;
	float   c, s, t;

	degrees = DEG2RAD( degrees );
	s = sin( degrees );
	c = cos( degrees );
	t = 1 - c;

	m[0][0] = t*dir[0]*dir[0] + c;
	m[0][1] = t*dir[0]*dir[1] + s*dir[2];
	m[0][2] = t*dir[0]*dir[2] - s*dir[1];

	m[1][0] = t*dir[0]*dir[1] - s*dir[2];
	m[1][1] = t*dir[1]*dir[1] + c;
	m[1][2] = t*dir[1]*dir[2] + s*dir[0];

	m[2][0] = t*dir[0]*dir[2] + s*dir[1];
	m[2][1] = t*dir[1]*dir[2] - s*dir[0];
	m[2][2] = t*dir[2]*dir[2] + c;
	dst = VectorRotate( point, m );
	return dst;
}

vec3 rotate_point(vec3 pivotPoint, float angle, vec3 point)
{
	vec3 p = point;
	float s = sin(angle);
	float c = cos(angle);

	// translate point back to origin:
	p.x -= pivotPoint.x;
	p.y -= pivotPoint.y;

	// rotate point
	float xnew = p.x * c - p.y * s;
	float ynew = p.x * s + p.y * c;

	// translate point back:
	p.x = xnew + pivotPoint.x;
	p.y = ynew + pivotPoint.y;
	return p;
}

bool InstanceCloudReductionCulling(vec4 InstancePosition, vec3 ObjectExtent) 
{
   /* create the bounding box of the object */
   vec4 BoundingBox[8];
   BoundingBox[0] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4( ObjectExtent.x, ObjectExtent.y, ObjectExtent.z, 1.0) );
   BoundingBox[1] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4(-ObjectExtent.x, ObjectExtent.y, ObjectExtent.z, 1.0) );
   BoundingBox[2] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4( ObjectExtent.x,-ObjectExtent.y, ObjectExtent.z, 1.0) );
   BoundingBox[3] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4(-ObjectExtent.x,-ObjectExtent.y, ObjectExtent.z, 1.0) );
   BoundingBox[4] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4( ObjectExtent.x, ObjectExtent.y,-ObjectExtent.z, 1.0) );
   BoundingBox[5] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4(-ObjectExtent.x, ObjectExtent.y,-ObjectExtent.z, 1.0) );
   BoundingBox[6] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4( ObjectExtent.x,-ObjectExtent.y,-ObjectExtent.z, 1.0) );
   BoundingBox[7] = u_ModelViewProjectionMatrix * ( InstancePosition + vec4(-ObjectExtent.x,-ObjectExtent.y,-ObjectExtent.z, 1.0) );
   
   /* check how the bounding box resides regarding to the view frustum */   
   int outOfBound[6];// = int[6]( 0, 0, 0, 0, 0, 0 );

   for (int i=0; i<6; i++)
	   outOfBound[i] = 0;

   for (int i=0; i<8; i++)
   {
      if ( BoundingBox[i].x >  BoundingBox[i].w ) outOfBound[0]++;
      if ( BoundingBox[i].x < -BoundingBox[i].w ) outOfBound[1]++;
      if ( BoundingBox[i].y >  BoundingBox[i].w ) outOfBound[2]++;
      if ( BoundingBox[i].y < -BoundingBox[i].w ) outOfBound[3]++;
      if ( BoundingBox[i].z >  BoundingBox[i].w ) outOfBound[4]++;
      if ( BoundingBox[i].z < -BoundingBox[i].w ) outOfBound[5]++;
   }

   bool inFrustum = true;
   
   for (int i=0; i<6; i++)
   {
      if ( outOfBound[i] == 8 ) 
	  {
         inFrustum = false;
		 break;
      }
   }

   return !inFrustum;
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
	
	// Do some ICR culling on the base surfaces... Save us looping through extra surfaces...
	vec3 maxs;
	maxs = max(gl_in[0].gl_Position.xyz - Pos, gl_in[1].gl_Position.xyz - Pos);
	maxs = max(maxs, gl_in[2].gl_Position.xyz - Pos);

	if (InstanceCloudReductionCulling(vec4(Pos, 0.0), maxs))
	{
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
	
	float m = 1.0 - clamp((VertDist-1024.0) / MAX_RANGE, 0.0, 1.0);

	float USE_DENSITY = pow(m, 3.0/*8.0*/);//m*m*m;

	int numAddedVerts = 0;
	int maxAddedVerts = int(float(MAX_FOLIAGES)*USE_DENSITY);

	for(int x = 0; x < MAX_FOLIAGES_LOOP; x++)
	{
		if (float(numAddedVerts) >= maxAddedVerts)
		{
			break;
		}

		vec3 vGrassFieldPos = randomBarycentricCoordinate().xyz;

		if (vGrassFieldPos.z < MAP_WATER_LEVEL && float(numAddedVerts) >= float(maxAddedVerts) * 0.025)
		{
			break;
		}

		vec4 controlMap = GetGrassMap(vGrassFieldPos);
		float controlMapScale = length(controlMap.rgb);

		if (controlMapScale < 0.2)//u_Local9.a)
		{// Check if this area is on the grass map. If not, there is no grass here...
			continue;
		}

		float VertDist2 = distance(u_ViewOrigin, vGrassFieldPos);

		if (VertDist2 >= MAX_RANGE) 
		{// Too far from viewer... Cull...
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
		else
		{
			heightMult *= 1.25;
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
		
#if 1
		float size = fGrassPatchSize*fGrassPatchHeight;
		vec3 doublesize = vec3(size * 2.0, size * 2.0, size);

		vec3 direction = vec3(randZeroOne(), randZeroOne(), 0.0);
		vec3 up = vec3(0.0, 0.0, 1.0/*0.5*/);
		vec3 normalOffset = (normal * vec3(direction.x, direction.y, 1.0));

		vec3 P = vGrassFieldPos.xyz + (up * (size*0.45));

		vec3 va = P - ((direction + normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
		vTexCoord = vec2(0.0, 1.0);
		vVertPosition = va.xyz;
		EmitVertex();  
  
		vec3 vb = P - ((direction - normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(0.0, 0.0);
		vVertPosition = vb.xyz;
		EmitVertex();  
 
		vec3 vd = P + ((direction - normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
		vTexCoord = vec2(1.0, 1.0);
		vVertPosition = vd.xyz;
		EmitVertex();  
 
		vec3 vc = P + ((direction + normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(1.0, 0.0);
		vVertPosition = vc.xyz;
		EmitVertex();  
  
		EndPrimitive();

		numAddedVerts+=4;
#elif 1
		//vec3 rotate_point(vec3 pivotPoint, float angle, vec3 point)

		float size = fGrassPatchSize*fGrassPatchHeight;
		vec3 doublesize = vec3(size * 2.0, size * 2.0, size);

		vec3 direction = vec3(randZeroOne(), randZeroOne(), 0.0);
		vec3 up = vec3(0.0, 0.0, 1.0/*0.5*/);
		vec3 normalOffset = (normal * vec3(direction.x, direction.y, 1.0));

		vec3 P = vGrassFieldPos.xyz + (up * (size*0.45));

		vec3 va = P - ((direction + normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
		vTexCoord = vec2(0.0, 1.0);
		vVertPosition = va.xyz;
		EmitVertex();  
  
		vec3 vb = P - ((direction - normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(0.0, 0.0);
		vVertPosition = vb.xyz;
		EmitVertex();  
 
		vec3 vd = P + ((direction - normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
		vTexCoord = vec2(1.0, 1.0);
		vVertPosition = vd.xyz;
		EmitVertex();  
 
		vec3 vc = P + ((direction + normalOffset) * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(1.0, 0.0);
		vVertPosition = vc.xyz;
		EmitVertex();  
  
		EndPrimitive();

		numAddedVerts+=4;

		va = rotate_point(P, 3.1, P - ((direction + normalOffset) * doublesize));
		gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
		vTexCoord = vec2(0.0, 1.0);
		vVertPosition = va.xyz;
		EmitVertex();  
  
		vb = rotate_point(P, 3.1, P - ((direction - normalOffset) * doublesize));
		gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(0.0, 0.0);
		vVertPosition = vb.xyz;
		EmitVertex();  
 
		vd = rotate_point(P, 3.1, P + ((direction - normalOffset) * doublesize));
		gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
		vTexCoord = vec2(1.0, 1.0);
		vVertPosition = vd.xyz;
		EmitVertex();  
 
		vc = rotate_point(P, 3.1, P + ((direction + normalOffset) * doublesize));
		gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(1.0, 0.0);
		vVertPosition = vc.xyz;
		EmitVertex();  
  
		EndPrimitive();

		numAddedVerts+=4;
#else
		
		float size = fGrassPatchSize*fGrassPatchHeight;
		vec3 doublesize = vec3(size * 2.0, size * 2.0, size);

		vec3 direction = vec3(randZeroOne(), randZeroOne(), 0.0) * doublesize;
		vec3 up = vec3(0.0, 0.0, 1.0);

		vec3 P = vGrassFieldPos.xyz + (up * (size*0.9));
		vec3 P1 = VectorMA( P, direction, normal );
		vec3 P2 = VectorMA( P, direction, -normal );
		
		vec3 va = P2 - (up * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(va, 1.0);
		vTexCoord = vec2(0.0, 1.0);
		vVertPosition = va.xyz;
		EmitVertex();  
  
		vec3 vb = P2;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vb + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(0.0, 0.0);
		vVertPosition = vb.xyz;
		EmitVertex();  
 
		vec3 vd = P1 - (up * doublesize);
		gl_Position = u_ModelViewProjectionMatrix * vec4(vd, 1.0);
		vTexCoord = vec2(1.0, 1.0);
		vVertPosition = vd.xyz;
		EmitVertex();  
 
		vec3 vc = P1;
		gl_Position = u_ModelViewProjectionMatrix * vec4(vc + vWindDirection*fWindPower, 1.0);
		vTexCoord = vec2(1.0, 0.0);
		vVertPosition = vc.xyz;
		EmitVertex();  
  
		EndPrimitive();

		numAddedVerts+=4;
#endif
	}
}


#if 0

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_NormalMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec4		u_Local0;

varying vec2		var_ScreenTex;
varying vec2		var_Dimensions;
varying vec3		var_Position;

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



float random( const vec2 p )
{
    return randZeroOne();
}



mat2 randomRotation( const vec2 p )
{
    float r = random(p);
    float sinr = sin(r);
    float cosr = cos(r);
    return mat2(cosr, sinr, -sinr, cosr);
}



float getLinearDepth(sampler2D depthMap, const vec2 tex)
{
    float depth = texture(depthMap, tex).r;
    return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}



vec2 poissonDisc[9] = vec2[9](
vec2(-0.7055767, 0.196515),    vec2(0.3524343, -0.7791386),
vec2(0.2391056, 0.9189604),    vec2(-0.07580382, -0.09224417),
vec2(0.5784913, -0.002528916), vec2(0.192888, 0.4064181),
vec2(-0.6335801, -0.5247476),  vec2(-0.5579782, 0.7491854),
vec2(0.7320465, 0.6317794)
);
float ambientOcclusion(const vec2 tex)
{
    float result = 0;
    float zFar = u_ViewInfo.g;
    float sampleZ = zFar * getLinearDepth(u_ScreenDepthMap, tex);
    vec2 expectedSlope = vec2(dFdx(sampleZ), dFdy(sampleZ)) / vec2(dFdx(tex.x), dFdy(tex.y));
    //if (length(expectedSlope) > 5000.0)
	//	return 1.0;
	
	vec2 offsetScale = vec2(3.0 / sampleZ);
    mat2 rmat = randomRotation(tex);
    int i;
    for (i = 0; i < 3; i++)
	{
        vec2 offset = rmat * poissonDisc[i] * offsetScale;
        float sampleZ2 = zFar * getLinearDepth(u_ScreenDepthMap, tex + offset);
        //if (abs(sampleZ - sampleZ2) > 20.0)
		//	result += 1.0;
		//else
		{
            float expectedZ = sampleZ + dot(expectedSlope, offset);
            result += step(expectedZ - 1.0, sampleZ2);
        }


	}
	
	result *= 0.33333;
    //result *= 0.5;
	
	return result;
}

#endif //0





#if 0

#define USE_NORMALMAP

const float		total_strength = 1.0;
const float		base = 0.2;
const float		area = 0.0075;
/*const*/ float		falloff = u_Local0.r;//0.000001;
/*const*/ float		radius = u_Local0.g;//0.0002;
const int		samples = 16;

const vec3 sample_sphere[samples] = const vec3[samples](
      const vec3( 0.5381, 0.1856,-0.4319), 
	  const vec3( 0.1379, 0.2486, 0.4430),
      const vec3( 0.3371, 0.5679,-0.0057), 
	  const vec3(-0.6999,-0.0451,-0.0019),
      const vec3( 0.0689,-0.1598,-0.8547), 
	  const vec3( 0.0560, 0.0069,-0.1843),
      const vec3(-0.0146, 0.1402, 0.0762), 
	  const vec3( 0.0100,-0.1924,-0.0344),
      const vec3(-0.3577,-0.5301,-0.4358), 
	  const vec3(-0.3169, 0.1063, 0.0158),
      const vec3( 0.0103,-0.5869, 0.0046), 
	  const vec3(-0.0897,-0.4940, 0.3287),
      const vec3( 0.7119,-0.0154,-0.0918), 
	  const vec3(-0.0533, 0.0596,-0.5411),
      const vec3( 0.0352,-0.0631, 0.5460), 
	  const vec3(-0.4776, 0.2847,-0.0271)
);

//vec2 projAB = vec2( u_ViewInfo.g / (u_ViewInfo.g - u_ViewInfo.r), u_ViewInfo.g * u_ViewInfo.r / (u_ViewInfo.g - u_ViewInfo.r) );

float getLinearDepth(sampler2D depthMap, vec2 tex)
{
    float depth = texture(depthMap, tex).r;
    return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
	//float linearDepth = projAB.y / (depth - projAB.x);
	//return linearDepth;
	//return (u_ViewInfo.r * u_ViewInfo.g / (u_ViewInfo.g - depth * (u_ViewInfo.g - u_ViewInfo.r)) - u_ViewInfo.r);
}

vec2 offset1 = vec2(0.0, 1.0 / var_Dimensions.y);
vec2 offset2 = vec2(1.0 / var_Dimensions.x, 0.0);

vec3 normal_from_depth(float depth, vec2 texcoords)
{
#if defined(USE_NORMALMAP)
	return texture(u_NormalMap, texcoords).rgb * 2.0 - 1.0;
#else //!defined(USE_NORMALMAP)
    float depth1 = getLinearDepth(u_ScreenDepthMap, texcoords + offset1);
    float depth2 = getLinearDepth(u_ScreenDepthMap, texcoords + offset2);
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;
    return normalize(normal);
#endif //defined(USE_NORMALMAP)
}

float ambientOcclusion(vec2 tex)
{
    //vec3 random = normalize( texture2D(u_SpecularMap, tex * 4.0).rgb );
    float depth = getLinearDepth(u_ScreenDepthMap, tex);
    vec3 position = vec3(tex, depth);
    vec3 normal = normal_from_depth(depth, tex);
    float radius_depth = radius/depth;
    float occlusion = 0.0;

    for(int i=0; i < samples; i++) {
        vec3 ray = radius_depth * reflect(sample_sphere[i], normal);//random);
		vec3 hemi_ray = position + (sign(dot(ray,normal)) * ray);
        float occ_depth = getLinearDepth(u_ScreenDepthMap, clamp(hemi_ray.xy, 0.0, 1.0));
        float difference = depth - occ_depth;
        occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
    }

	float ao = 1.0 - clamp(total_strength * occlusion * (1.0 / samples), 0.0, 1.0);
    return clamp(ao + base, 0.0, 1.0);
}

void main()
{
    //vLocalSeed = var_Position.xyz;
    float ao = clamp(ambientOcclusion(var_ScreenTex), 0.0, 1.0);
    gl_FragColor = vec4(vec3(ao), 1.0);
}

#endif //0







uniform sampler2D   u_ScreenDepthMap; //depth
uniform sampler2D   u_NormalMap; //normal

uniform vec2        u_Dimensions;
uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform mat4        u_ModelViewProjectionMatrix;

uniform vec4		u_Local0;

const float camerazoom = 1.0;

//#define raycasts 4			//Increase for better quality
//#define raysegments 6			//Increase for better quality
//#define aoscatter 1.0

#define raycasts 1			//Increase for better quality
#define raysegments 1			//Increase for better quality
#define aoscatter 0.5

//#define raycasts u_Local0.r
//#define raysegments u_Local0.g
//#define aoscatter u_Local0.b

float linearize(float depth)
{
	//float d = clamp((1.0 / mix(u_ViewInfo.z, 1.0, depth)) + u_Local0.g * u_Local0.b, 0.0, 1.0);
	float d = clamp(pow((1.0 / mix(u_ViewInfo.z, 1.0, depth)), 0.2), 0.0, 1.0);
	return d;
}

float DepthToZPosition(in float depth) {
	return u_ViewInfo.x / (u_ViewInfo.y - depth * (u_ViewInfo.y - u_ViewInfo.x)) * u_ViewInfo.y;
}

float GetDepth(in vec2 texcoord)
{
	return linearize(texture2D(u_ScreenDepthMap, texcoord).x);
}

float readZPosition(in vec2 texcoord) {
    return DepthToZPosition( GetDepth( texcoord /*+ (0.25 / u_Dimensions)*/ ) );
}

mat3 vec3tomat3( in vec3 z ) {
        mat3 mat;
        mat[2]=z;
        vec3 v=vec3(z.z,z.x,-z.y);//make a random vector that isn't the same as vector z
        mat[0]=cross(z,v);//cross product is the x axis
        mat[1]=cross(mat[0],z);//cross product is the y axis
        return mat;
}

float compareDepths( in float depth1, in float depth2, in float m ) {
        /*float mindiff=0.05*m;
        float middiff=0.25*m;
        float maxdiff=1.30*m;
        float enddiff=1.50*m;*/
        
        float diff = (depth1-depth2);

		//if (diff>u_Local0.r) return 1.0;

		return 1.0 - clamp(diff * 128.0, 0.0, 1.0);
		
        /*if (diff<mindiff) {
                return 1.0;
        }*/
        /*if (diff<middiff) {
                diff -= mindiff;
                return 1.0-(diff/(middiff-mindiff));
        }*/
        /*if (diff<maxdiff) {
                return 0.0;
        }*/
        /*if (diff<enddiff) {
                diff -= maxdiff;
                return diff/(enddiff-maxdiff);
        }*/
        return 1.0;     
}

// Calculates the screen-space position of a pixel
// Make sure the depth lookup is using the right texture!!!
vec3 GetPixelPosition( in vec2 coord ) {
        vec3 pos;
        vec2 hu_Dimensions=u_Dimensions/2.0;
        pos.z = DepthToZPosition(GetDepth(coord));
        pos = vec3((((coord.x+0.5)/hu_Dimensions.x)-0.5) * 2.0,(((-coord.y+0.5)/hu_Dimensions.y)+0.5) * 2.0 / (hu_Dimensions.x/hu_Dimensions.y),pos.z);
        pos.x *= pos.z / camerazoom;
        pos.y *= -pos.z / camerazoom;
        return pos;
}

vec3 ScreenCoordToPosition( in vec2 coord ) {
        vec3 pos;
        vec2 hu_Dimensions=u_Dimensions/2.0;
        pos.z = DepthToZPosition(GetDepth(coord));
        pos.x = ((((coord.x+0.5)/hu_Dimensions.x)-0.5) * 2.0)*(pos.z / camerazoom);
        pos.y = ((((-coord.y+0.5)/hu_Dimensions.y)+0.5) * 2.0 / (hu_Dimensions.x/hu_Dimensions.y))*(-pos.z / camerazoom);
        return pos;
}

vec3 ScreenCoordToPosition2( in vec2 coord, in float z ) {
        vec3 pos;
        vec2 hu_Dimensions=u_Dimensions/2.0;
        pos.z = z;
        pos.x = ((((coord.x+0.5)/hu_Dimensions.x)-0.5) * 2.0)*(pos.z / camerazoom);
        pos.y = ((((-coord.y+0.5)/hu_Dimensions.y)+0.5) * 2.0 / (hu_Dimensions.x/hu_Dimensions.y))*(-pos.z / camerazoom);
        return pos;
}

//Converts a screen position to texture coordinate
vec2 ScreenPositionToCoord( in vec3 pos ) {
        vec2 coord;
        vec2 hu_Dimensions=u_Dimensions/2.0;
        pos.x /= (pos.z / camerazoom);
        coord.x = (pos.x / 2.0 + 0.5);
        
        pos.y /= (-pos.z / camerazoom) / (hu_Dimensions.x/hu_Dimensions.y);
        coord.y = -(pos.y / 2.0 - 0.5);
        
        return coord;// + 0.5/(u_Dimensions/2.0);
}

vec4 PlaneFromPointNormal( in vec3 point, in vec3 normal ) {
        vec4 plane;
        plane.x = normal.x;
        plane.y = normal.y;
        plane.z = normal.z;
        plane.w = -dot(point,normal);
        return plane;
}

vec2 offset1 = vec2(0.0, 1.0 / u_Dimensions.y);
vec2 offset2 = vec2(1.0 / u_Dimensions.x, 0.0);

vec3 normal_from_depth(vec2 texcoords) {
	float depth = GetDepth(texcoords);
	float depth1 = GetDepth(texcoords + offset1);
	float depth2 = GetDepth(texcoords + offset2);
  
	vec3 p1 = vec3(offset1, depth1 - depth);
	vec3 p2 = vec3(offset2, depth2 - depth);
  
	vec3 normal = cross(p1, p2);
	normal.z = -normal.z;
  
	return normalize(normal);
}

vec3 GetPixelNormal( in vec2 coord ) {
		return normal_from_depth(coord);
}

vec4 GetPixelPlane( in vec2 coord ) {
        vec3 point = GetPixelPosition( coord );
        vec3 normal = GetPixelNormal( coord );
        return PlaneFromPointNormal( point, normal);
}

float PointPlaneDistance( in vec4 plane, in vec3 point ) {
        return plane.x*point.x+plane.y*point.y+plane.z*point.z+plane.w;
}

float rand(vec2 co) {
        return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main( void ) {
        float ao=0.0;
        float dn;
        float zd;
        float newdepth;
        float angletest;
        float lineardepth;
        float depth;
        vec3 newnormal;
        vec3 newdiffuse;
        vec3 normal;
        vec2 texcoord2;
        vec2 texcoord = gl_FragCoord.xy/u_Dimensions + 0.5/(u_Dimensions*0.5);
        vec3 screencoord;
        vec4 outcolor;

		depth=GetDepth( texcoord );
        
        if (depth<1.0) {
                
				lineardepth = DepthToZPosition(depth);

                normal = GetPixelNormal( texcoord ).rgb;
                normal=normalize(normal);
                normal.z=-normal.z;
                
                vec3 p0=ScreenCoordToPosition2(vec2(0.0,0.5),lineardepth);
                vec3 p1=ScreenCoordToPosition2(vec2(1.0,0.5),lineardepth);
                float dist = abs(p1.x-p0.x);
                
                screencoord = vec3((((gl_FragCoord.x+0.5)/u_Dimensions.x)-0.5) * 2.0,(((-gl_FragCoord.y+0.5)/u_Dimensions.y)+0.5) * 2.0 / (u_Dimensions.x/u_Dimensions.y),lineardepth);
                screencoord.x *= screencoord.z / camerazoom;
                screencoord.y *= -screencoord.z / camerazoom;
                
                vec3 newpoint;
                vec2 coord;
                vec3 raynormal;
                vec3 offsetvector;
                float diff;             
                vec2 randcoord;         
                float randsum = u_ModelViewProjectionMatrix[0][0]+u_ModelViewProjectionMatrix[0][1]+u_ModelViewProjectionMatrix[0][2];
                randsum+=u_ModelViewProjectionMatrix[1][0]+u_ModelViewProjectionMatrix[1][1]+u_ModelViewProjectionMatrix[1][2];
                randsum+=u_ModelViewProjectionMatrix[2][0]+u_ModelViewProjectionMatrix[2][1]+u_ModelViewProjectionMatrix[2][2];
                randsum+=u_ModelViewProjectionMatrix[3][0]+u_ModelViewProjectionMatrix[3][1]+u_ModelViewProjectionMatrix[3][2];
                
                float raylength = 0.5;//*dist*1.0*50.0;
                float cdm = 1.0;// * dist * 50.0;
                
                ao=0.0;
                
                vec4 gicolor;
                float gisamples;
                mat3 mat=vec3tomat3(normal);
                float a;
                float wheredepthshouldbe;
                
                float mix=(1.0-(clamp(lineardepth-50.0,0.0,50.0)/50.0));
                
                //Get a random number
                a=rand( randsum+texcoord);// + float(53.0*m*raysegments + i*13.0) );
                                        
                if (mix>0.0) {
                        for ( int i=0;i<(raycasts);i++ ) {
                                for ( int m=0;m<(raysegments);m++ ) {
                                        

                                        
                                        offsetvector.x=cos(a+float(i)/float(raycasts)*3.14*4.0)*aoscatter;
                                        offsetvector.y=sin(a+float(i)/float(raycasts)*3.14*4.0)*aoscatter;
                                        offsetvector.z=1.0;
                                        offsetvector=normalize(offsetvector);
                                        
                                        //Create the ray vector
                                        raynormal=mat*offsetvector;
                                        
                                        //Add the ray vector to the screen position
                                        newpoint = screencoord + raynormal * (raylength/raysegments) * float(m+1);
                                        wheredepthshouldbe=newpoint.z;
                                        
                                        //Turn the point back into a screen coord:
                                        coord = ScreenPositionToCoord( newpoint );
                                        
                                        //Look up the depth value at that rays position
                                        newdepth = readZPosition( coord );
                                        
                                        //If new depth is closer to camera darken ao value
                                        //rayao = max(rayao,compareDepths( lineardepth, newdepth ));
                                        //ao = min(ao,compareDepths( lineardepth, newdepth ));
                                        ao += compareDepths( wheredepthshouldbe,newdepth, cdm );
                                        //ao+=compareDepths( lineardepth, newdepth );
                                }
                        }
                }
                
                ao /= (raycasts*raysegments);
                //ao = max(ao * ao,0.5);
                ao = ao * mix + (1.0-mix);              
                gl_FragColor=vec4(ao);
        }
        
        else {
                gl_FragColor=vec4(1.0);
        }
        
}

uniform sampler2D	u_TextureMap; // Screen Image...
uniform sampler2D	u_NormalMap; // Water Map...
uniform sampler2D	u_SubsurfaceMap; // Sky Image...
uniform sampler2D	u_ScreenDepthMap; // Depth Map...
uniform sampler2D	u_SpecularMap; // Random Map...

uniform vec2		u_Dimensions;
uniform float		u_Time;

varying vec2		var_TexCoords;
varying vec3		var_ViewDir;
varying vec3		var_position;
varying vec3		var_viewOrg;

#if 0


#if 0

const int NUM_STEPS = 8;
const float PI	 	= 3.1415;
const float EPSILON	= 1e-3;
float EPSILON_NRM	= 0.1;// / iResolution.x;

// sea
const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.1,0.19,0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
float SEA_TIME = u_Time * SEA_SPEED;
mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

// math
mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
	return m;
}
float hash( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}
#if 0
// 2d noise function
float noise(vec2 p)
{
  return texture2D(u_SpecularMap,p*vec2(1./256.)).x;
}
#else
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}
#endif

// lighting
float diffuse(vec3 n,vec3 l,float p) {
    return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(vec3 n,vec3 l,vec3 e,float s) {    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sky
vec3 getSkyColor(vec3 e) {
    e.y = max(e.y,0.0);
    vec3 ret;
    ret.x = pow(1.0-e.y,2.0);
    ret.y = 1.0-e.y;
    ret.z = 0.6+(1.0-e.y)*0.4;
    return ret;
}

// sea
float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float map_detailed(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {  
    float fresnel = 1.0 - max(dot(n,-eye),0.0);
    fresnel = pow(fresnel,3.0) * 0.65;
        
    vec3 reflected = getSkyColor(reflect(eye,n));    
    vec3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.12; 
    
    vec3 color = mix(refracted,reflected,fresnel);
    
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    color += vec3(specular(n,l,eye,60.0));
    
    return color;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
    vec3 n;
    n.y = map_detailed(p);    
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {  
    float tm = 0.0;
    float tx = 1000.0;    
    float hx = map(ori + dir * tx);
    if(hx > 0.0) return tx;   
    float hm = map(ori + dir * tm);    
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));                   
        p = ori + dir * tmid;                   
    	float hmid = map(p);
		if(hmid < 0.0) {
        	tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

// main
vec4 doWater( in vec2 fragCoord ) 
{
	vec2 uv = fragCoord.xy;
    uv = uv - 1.1;
    float time = u_Time * 0.3;
    
    //vec2 trueView = normalize(var_ViewDir.xy);//vec2(0.0, 0.0);
	//vec2 trueView = var_ViewDir.yx / 180.0;
	vec2 trueView = vec2(var_ViewDir.x / 360.0, ((0.0 - var_ViewDir.y) / 360.0));

	var_ViewDir.x /= PI*2.0;

	if (trueView.y > -0.001) trueView.y = -0.999 + trueView.y;
	if (trueView.x > -0.001) trueView.x = -0.999 + trueView.x;
        
    // ray
    vec3 ang = vec3(PI, PI * trueView.x, PI * trueView.y);
    vec3 ori = vec3(0.0, 3.0, 0.0);
    vec3 dir = normalize(vec3(uv.xy,2.0)); dir.z += length(uv) * 0.15;
    dir = normalize(dir) * fromEuler(ang);
    
    // tracing
    vec3 p;
    heightMapTracing(ori,dir,p);
    vec3 dist = p - ori;
    float d = length(dist);
    vec3 n = getNormal(p, d*d*.0003);// Or whatever value that suits.
    //vec3 n = getNormal(p, dot(dist,dist) * EPSILON_NRM);
    vec3 light = normalize(vec3(0.0,1.0,0.8)); 
             
    // color
    //vec3 color = mix(
    //    getSkyColor(dir),
    //    getSeaColor(p,n,light,dir,dist),
    //	pow(smoothstep(0.0,-0.05,dir.y),0.3));
    vec3 color = getSeaColor(p,n,light,dir,dist);
        
    // post
	return vec4(pow(color,vec3(0.75)), 1.0);
}



#else


//uniform sampler2D	u_TextureMap; // Screen Image...
//uniform sampler2D	u_NormalMap; // Water Map...
//uniform sampler2D	u_SubsurfaceMap; // Sky Image...
//uniform sampler2D	u_ScreenDepthMap; // Depth Map...
//uniform sampler2D	u_SpecularMap; // Random Map...


#define heightMap u_ScreenDepthMap
#define backBufferMap u_TextureMap
//#define backBufferMap u_ScreenDepthMap
#define positionMap u_ScreenDepthMap
#define normalMap u_TextureMap
#define foamMap u_SubsurfaceMap
#define reflectionMap u_SubsurfaceMap

#define saturate(a) clamp(a, 0.0, 1.0)
#define lerp(a, b, t) mix(a, b, t)

//mat4 matViewProj;
uniform mat4	u_ModelViewProjectionMatrix;
#define matViewProj u_ModelViewProjectionMatrix

// We need this matrix to restore position in world space
//uniform mat4 u_invEyeProjectionMatrix;
uniform mat4 u_invProjectionMatrix;

#define u_invEyeProjectionMatrix u_invProjectionMatrix
//#define u_invEyeProjectionMatrix u_ModelViewProjectionMatrix

// Level at which water surface begins
float waterLevel = 0.0;

// Position of the camera
vec3 cameraPos = var_viewOrg;

// How fast will colours fade out. You can also think about this
// values as how clear water is. Therefore use smaller values (eg. 0.05f)
// to have crystal clear water and bigger to achieve "muddy" water.
float fadeSpeed = 0.15;

// Timer
float timer = u_Time;

// Normals scaling factor
float normalScale = 1.0;

// R0 is a constant related to the index of refraction (IOR).
// It should be computed on the CPU and passed to the shader.
float R0 = 0.5;

// Maximum waves amplitude
float maxAmplitude = 1.0;

// Direction of the light
vec3 lightDir = vec3(0.0, 1.0, 0.0);

// Colour of the sun
vec3 sunColor = vec3(1.0, 1.0, 1.0);

// The smaller this value is, the more soft the transition between
// shore and water. If you want hard edges use very big value.
// Default is 1.0f.
float shoreHardness = 1.0;

// This value modifies current fresnel term. If you want to weaken
// reflections use bigger value. If you want to empasize them use
// value smaller then 0. Default is 0.0f.
float refractionStrength = 0.0;
//float refractionStrength = -0.3;

// Modifies 4 sampled normals. Increase first values to have more
// smaller "waves" or last to have more bigger "waves"
vec4 normalModifier = vec4(1.0, 2.0, 4.0, 8.0);

// Strength of displacement along normal.
float displace = 1.7;

// Describes at what depth foam starts to fade out and
// at what it is completely invisible. The fird value is at
// what height foam for waves appear (+ waterLevel).
vec3 foamExistence = vec3(0.65, 1.35, 0.5);

float sunScale = 3.0;

/*
HLSL 
float4x4 = // [row][column] 
[0][0], [0][1], [0][2], [0][3] 
[1][0], [1][1], [1][2], [1][3] 
[2][0], [2][1], [2][2], [2][3] 
[3][0], [3][1], [3][2], [3][3]

GLSL 
float4x4 = // [column][row] 
[0][0], [0][1], [0][2], [0][3] 
[1][0], [1][1], [1][2], [1][3] 
[2][0], [2][1], [2][2], [2][3] 
[3][0], [3][1], [3][2], [3][3]
*/

/*
mat4 matReflection =
mat4(
	vec4(0.5, 0.0, 0.0, 0.5),
	vec4(0.0, 0.5, 0.0, 0.5),
	vec4(0.0, 0.0, 0.0, 0.5),
	vec4(0.0, 0.0, 0.0, 1.0)
);
*/
mat4 matReflection =
mat4(
	vec4(0.5, 0.0, 0.0, 0.0),
	vec4(0.0, 0.5, 0.0, 0.0),
	vec4(0.0, 0.0, 0.0, 0.0),
	vec4(0.5, 0.5, 0.5, 1.0)
);


float shininess = 0.7;
float specular_intensity = 0.32;

// Colour of the water surface
vec3 depthColour = vec3(0.0078, 0.5176, 0.7);
// Colour of the water depth
vec3 bigDepthColour = vec3(0.0039, 0.00196, 0.145);
vec3 extinction = vec3(7.0, 30.0, 40.0);			// Horizontal

// Water transparency along eye vector.
float visibility = 4.0;

// Increase this value to have more smaller waves.
vec2 scale = vec2(0.005, 0.005);
float refractionScale = 0.005;

// Wind force in x and z axes.
vec2 wind = vec2(-0.3, 0.7);

vec3 DepthToPos(vec2 texcoord)
{
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texcoord * 2.0 - 1.0;
  clipSpaceLocation.z = texture2D(u_ScreenDepthMap, texcoord).r * 2.0 - 1.0;
  clipSpaceLocation.w = 1.0;
  vec4 homogenousLocation = u_invProjectionMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
}

float depthMult = 1.0;//255.0;
float ratex = (1.0/u_Dimensions.x);
float ratey = (1.0/u_Dimensions.y);

vec2 offset1 = vec2(0.0, ratey);
vec2 offset2 = vec2(ratex, 0.0);

vec3 normal_from_depth(float depth, vec2 texcoords) {
  float depth1 = texture2D(u_ScreenDepthMap, texcoords + offset1).r * depthMult;
  float depth2 = texture2D(u_ScreenDepthMap, texcoords + offset2).r * depthMult;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

vec4 GetHeightMap(vec2 coord)
{
	return texture2D(heightMap, coord) * depthMult;
	//return texture2D(u_NormalMap, coord);
}

vec4 GetBackBufferMap(vec2 coord)
{
	return texture2D(backBufferMap, coord);
}

vec4 GetPositionMap(vec2 coord)
{
	//return texture2D(positionMap, coord);
	return vec4(DepthToPos(coord), 1.0);
}

vec4 GetNormalMap(vec2 coord)
{
	//return texture2D(normalMap, coord) * u_Time;
	return vec4(normal_from_depth(texture2D(u_ScreenDepthMap, coord).r * depthMult, coord), 1.0);
}

vec4 GetFoamMap(vec2 coord)
{
	return texture2D(foamMap, coord);
}

vec4 GetReflectionMap(vec3 coord)
{
	return texture2DProj(u_TextureMap/*reflectionMap*/, coord);
}


mat3 compute_tangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

// Function calculating fresnel term.
// - normal - normalized normal vector
// - eyeVec - normalized eye vector
float fresnelTerm(vec3 normal, vec3 eyeVec)
{
		float angle = 1.0 - saturate(dot(normal, eyeVec));
		float fresnel = angle * angle;
		fresnel = fresnel * fresnel;
		fresnel = fresnel * angle;
		return saturate(fresnel * (1.0 - saturate(R0)) + R0 - refractionStrength);
}

vec4 doWater( in vec2 fragCoord, vec4 waterMapColor ) 
{
	vec3 color2 = GetBackBufferMap(fragCoord).rgb;
	vec3 color = color2;

	//return vec4(color2.rgb, 1.0);
	//return GetFoamMap(fragCoord);
	
	vec3 position = (u_invEyeProjectionMatrix * vec4(GetPositionMap(fragCoord).xyz, 1.0)).xyz;
	float level = waterLevel;
	float depth = 0.0;

	//return vec4(vec3(cameraPos.z), 1.0);
	
	// If we are underwater let's leave out complex computations
	if(level >= cameraPos.y)
		return vec4(color2, 1.0);
	
	if(position.y <= level + maxAmplitude)
	{
		vec3 eyeVec = position - cameraPos;
		float diff = level - position.y;
		float cameraDepth = cameraPos.y - position.y;
		
		// Find intersection with water surface
		vec3 eyeVecNorm = normalize(eyeVec);
		float t = (level - cameraPos.y) / eyeVecNorm.y;
		vec3 surfacePoint = cameraPos + eyeVecNorm * t;
		
		eyeVecNorm = normalize(eyeVecNorm);
		
		vec2 texCoord;
		for(int i = 0; i < 10; ++i)
		{
			texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * scale + timer * 0.000005 * wind;
			
			float bias = GetHeightMap(texCoord).r;
	
			bias *= 0.1;
			level += bias * maxAmplitude;
			t = (level - cameraPos.y) / eyeVecNorm.y;
			surfacePoint = cameraPos + eyeVecNorm * t;
		}
		
		depth = length(position - surfacePoint);
		float depth2 = surfacePoint.y - position.y;
		
		eyeVecNorm = normalize(cameraPos - surfacePoint);
		
		float normal1 = GetHeightMap((texCoord + vec2(-1, 0) / 256)).r;
		float normal2 = GetHeightMap((texCoord + vec2(1, 0) / 256)).r;
		float normal3 = GetHeightMap((texCoord + vec2(0, -1) / 256)).r;
		float normal4 = GetHeightMap((texCoord + vec2(0, 1) / 256)).r;
		
		vec3 myNormal = normalize(vec3((normal1 - normal2) * maxAmplitude,
										   normalScale,
										   (normal3 - normal4) * maxAmplitude));   
		
		texCoord = surfacePoint.xz * 1.6 + wind * timer * 0.00016;
		mat3 tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal0a = normalize(((2.0 * GetNormalMap(texCoord).rgb - 1.0) * tangentFrame));

		texCoord = surfacePoint.xz * 0.8 + wind * timer * 0.00008;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal1a = normalize(((2.0 * GetNormalMap(texCoord).rgb - 1.0) * tangentFrame));
		
		texCoord = surfacePoint.xz * 0.4 + wind * timer * 0.00004;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal2a = normalize(((2.0 * GetNormalMap(texCoord).rgb - 1.0) * tangentFrame));
		
		texCoord = surfacePoint.xz * 0.1 + wind * timer * 0.00002;
		tangentFrame = compute_tangent_frame(myNormal, eyeVecNorm, texCoord);
		vec3 normal3a = normalize(((2.0 * GetNormalMap(texCoord).rgb - 1.0) * tangentFrame));
		
		vec3 normal = normalize(normal0a * normalModifier.x + normal1a * normalModifier.y +
								  normal2a * normalModifier.z + normal3a * normalModifier.w);
		
		texCoord = fragCoord.xy;
		texCoord.x += sin(timer * 0.002 + 3.0 * abs(position.y)) * (refractionScale * min(depth2, 1.0));
		vec3 refraction = GetBackBufferMap(texCoord).rgb;
		if((vec4(GetPositionMap(texCoord).xyz, 1.0) * u_invEyeProjectionMatrix).y > level)
			refraction = color2;

		mat4 matTextureProj = (matReflection * matViewProj);
				
		vec3 waterPosition = surfacePoint.xyz;
		waterPosition.y -= (level - waterLevel);
		vec4 texCoordProj = (vec4(waterPosition, 1.0) * matTextureProj);
		
		vec4 dPos;
		dPos.x = texCoordProj.x + displace * normal.x;
		dPos.z = texCoordProj.z + displace * normal.z;
		dPos.yw = texCoordProj.yw;
		texCoordProj = dPos;		
		
		vec3 reflect = GetReflectionMap(texCoordProj);
		return vec4(reflect, 1.0);
		
		float fresnel = fresnelTerm(normal, eyeVecNorm);
		
		vec3 depthN = depth * fadeSpeed;
		vec3 waterCol = saturate(length(sunColor) / sunScale);
		refraction = lerp(lerp(refraction, depthColour * waterCol, saturate(depthN / visibility)),
						  bigDepthColour * waterCol, saturate(depth2 / extinction));

		float foam = 0.0;		

		texCoord = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00001 * wind + sin(timer * 0.001 + position.x) * 0.005;
		vec2 texCoord2 = (surfacePoint.xz + eyeVecNorm.xz * 0.1) * 0.05 + timer * 0.00002 * wind + sin(timer * 0.001 + position.z) * 0.005;
		
		if(depth2 < foamExistence.x)
			foam = (GetFoamMap(texCoord) + GetFoamMap(texCoord2)) * 0.5;
		else if(depth2 < foamExistence.y)
		{
			foam = lerp((GetFoamMap(texCoord) + GetFoamMap(texCoord2)) * 0.5, 0.0,
						 (depth2 - foamExistence.x) / (foamExistence.y - foamExistence.x));
			
		}
		
		if(maxAmplitude - foamExistence.z > 0.0001)
		{
			foam += (GetFoamMap(texCoord) + GetFoamMap(texCoord2)) * 0.5 * 
				saturate((level - (waterLevel + foamExistence.z)) / (maxAmplitude - foamExistence.z));
		}


		vec3 specular = vec3(0.0);

		vec3 mirrorEye = (2.0 * dot(eyeVecNorm, normal) * normal - eyeVecNorm);
		float dotSpec = saturate(dot(mirrorEye.xyz, -lightDir) * 0.5 + 0.5);
		specular = (1.0 - fresnel) * saturate(-lightDir.y) * ((pow(dotSpec, 512.0)) * (shininess * 1.8 + 0.2))* sunColor;
		specular += specular * 25 * saturate(shininess - 0.05) * sunColor;		

		color = lerp(refraction, reflect, fresnel);
		color = saturate(color + max(specular, foam * sunColor));
		
		color = lerp(refraction, color, saturate(depth * shoreHardness));

		//color = vec3(depth * shoreHardness);
	}
	
	if(position.y > level)
		color = color2;

	return vec4(color, 1.0);
}
#endif

void main()
{
	vec4 origColor = texture2D(u_TextureMap, var_TexCoords);
	vec4 waterMapColor = texture2D(u_NormalMap, var_TexCoords);

	if (waterMapColor.a > 0.0)
	{// This pixel is water...
		/*
		float trans = 0.2;
		
		vec4 origColorFinal = vec4(origColor.rgb * trans, origColor.a);
		vec4 waterColor = doWater( var_TexCoords ) * (1.0 - trans);

		gl_FragColor.rgb = (origColorFinal.rgb + waterColor.rgb) / 2.0;
		gl_FragColor.a = 1.0;
		*/

		//vec2 trueView = (var_ViewDir.xy + 180.0) / 360.0;
		//gl_FragColor = vec4(trueView.xy, 0.0, 1.0);

		vec4 waterColor = doWater( var_TexCoords, waterMapColor );
		gl_FragColor = waterColor;
	}
	else
	{
		gl_FragColor = origColor;
	}
}

#endif

void main()
{
	gl_FragColor = texture2D(u_TextureMap, var_TexCoords);
}

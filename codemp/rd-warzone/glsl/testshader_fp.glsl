/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define tex2D(tex, coord) texture2D(tex, coord)
#define tex2Dlod(tex, coord) texture2D(tex, coord)
#define lerp(a, b, t) mix(a, b, t)
#define saturate(a) clamp(a, 0.0, 1.0)
#define mad(a, b, c) (a * b + c)
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4
#define frac fract

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define UQAO
//#define SCREEN_SPACE_CURVATURE

uniform mat4		u_ModelViewProjectionMatrix;
uniform mat4		u_ModelViewMatrix;

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local1;		// testshadervalue1, testshadervalue2, testshadervalue3, testshadervalue4
uniform vec4		u_Local2;		// sun dir
uniform vec4		u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

varying vec2		var_TexCoords;
varying vec3		var_vertPos;
varying vec3		var_viewOrg;
varying vec3		var_rayOrg;
varying vec3		var_sunOrg;
varying vec3		var_rayDir;
varying vec3		var_sunDir;


#if defined(UQAO)

//==RAY MARCHING CONSTANTS=========================================================
#define EPSILON .0001
#define MAX_VIEW_STEPS 100
#define MAX_SHADOW_STEPS 64
#define OCCLUSION_SAMPLES 8.0
#define OCCLUSION_FACTOR .5
#define MAX_DEPTH 10.0
#define BUMP_FACTOR .04
#define TEX_SCALE_FACTOR .4

#define PEN_FACTOR 50.0


vec3 Matrixify(vec3 pos)
{
	return vec4(u_ModelViewProjectionMatrix * vec4(pos, 1.0)).xyz;
}

float getDist(vec3 pos, vec3 samplePos)
{
	vec2 coord;
	vec3 sPos = samplePos;

	coord = var_TexCoords + ((pos.xy - sPos.xy) * (1.0 / u_Dimensions));

	if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
	{// Off screen...
		return 4.0;
	}

	vec3 nPos = Matrixify(texture2D(u_PositionMap, coord).xyz);
	return min(length(nPos - pos) * 0.2, 4.0) * 128.0;
}

const float PI = 3.14159265359;

float hash( float n )//->0:1
{
    return fract(sin(n)*3538.5453);
}

vec3 randomSphereDir(vec2 rnd)
{
	float s = rnd.x*PI*2.;
	float t = rnd.y*2.-1.;
	return vec3(sin(s), cos(s), t) / sqrt(1.0 + t * t);
}

vec3 randomHemisphereDir(vec3 dir, float i)
{
	vec3 v = randomSphereDir( vec2(hash(i+1.), hash(i+2.)) );
	return v * sign(dot(v, dir));
}

float ambientOcclusion( in vec3 p, in vec3 n, float maxDist, float falloff )
{
	const int nbIte = 32;
    const float nbIteInv = 1./float(nbIte);
    const float rad = 1.-1.*nbIteInv; //Hemispherical factor (self occlusion correction)
    
	float ao = 0.0;
    
    for( int i=0; i<nbIte; i++ )
    {
        float l = hash(float(i))*maxDist;
        vec3 rd = normalize(n+randomHemisphereDir(n, l )*rad)*l; // mix direction with the normal
        
        ao += (l - /*map( p + rd ).x*/getDist(p, p + rd) * u_Local0.g) / pow(1.+l, falloff);
    }
	
    return clamp( 1.-ao*nbIteInv, 0., 1.);
}

float calcOcclusion(vec3 pos, vec3 surfaceNormal)
{
	return ambientOcclusion( pos, surfaceNormal, 1.0, 0.001 );
	/*
	float result = 0.0;
	vec3 normalPos = vec3(pos);
	for(float i = 0.0; i < OCCLUSION_SAMPLES; i+=1.0)
	{
		normalPos += surfaceNormal * (1.0/OCCLUSION_SAMPLES);
		result += (1.0/exp2(i)) * (i/OCCLUSION_SAMPLES)-getDist(pos, normalPos);
	}
	return 1.0-(OCCLUSION_FACTOR*result);
	*/
}

float calcShadow( vec3 origin, vec3 lightDir, vec3 lightpos, float penumbraFactor, vec3 viewPos)
{
	float dist;
	float result = 1.0;
	float lightDist = length(lightpos-origin);
	
	vec3 pos = vec3(origin)+(lightDir*(EPSILON*15.0+BUMP_FACTOR));
	
	for(int i = 0; i < MAX_SHADOW_STEPS; i++)
	{
		dist = getDist(origin, pos);
		if(dist < EPSILON)
		{
			return 0.0;
		}
		if(length(pos-origin) > lightDist || length(pos-origin) > MAX_DEPTH)
		{
			return result;
		}
		pos+=lightDir*dist;
		if( length(pos-origin) < lightDist )
		{
			result = min( result, penumbraFactor*dist / length(pos-origin) );
		}
	}
	return result;
}

vec4 GetPosAndDepth(sampler2D tex, ivec2 coord, int num)
{
	vec4 pos = texelFetch(tex, coord, num);
	pos.w = length((pos.xyz - u_ViewOrigin) / u_MapInfo.xyz);
	return pos;
}

void main ( void )
{
	vec2 coord = var_TexCoords;
	vec3 normal = texture2D(u_NormalMap, coord).xyz * 2.0 - 1.0;

	/*if (u_Local0.a > 0.0)
	{// Just draw screen normals for debugging...
		gl_FragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}*/

	vec4 color = texture2D(u_DiffuseMap, coord);
	vec4 positionMap = texture2D(u_PositionMap, coord);

	if (positionMap.a == 0.0)
	{// No material info on this pixel, it is most likely sky or generic shader...
		gl_FragColor = color;
		return;
	}

	vec3 pos = Matrixify(positionMap.xyz);

	float occlusion = (1.0 - clamp(calcOcclusion(pos, normal) * u_Local0.r/*0.0002*/, 0.0, 1.0));
	color.rgb *= occlusion;
	//color.rgb = vec3(occlusion);

	gl_FragColor = color;
}

#elif defined(SCREEN_SPACE_CURVATURE)

#extension GL_OES_standard_derivatives : enable

vec4 GetNormalAndHeight(vec2 coord)
{
	vec4 norm = texture2D(u_NormalMap, coord);
	return norm;
}

vec4 GetPosAndDepth(vec2 coord)
{
	vec4 pos = texture2D(u_PositionMap, coord);
	pos.w = length((pos.xyz - u_ViewOrigin) / u_MapInfo.xyz);
	return pos;
}

vec3 GetVertexForPosition(vec3 pos)
{
	return (u_ModelViewMatrix * vec4(pos.xyz, 1.0)).xyz;
}

void main() {
  vec3 normal = GetNormalAndHeight(var_TexCoords).xyz * 2.0 - 1.0;
  vec4 pos = GetPosAndDepth(var_TexCoords);
  vec3 vertex = GetVertexForPosition(pos.xyz);
  vec4 color = texture2D(u_DiffuseMap, var_TexCoords);

  vec3 n = normalize(normal);

  // Compute curvature
  vec3 dx = dFdx(n);
  vec3 dy = dFdy(n);
  vec3 xneg = n - dx;
  vec3 xpos = n + dx;
  vec3 yneg = n - dy;
  vec3 ypos = n + dy;
  float depth = u_Local0.g;//pos.a;//length(vertex);
  float curvature = ((cross(xneg, xpos).y * u_Dimensions.y) - (cross(yneg, ypos).x * u_Dimensions.x))/* * 4.0*/ * u_Local0.r / depth;

  //curvature *= u_Local0.r;

#define TEST_SCENE
  // Compute surface properties
#if defined(TEST_SCENE)
  float corrosion = clamp(-curvature * 3.0, 0.0, 1.0);
  float shine = clamp(curvature * 5.0, 0.0, 1.0);
  vec3 light = normalize(vec3(0.0, 1.0, 10.0));
  vec3 ambient = color.rgb;//vec3(0.15, 0.1, 0.1);
  //vec3 diffuse = mix(mix(vec3(0.3, 0.25, 0.2), vec3(0.45, 0.5, 0.5), corrosion),
   // vec3(0.5, 0.4, 0.3), shine) - ambient;
vec3 diffuse = mix(mix(vec3(0.3, 0.25, 0.2) * color.rgb, vec3(0.45, 0.5, 0.5) * color.rgb, corrosion),
    vec3(0.5, 0.4, 0.3) * color.rgb, shine) - ambient;
  vec3 specular = mix(vec3(0.0), vec3(1.0) - ambient - diffuse, shine);
  float shininess = 128.0;
#elif defined(TEST_RED_WAX)
  float dirt = clamp(0.25 - curvature * 4.0, 0.0, 1.0);
  vec3 light = normalize(vec3(0.0, 1.0, 10.0));
  vec3 ambient = vec3(0.1, 0.05, 0.0);
  vec3 diffuse = mix(vec3(0.4, 0.15, 0.1), vec3(0.4, 0.3, 0.3), dirt) - ambient;
  vec3 specular = mix(vec3(0.15) - ambient, vec3(0.0), dirt);
  float shininess = 32.0;
#elif defined(TEST_METAL)
  float corrosion = clamp(-curvature * 3.0, 0.0, 1.0);
  float shine = clamp(curvature * 5.0, 0.0, 1.0);
  vec3 light = normalize(vec3(0.0, 1.0, 10.0));
  vec3 ambient = vec3(0.15, 0.1, 0.1);
  vec3 diffuse = mix(mix(vec3(0.3, 0.25, 0.2), vec3(0.45, 0.5, 0.5), corrosion),
    vec3(0.5, 0.4, 0.3), shine) - ambient;
  vec3 specular = mix(vec3(0.0), vec3(1.0) - ambient - diffuse, shine);
  float shininess = 128.0;
#else // Debugging curvature
  // View curvature...
  vec3 light = vec3(0.0);
  vec3 ambient = vec3(curvature + 0.5);
  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  float shininess = 0.0;
#endif

  // Compute final color
  float cosAngle = dot(n, light);
  gl_FragColor.rgb = ambient +
    diffuse * max(0.0, cosAngle) +
    specular * pow(max(0.0, cosAngle), shininess);
}

#else // NO TEST SHADER DEFINED

vec4 GetPosAndDepth(vec2 coord)
{
	vec4 pos = texture2D(u_PositionMap, coord);
	pos.a = distance(pos.xyz / u_MapInfo.xyz, u_ViewOrigin / u_MapInfo.xyz);
	return pos;
}

vec2 WorldToScreen( vec3 worldPos )
{
	vec4 hpos = u_ModelViewProjectionMatrix * vec4(worldPos, 1.0);

	/*
	vec2 pos;

	// transform to UV coords
	hpos.w = 0.5 / hpos.z;

	pos.x = 0.5 + hpos.x * hpos.w;
	pos.y = 0.5 + hpos.y * hpos.w;
	*/
	hpos.xyz /= hpos.w;
	hpos.xy = hpos.xy * 0.5 + vec2(0.5);

	return hpos.xy;
}

vec2 mvpPosToScreen( vec4 mvpPos )
{
	vec4 hpos = mvpPos;
	vec2 pos;

	// transform to UV coords
	hpos.w = 0.5 / hpos.z;

	pos.x = 0.5 + hpos.x * hpos.w;
	pos.y = 0.5 + hpos.y * hpos.w;

	return pos;
}

float map( vec4 pos )
{
	//vec2 screenPos = WorldToScreen(pos);
	vec2 screenPos = mvpPosToScreen( pos );
	vec4 pPos = u_ModelViewProjectionMatrix * vec4(GetPosAndDepth(screenPos).xyz, 1.0);
	//vec3 pPos = GetPosAndDepth(screenPos).xyz;
	float d1 = pPos.y;
	float d2 = length(pPos.xyz - vec3(1.0, 0.5, 0.0))-0.5;
	return min(d1, d2);
}

float shadow( vec3 pPos, vec3 light ) 
{
	vec4 p = u_ModelViewProjectionMatrix * vec4(pPos, 1.0);
	vec4 l = u_ModelViewProjectionMatrix * vec4(light, 1.0);
    vec3 dr = normalize(p.xyz - l.xyz);
    float dst = 0.0;
    float res = 1.0;
    for (int i = 0; i < 100; ++i) {
        float dt = map(l);
        l.xyz += dr * dt * 0.8;
        dst += dt * 0.8;
        if (dt < 0.0001) {
            if (distance(l.xyz, p.xyz) < 0.001) {
                return res;
            } else {
            	return 0.0;
            }
        }
        res = min(res, 4.0 * dt * dst / length(p.xyz - l.xyz));
    }
    return res;// * l.w;
}

vec4 positionMapAtCoord ( vec2 coord )
{
	return texture2D(u_PositionMap, coord).xyza;
}

float linearize(float depth)
{
	return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}

/*vec3 applyFog( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir )  // sun light direction
{
    float fogAmount = 1.0 - exp( -distance*b );
    float sunAmount = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );
    return mix( rgb, fogColor, fogAmount );
}*/

vec3 applyFog2( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayOri,   // camera position
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir )  // sun light direction
{
	float b = u_Local0.r; // the falloff of this density
	// be extintion and bi inscattering
	float c = u_Local0.g; // height falloff

    //float fogAmount = c * exp(-rayOri.z*b) * (1.0-exp( -distance*rayDir.z*b ))/rayDir.z;
	float fogAmount = 1.0 - exp( -distance*b );
	fogAmount = clamp(fogAmount, 0.1, u_Local0.a);
	float sunAmount = max( clamp(dot( rayDir, sunDir )*u_Local0.b, 0.0, 1.0), 0.0 );
	if (u_MapInfo.a <= 0.0) sunAmount = 0.0;
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );
	return mix( rgb, fogColor, fogAmount );
}

void main ( void )
{
	gl_FragColor = texture2D(u_DiffuseMap, var_TexCoords);
}

#endif // NO TEST SHADER DEFINED

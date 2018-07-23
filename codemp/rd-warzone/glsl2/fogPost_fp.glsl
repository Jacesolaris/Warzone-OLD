//#define HEIGHT_BASED_FOG

uniform mat4		u_ModelViewProjectionMatrix;

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_WaterMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local2;		// FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_STANDARD_ENABLE
uniform vec4		u_Local3;		// FOG_COLOR_SUN_R, FOG_COLOR_SUN_G, FOG_COLOR_SUN_B, FOG_RANGE_MULTIPLIER
uniform vec4		u_Local4;		// FOG_DENSITY, FOG_VOLUMETRICS, FOG_VOLUMETRIC_SUN_PENETRATION, FOG_VOLUMETRIC_ALTITUDE_BOTTOM
uniform vec4		u_Local5;		// FOG_VOLUMETRIC_COLOR_R, FOG_VOLUMETRIC_COLOR_G, FOG_VOLUMETRIC_COLOR_B, FOG_VOLUMETRIC_ALPHA
uniform vec4		u_Local6;		// MAP_INFO_MAXSIZE, FOG_ACCUMULATION_MODIFIER, FOG_VOLUMETRIC_CLOUDINESS, FOG_VOLUMETRIC_WIND
uniform vec4		u_Local7;		// nightScale, FOG_VOLUMETRIC_ALTITUDE_TOP, FOG_VOLUMETRIC_ALTITUDE_FADE, 0.0
uniform vec4		u_Local8;		// sun color
uniform vec4		u_Local9;		// FOG_VOLUMETRIC_BBOX
uniform vec4		u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform float		u_Time;

varying vec2		var_TexCoords;

vec4 positionMapAtCoord ( vec2 coord )
{
	vec4 pos = textureLod(u_PositionMap, coord, 0.0);
	return pos;
}

//
// Normal fog...
//
vec3 applyLinearFog( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayOri,   // camera position
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir,    // sun light direction
			   in vec4 position )
{
	float b = u_Local4.r; // the falloff of this density

#if defined(HEIGHT_BASED_FOG)
	float c = u_Local0.r; // height falloff

    float fogAmount = c * exp(-rayOri.z*b) * (1.0-exp( -distance*rayDir.z*b ))/rayDir.z; // height based fog
#else //!defined(HEIGHT_BASED_FOG)
	float fogExp = clamp(exp( -pow(distance, u_Local6.g) * b ), 0.0, 1.0);
	float fogAmount = 1.0 - fogExp;
#endif //defined(HEIGHT_BASED_FOG)

	fogAmount = clamp(fogAmount * u_Local3.a, 0.0, 1.0);
	float sunAmount = clamp(max( dot( rayDir, sunDir ), 0.0 ), 0.0, 1.0);
	
	/*if (!(position.a-1.0 == MATERIAL_SKY || position.a-1.0 == MATERIAL_SUN))
	{// Not Skybox or Sun... No don't do sun color here...
		sunAmount = 0.0;
	}*/

	vec3  fogColor  = mix( u_Local2.rgb, // bluish
                           u_Local3.rgb, // yellowish
                           pow(sunAmount,8.0) );

	return mix( rgb, fogColor, fogAmount );
}

//
// Volumetric fog...
//

#define			FOG_VOLUMETRIC_QUALITY				2				// 2 is just fine... Higher looks only slightly better at a much greater FPS cost.

#define			FOG_VOLUMETRIC_SUN_PENETRATION		u_Local4.b

#define			FOG_VOLUMETRIC_ALTITUDE_BOTTOM		u_Local4.a
#define			FOG_VOLUMETRIC_ALTITUDE_TOP			u_Local7.g
#define			FOG_VOLUMETRIC_ALTITUDE_FADE		u_Local7.b

#define			FOG_VOLUMETRIC_BBOX					u_Local9.rgba

#define			FOG_VOLUMETRIC_COLOR				u_Local5.rgb
#define			FOG_VOLUMETRIC_ALPHA				u_Local5.a

#define			FOG_VOLUMETRIC_CLOUDINESS			u_Local6.b
#define			FOG_VOLUMETRIC_WIND					u_Local6.a

#define			FOG_VOLUMETRIC_SUN_COLOR			u_Local8.rgb

float			fogBottom							= FOG_VOLUMETRIC_ALTITUDE_BOTTOM;
float			fogHeight							= FOG_VOLUMETRIC_ALTITUDE_TOP;
float			fadeAltitude						= FOG_VOLUMETRIC_ALTITUDE_FADE;
float			fogThicknessInv						= 1. / (fogHeight - fogBottom);

float			fogWindTime							= u_Time * FOG_VOLUMETRIC_WIND * FOG_VOLUMETRIC_CLOUDINESS * 2000.0;

bool			fogHasBBox							= (FOG_VOLUMETRIC_BBOX[0] == 0.0 && FOG_VOLUMETRIC_BBOX[1] == 0.0 && FOG_VOLUMETRIC_BBOX[2] == 0.0 && FOG_VOLUMETRIC_BBOX[3] == 0.0) ? false : true;

vec3			vAA									= vec3( fogHasBBox ? FOG_VOLUMETRIC_BBOX[0] : -524288.0, fogBottom, fogHasBBox ? FOG_VOLUMETRIC_BBOX[1] : -524288.0 );
vec3			vBB									= vec3( fogHasBBox ? FOG_VOLUMETRIC_BBOX[2] :  524288.0, fogHeight, fogHasBBox ? FOG_VOLUMETRIC_BBOX[3] :  524288.0 );

#define			eyePos								u_ViewOrigin.xzy

vec3			sundir								= normalize(u_ViewOrigin.xzy - u_PrimaryLightOrigin.xzy);
const float		sunDiffuseStrength					= float(6.0);
const float		sunSpecularExponent					= float(100.0);


const mat3 nm = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 ) * 2.02;

struct Ray {
	vec3 Origin;
	vec3 Dir;
};

struct AABB {
	vec3 Min;
	vec3 Max;
};

bool IntersectBox(in Ray r, in AABB aabb, out float t0, out float t1)
{
	vec3 invR = 1.0 / r.Dir;
	vec3 tbot = invR * (aabb.Min - r.Origin);
	vec3 ttop = invR * (aabb.Max - r.Origin);
	vec3 tmin = min(ttop, tbot);
	vec3 tmax = max(ttop, tbot);
	vec2 t = max(tmin.xx, tmin.yz);
	t0 = max(0.,max(t.x, t.y));
	t  = min(tmax.xx, tmax.yz);
	t1 = min(t.x, t.y);
	//return (t0 <= t1) && (t1 >= 0.);
	return (abs(t0) <= t1);
}

float hash( float n ) {
	return fract(sin(n)*43758.5453);
}

float fnoise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}

float noise(in vec3 x)
{
	return fnoise( x / (FOG_VOLUMETRIC_CLOUDINESS * 1000.0) );
}

float MapClouds(in vec3 p)
{
	float factor = 1.0 - smoothstep(fadeAltitude, fogHeight, p.y);

	vec3 pos = p;//normalize(p);

	pos += fogWindTime * 0.07;

	float f = noise( pos );
	pos = m*pos - fogWindTime * 0.3;
	f += 0.25 * noise( pos );
	pos = m*pos - fogWindTime * 0.07;
	f += 0.1250 * noise( pos );
	pos = m*pos + fogWindTime * 0.8;
	f += 0.0625 * noise( pos );

    f = mix(0.0, f, factor);

	return f;
}

vec4 RaymarchClouds(in vec3 start, in vec3 end)
{
	float l = length(end - start);
	const float numsteps = FOG_VOLUMETRIC_QUALITY;//20.0;
	const float tstep = 1. / numsteps;
	float depth = min(l * fogThicknessInv, 1.5);

	float fogContrib = 0.;
	float sunContrib = 0.;
	float alpha = 0.;

	for (float t=0.0; t<=1.0; t+=tstep) 
	{
		vec3  pos = mix(start, end, t);
		float fog = MapClouds(pos);
		fogContrib += fog;

		vec3  lightPos = sundir * FOG_VOLUMETRIC_SUN_PENETRATION + pos;
		float lightFog = MapClouds(lightPos);
		float sunVisibility = clamp((fog - lightFog), 0.0, 1.0 ) * sunDiffuseStrength;
		sunContrib += sunVisibility;

		float b = smoothstep(1.0, 0.7, abs((t - 0.5) * 2.0));
		alpha += b;
	}

	fogContrib *= tstep;
	sunContrib *= tstep;
	alpha      *= tstep * FOG_VOLUMETRIC_ALPHA * depth;

	vec3 ndir = (end - start) / l;
	float sun = pow( clamp( dot(sundir, ndir), 0.0, 1.0 ), sunSpecularExponent );
	sunContrib += sun * clamp(1. - fogContrib * alpha, 0.2, 1.) * 1.0;

	vec4 col;
	col.rgb = sunContrib * FOG_VOLUMETRIC_SUN_COLOR + FOG_VOLUMETRIC_COLOR;
	col.a   = fogContrib * clamp(pow(alpha, 0.001), 0.0, FOG_VOLUMETRIC_ALPHA);
	return col;
}

//
// Shared...
//
void main ( void )
{
	vec3 col = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
	vec3 fogColor = col;

	if (u_Local7.r < 1.0 && (u_Local2.a > 0.0 || u_Local4.g > 0.0))
	{// At night no point thinking about fogs... For now... Sky doesn't like it much at night transition (sun angles, etc)...
		vec4 pMap = positionMapAtCoord( var_TexCoords );
		vec3 viewOrg = u_ViewOrigin.xyz;

		//
		// Linear fog...
		//
		if (u_Local2.a > 0.0)
		{
			vec3 rayDir = normalize(viewOrg.xyz - pMap.xyz);
			vec3 lightDir = normalize(viewOrg.xyz - u_PrimaryLightOrigin.xyz);
			float depth = textureLod(u_ScreenDepthMap, var_TexCoords, 0.0).r;
			fogColor = applyLinearFog(fogColor.rgb, depth, viewOrg.xyz, rayDir, lightDir, pMap);
		}

		//
		// Volumetric fog...
		//
		if (u_Local4.g > 0.0)
		{
			vec3 worldPos = pMap.xzy;

			// clamp ray in boundary box
			Ray r;
			r.Origin = eyePos;
			r.Dir = worldPos - eyePos;
			AABB box;
			box.Min = vAA;
			box.Max = vBB;
			float t1, t2;
	
			if (IntersectBox(r, box, t1, t2)) 
			{
				t1 = clamp(t1, 0.0, 1.0);
				t2 = clamp(t2, 0.0, 1.0);
				vec3 startPos = r.Dir * t1 + r.Origin;
				vec3 endPos   = r.Dir * t2 + r.Origin;

				// finally raymarch the volume
				vec4 vFog = RaymarchClouds(startPos, endPos);

				// blend with distance to make endless fog have smooth horizon
				//fogColor = mix(fogColor, clamp(fogColor + vFog.rgb, 0.0, 1.0), smoothstep(gl_Fog.end * 10.0, gl_Fog.start, length(worldPos - eyePos)));
				fogColor = mix(fogColor, clamp(fogColor + vFog.rgb, 0.0, 1.0), vFog.a);
			}
		}

		// Blend out fog as we head more to night time... For now... Sky doesn't like it much at night transition (sun angles, etc)...
		fogColor.rgb = mix(clamp(fogColor.rgb, 0.0, 1.0), col.rgb, u_Local7.r);
	}

	gl_FragColor = vec4(fogColor, 1.0);
}

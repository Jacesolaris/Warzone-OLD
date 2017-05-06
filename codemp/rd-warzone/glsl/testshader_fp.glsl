uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_SpecularMap;

uniform vec4		u_ViewInfo;
uniform vec2		u_Dimensions;
uniform vec4		u_Local0; // r_testvalue's
uniform vec4		u_Local1; // r_testshadervalue's


uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

// Position of the camera
uniform vec3		u_ViewOrigin;
uniform float		u_Time;

varying vec2		var_TexCoords;




//vec3				offset = vec3(0.0001, 0.0001, 0.0001);
const vec3			offset = vec3(0.0, 0.0, 0.0);
//#define				offset u_Local0.gba



#define eyePos		(u_ViewOrigin.xzy / 64.0)
#define sundir		normalize((u_PrimaryLightOrigin.xzy / 64.0) - eyePos)
#define suncolor	u_PrimaryLightColor


const float noiseScale = 1. / float(2.0);
//float noiseScale = 1. / float(u_Local0.r);
/*const*/ float fogBottom = float(u_Local1.g) / 64.0;
/*const*/ float fogHeight = float(fogBottom) + float(u_Local1.b / 64.0);
/*const*/ float fadeAltitude = float(fogBottom) + float(u_Local1.a / 64.0);
/*const*/ float fogThicknessInv = 1. / (fogHeight - fogBottom);
const vec3 fogColor   = vec3(1.0, 1.0, 1.0);
/*const*/ float opacity = float(u_Local1.r/*0.5*/);

const float sunPenetrationDepth = float(0.5); 
const float sunDiffuseStrength = float(6.0);
const float noiseTexSizeInv = 1.0 / 2048.0;
const float noiseCloudness = float(0.35); // TODO: configurable


vec2 fvTexelSize = vec2(1.0) / u_Dimensions.xy;

// 'threshold ' is constant , 'value ' is smoothly varying
float aastep ( float threshold , float value )
{
	float afwidth = 0.7 * length ( vec2 ( dFdx ( value ) , dFdy ( value ))) ;
	// GLSL 's fwidth ( value ) is abs ( dFdx ( value )) + abs ( dFdy ( value ))
	return smoothstep ( threshold - afwidth , threshold + afwidth , value ) ;
}

	/*const*/ vec3 vAA = vec3(-300000.0, fogBottom, -300000.0);
	/*const*/ vec3 vBB = vec3( 300000.0, fogHeight,  300000.0);


const float sunSpecularExponent = float(100.0);

float noise(in vec3 x)
{
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xz + vec2(37.0,17.0)*p.y) + f.xz;
	vec2 rg = texture2D( u_SpecularMap, (uv + 0.5) * noiseTexSizeInv).yx;
	return smoothstep(0.5 - noiseCloudness, 0.5 + noiseCloudness, mix( rg.x, rg.y, f.y ));
}


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


const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 ) * 2.02;

float MapClouds(in vec3 p)
{
	float factor = 1.0-smoothstep(fadeAltitude,fogHeight,p.y);

	p += offset;
	p *= noiseScale;
	p += u_Time * 0.07;

	float f = noise( p );
	p = m*p - u_Time * 0.3;
	f += 0.25 * noise( p );
	p = m*p - u_Time * 0.07;
	f += 0.1250 * noise( p );
	p = m*p + u_Time * 0.8;
	f += 0.0625 * noise( p );

    f = mix(0.0,f,factor);

	return f;
}


vec4 RaymarchClouds(in vec3 start, in vec3 end)
{
	float l = length(end - start);
	const float numsteps = 5.0;//20.0;
	//float numsteps = u_Local0.r;
	const float tstep = 1. / numsteps;
	float depth = min(l * fogThicknessInv, 1.5);

	float fogContrib = 0.;
	float sunContrib = 0.;
	float alpha = 0.;

	for (float t=0.0; t<=1.0; t+=tstep) {
		vec3  pos = mix(start, end, t);
		float fog = MapClouds(pos);
		fogContrib += fog;

		vec3  lightPos = sundir * sunPenetrationDepth + pos;
		float lightFog = MapClouds(lightPos);
		float sunVisibility = clamp((fog - lightFog), 0.0, 1.0 ) * sunDiffuseStrength;
		sunContrib += sunVisibility;

		float b = smoothstep(1.0, 0.7, abs((t - 0.5) * 2.0));
		alpha += b;
	}

	fogContrib *= tstep;
	sunContrib *= tstep;
	alpha      *= tstep * opacity * depth;

	vec3 ndir = (end - start) / l;
	float sun = pow( clamp( dot(sundir, ndir), 0.0, 1.0 ), sunSpecularExponent );
	sunContrib += sun * clamp(1. - fogContrib * alpha, 0.2, 1.) * 1.0;

	vec4 col;
	col.rgb = sunContrib * suncolor + fogColor;
	col.a   = fogContrib * alpha;
	return col;
}

vec3 GetWorldPos ( vec2 screenpos )
{
	return textureLod(u_PositionMap, screenpos, 0.0).xzy / 64.0;
}

void main()
{
	vec4 color = texture(u_DiffuseMap, var_TexCoords);

	// reconstruct worldpos from depthbuffer
	vec3 worldPos = GetWorldPos(var_TexCoords);

	// clamp ray in boundary box
	Ray r;
	r.Origin = eyePos;
	r.Dir = worldPos - eyePos;
	AABB box;
	box.Min = vAA;
	box.Max = vBB;
	float t1, t2;
	
	// TODO: find a way to do this when eye is inside volume
	if (!IntersectBox(r, box, t1, t2)) {
		gl_FragColor = color;
		return;
	}
	t1 = clamp(t1, 0.0, 1.0);
	t2 = clamp(t2, 0.0, 1.0);
	vec3 startPos = r.Dir * t1 + r.Origin;
	vec3 endPos   = r.Dir * t2 + r.Origin;

	// finally raymarch the volume
	vec4 fogColor = RaymarchClouds(startPos, endPos);
	gl_FragColor.rgb = mix(color.rgb, fogColor.rgb, pow(fogColor.a, 3.0));
	gl_FragColor.a = 1.0;

	// blend with distance to make endless fog have smooth horizon
	//gl_FragColor.a *= smoothstep(gl_Fog.end * 10.0, gl_Fog.start, length(worldPos - eyePos));
}


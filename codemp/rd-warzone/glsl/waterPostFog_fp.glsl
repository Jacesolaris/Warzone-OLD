uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_WaterPositionMap;
uniform sampler2D	u_ScreenDepthMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

#define FOG_VOLUMETRIC_CLOUDINESS	1.0		//u_Local0.r
#define FOG_VOLUMETRIC_WIND			0.001	//u_Local0.g
#define FOG_VOLUMETRIC_STRENGTH		8.0		//u_Local0.b
#define FOG_VOLUMETRIC_DENSITY		0.3		//u_Local0.a

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform float		u_Time;

varying vec2		var_TexCoords;

vec4 positionMapAtCoord ( vec2 coord )
{
	return textureLod(u_PositionMap, coord, 0.0);
}

vec4 waterMapAtCoord ( vec2 coord )
{
	return textureLod(u_WaterPositionMap, coord, 0.0);
}

float getDepth(vec2 coord)
{
	float depth = textureLod(u_ScreenDepthMap, coord, 0.0).r;
	return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}

//
// Volumetric fog...
//
float tri(in float x){return abs(fract(x)-.5);}
vec3 tri3(in vec3 p){return vec3( tri(p.z+tri(p.y*1.)), tri(p.z+tri(p.x*1.)), tri(p.y+tri(p.x*1.)));}

float triNoise3d(vec3 p, float spd)
{
    float z=1.4;
	float rz = 0.0;
    vec3 bp = p;

	for (float i=0.; i<=3.; i++ )
	{
        vec3 dg = tri3(bp*2.);
        p += (dg+u_Time*spd);

        bp *= 1.8;
		z *= 1.5;
		p *= 1.2;
        
        rz+= (tri(p.z+tri(p.x+tri(p.y))))/z;
        bp += 0.14;
	}
	return rz;
}

float fogmap(vec3 p, float d)
{
	p.xyz *= (100.0 * FOG_VOLUMETRIC_CLOUDINESS);
    p.x += u_Time * FOG_VOLUMETRIC_WIND;
    p.z += sin(p.x*.5);
    return triNoise3d(p*2.2/(d+20.),0.2)*(1.-smoothstep(0.,.7,p.y));
}

//
// Shared...
//
void main ( void )
{
	vec3 fogColor = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;
	vec4 wMap = waterMapAtCoord( var_TexCoords );
	vec4 pMap = positionMapAtCoord( var_TexCoords );

	float dw = distance(wMap.xyz, u_ViewOrigin.xyz);
	float dp = distance(pMap.xyz, u_ViewOrigin.xyz);

	bool isWater = false;
	bool isWaterFall = false;

	if (wMap.a >= 1.0 && (dw <= dp || (pMap.x == 0.0 && pMap.y == 0.0 && pMap.z == 0.0 || pMap.a == 0.0 || pMap.a == 1024.0 || pMap.a == 1025.0)))
	{
		isWater = true;

		if (wMap.a >= 2.0)
		{
			isWaterFall = true;
		}
	}

	float distMult = clamp(length(dw-dp) / 32.0, 0.0, 1.0);
	distMult = pow(distMult, 2.5);
	distMult = clamp(distMult, 0.0, 1.0);

	//
	// Volumetric fog...
	//
	if (isWater /*|| isWaterFall*/)
	{
		vec3 rayDir = normalize(u_ViewOrigin.xzy - pMap.xzy);
		vec3 fog = vec3(0.0);
		float dafuck = 0.5;
		float numAdded = 0.0;
		float mt = FOG_VOLUMETRIC_DENSITY;

#pragma unroll 7
		for(int i=0; i<7; i++)
		{
			vec3 pos = rayDir*dafuck;
			float rz = fogmap(pos, dafuck);

			float grd =  clamp((rz - fogmap(pos+0.8-float(i)*0.1,dafuck))*3.0, 0.1, 1.0 );
			vec3 col2 = clamp(vec3(1.0) * (1.7-grd), 0.0, 1.0) * FOG_VOLUMETRIC_STRENGTH;
			fog = mix(fog,col2,clamp(rz*smoothstep(dafuck-0.4,dafuck+2.0+dafuck*0.75,mt),0.0,1.0) );
			dafuck *= 0.45;
			numAdded += 1.0;
			if (dafuck>mt)break;
		}

		vec3 fogPower = clamp((fog / numAdded) * 5.0, 0.0, 1.0);
		fogColor = mix(fogColor, clamp(fogColor + fogPower, 0.0, 1.0), distMult * (length(fogPower) / 3.0));
		
	}

	gl_FragColor = vec4(fogColor, 1.0);
}

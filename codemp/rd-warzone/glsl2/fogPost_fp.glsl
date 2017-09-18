//#define HEIGHT_BASED_FOG

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_WaterMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_Local2;		// FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, FOG_STANDARD_ENABLE
uniform vec4		u_Local3;		// FOG_COLOR_SUN_R, FOG_COLOR_SUN_G, FOG_COLOR_SUN_B, FOG_RANGE_MULTIPLIER
uniform vec4		u_Local4;		// FOG_DENSITY, FOG_VOLUMETRICS, FOG_VOLUMETRIC_DENSITY, FOG_VOLUMETRIC_VELOCITY
uniform vec4		u_Local5;		// FOG_VOLUMETRIC_COLOR_R, FOG_VOLUMETRIC_COLOR_G, FOG_VOLUMETRIC_COLOR_B, FOG_VOLUMETRIC_STRENGTH
uniform vec4		u_Local6;		// MAP_INFO_MAXSIZE, FOG_ACCUMULATION_MODIFIER, FOG_VOLUMETRIC_CLOUDINESS, FOG_VOLUMETRIC_WIND
uniform vec4		u_Local7;		// nightScale, 0.0, 0.0, 0.0
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

float linearize(float depth)
{
	return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}

//
// Volumetric fog...
//
float tri(in float x){return abs(fract(x)-.5);}
vec3 tri3(in vec3 p){return vec3( tri(p.z+tri(p.y*1.)), tri(p.z+tri(p.x*1.)), tri(p.y+tri(p.x*1.)));}
                                 
//mat2 m2 = mat2(0.970,  0.242, -0.242,  0.970);

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
        //p.xz*= m2;
        
        rz+= (tri(p.z+tri(p.x+tri(p.y))))/z;
        bp += 0.14;
	}
	return rz;
}

float fogmap(vec3 p, float d)
{
	p.xyz *= (100.0 * u_Local6.b);
    p.x += u_Time * u_Local6.a;
    p.z += sin(p.x*.5);
    return triNoise3d(p*2.2/(d+20.),0.2)*(1.-smoothstep(0.,.7,p.y));
}


//
// Normal fog...
//
vec3 applyFog2( in vec3  rgb,      // original color of the pixel
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

	fogAmount = clamp(fogAmount * u_Local3.a, 0.1, 1.0);
	float sunAmount = max( clamp(dot( rayDir, sunDir ) * 1.1, 0.0, 1.0), 0.0 );
	
	if (!(position.a-1.0 == MATERIAL_SKY || position.a-1.0 == MATERIAL_SUN))
	{// Not Skybox or Sun... No don't do sun color here...
		sunAmount = 0.0;
	}

	vec3  fogColor  = mix( u_Local2.rgb, // bluish
                           u_Local3.rgb, // yellowish
                           pow(sunAmount,8.0) );

	return mix( rgb, fogColor, fogAmount );
}

//
// Shared...
//
void main ( void )
{
	vec3 col = textureLod(u_DiffuseMap, var_TexCoords, 0.0).rgb;

	if (u_Local7.r >= 1.0)
	{// At night no point thinking about fogs... For now... Sky doesn't like it much at night transition (sun angles, etc)...
		gl_FragColor = vec4(col, 1.0);
		return;
	}

	vec4 pMap = positionMapAtCoord( var_TexCoords );
	vec3 viewOrg = u_ViewOrigin.xyz;
	vec3 fogColor = col;

	//
	// Normal fog...
	//
	if (u_Local2.a > 0.0)
	{
		vec3 rayDir = -gl_FragCoord.xyz;//normalize(viewOrg.xyz - pMap.xyz);
		vec3 lightDir = normalize(viewOrg.xyz - u_PrimaryLightOrigin.xyz);
		float depth = linearize(textureLod(u_ScreenDepthMap, var_TexCoords, 0.0).r);
		fogColor = applyFog2(fogColor.rgb, depth, viewOrg.xyz, rayDir, lightDir, pMap);
	}

	//
	// Volumetric fog...
	//
	if (u_Local4.g > 0.0)
	{
		vec3 pM = pMap.xzy;
		vec3 vO = vec3(0.0, -viewOrg.z, 0.0);
		pM.z += 524288.0;
		vO.z += 524288.0;
		vec3 rayDir = normalize(vO - pM);
		vec3 fog = vec3(0.0);
		float dafuck = 0.5;
		float numAdded = 0.0;
		float mt = u_Local4.b;

//#pragma unroll 7
		for(int i=0; i<7; i++)
		{
			vec3 pos = rayDir*dafuck;
			float rz = fogmap(pos, dafuck);

			float grd =  clamp((rz - fogmap(pos+0.8-float(i)*0.1,dafuck))*3.0, 0.1, 1.0 );
			vec3 col2 = clamp(u_Local5.rgb * (1.7-grd), 0.0, 1.0) * u_Local5.a;
			fog = mix(fog,col2,clamp(rz*smoothstep(dafuck-0.4,dafuck+2.0+dafuck*0.75,mt),0.0,1.0) );
			dafuck *= 0.45;
			numAdded += 1.0;
			if (dafuck>mt)break;
		}

		fogColor += (fog / numAdded) * 5.0;
	}

	// Blend out fog as we head more to night time... For now... Sky doesn't like it much at night transition (sun angles, etc)...
	fogColor.rgb = mix(fogColor.rgb, col.rgb, u_Local7.r);

	gl_FragColor = vec4(fogColor, 1.0);
}

//#define HEIGHT_BASED_FOG
//#define VFOG

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_ScreenDepthMap;
uniform sampler2D	u_PositionMap;

uniform vec4		u_ViewInfo; // zmin, zmax, zmax / zmin
uniform vec2		u_Dimensions;

uniform vec4		u_Local0;		// testvalue0, testvalue1, testvalue2, testvalue3
uniform vec4		u_MapInfo;		// MAP_INFO_SIZE[0], MAP_INFO_SIZE[1], MAP_INFO_SIZE[2], SUN_VISIBLE

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform float		u_Time;

varying vec2		var_TexCoords;

vec4 positionMapAtCoord ( vec2 coord )
{
	return texture2D(u_PositionMap, coord).xyzw;
}

float linearize(float depth)
{
	return clamp(1.0 / mix(u_ViewInfo.z, 1.0, depth), 0.0, 1.0);
}

#ifdef VFOG
#define time u_Time

float tri(in float x){return abs(fract(x)-.5);}
vec3 tri3(in vec3 p){return vec3( tri(p.z+tri(p.y*1.)), tri(p.z+tri(p.x*1.)), tri(p.y+tri(p.x*1.)));}
                                 
mat2 m2 = mat2(0.970,  0.242, -0.242,  0.970);

float triNoise3d(vec3 p, float spd)
{
    float z=1.4;
	float rz = 0.;
    vec3 bp = p;
	for (float i=0.; i<=3.; i++ )
	{
        vec3 dg = tri3(bp*2.);
        p += (dg+time*spd);

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
    p.x += time*1.5;
    p.z += sin(p.x*.5);
    return triNoise3d(p*2.2/(d+20.),0.2)*(1.-smoothstep(0.,.7,p.y));
}

#else //!VFOG

vec3 applyFog2( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayOri,   // camera position
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir,
			   in vec4 position )  // sun light direction
{
	const float b = 0.5;//0.7;//u_Local0.r; // the falloff of this density

#if defined(HEIGHT_BASED_FOG)
	float c = u_Local0.g; // height falloff

    float fogAmount = c * exp(-rayOri.z*b) * (1.0-exp( -distance*rayDir.z*b ))/rayDir.z; // height based fog
#else //!defined(HEIGHT_BASED_FOG)
	float fogAmount = 1.0 - exp( -distance*b );
#endif //defined(HEIGHT_BASED_FOG)

	fogAmount = clamp(fogAmount, 0.1, 1.0/*u_Local0.a*/);
	float sunAmount = max( clamp(dot( rayDir, sunDir )*1.1, 0.0, 1.0), 0.0 );
	
	//if (u_MapInfo.a <= 0.0) sunAmount = 0.0;
	if (!(position.a == 1024.0 || position.a == 1025.0))
	{// Not Skybox or Sun... No don't do sun color here...
		sunAmount = 0.0;
	}

    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );

	return mix( rgb, fogColor, fogAmount );
}
#endif //VFOG

void main ( void )
{
#ifdef VFOG
	vec3 col = texture2D(u_DiffuseMap, var_TexCoords).rgb;
	vec4 pMap = positionMapAtCoord( var_TexCoords );
	vec3 viewOrg = u_ViewOrigin.xyz;
	vec3 rayDir = normalize(viewOrg.xyz - pMap.xyz);
	float mt = u_Local0.r;

	float dafuck = 0.5;
    for(int i=0; i<7; i++)
    {
        vec3 pos = rayDir*dafuck;
        float rz = fogmap(pos, dafuck);

		float grd =  clamp((rz - fogmap(pos+0.8-float(i)*0.1,dafuck))*3.0, 0.1, 1.0 );
        vec3 col2 = (vec3(0.1,0.8,0.5)*0.5 + 0.5*vec3(0.5, 0.8, 1.0)*(1.7-grd))*0.55;
        col = mix(col,col2,clamp(rz*smoothstep(dafuck-0.4,dafuck+2.0+dafuck*0.75,mt),0.0,1.0) );
        dafuck *= 1.5+0.3;
        if (dafuck>mt)break;
    }

	vec3 fogColor = col;
#else //!VFOG
	vec4 pMap = positionMapAtCoord( var_TexCoords );
	vec4 pixelColor = texture2D(u_DiffuseMap, var_TexCoords);
	float depth = linearize(texture2D(u_ScreenDepthMap, var_TexCoords).r);
	vec3 viewOrg = u_ViewOrigin.xyz;
	vec3 sunOrg = u_PrimaryLightOrigin.xyz;

	vec3 fogColor = applyFog2(pixelColor.rgb, depth, viewOrg.xyz/*pMap.xyz*/, normalize(viewOrg.xyz - pMap.xyz), normalize(viewOrg.xyz - sunOrg.xyz), pMap);
#endif //VFOG
	
	gl_FragColor = vec4(fogColor, 1.0);
}

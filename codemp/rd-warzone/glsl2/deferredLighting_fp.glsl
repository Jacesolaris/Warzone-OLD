//#define __CUBEMAP__
//#define __SSR__
//#define __AMBIENT_OCCLUSION__
//#define __BUMP__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;
uniform samplerCube	u_CubeMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif //defined(USE_SHADOWMAP)

uniform vec2		u_Dimensions;

uniform vec4		u_Local2; // r_blinnPhong, SHADOWS_ENABLED, r_shadowContrast, 0.0
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

#define MAX_LIGHTALL_DLIGHTS 64//16//24

uniform int			u_lightCount;
uniform vec2		u_lightPositions[MAX_LIGHTALL_DLIGHTS];
uniform vec3		u_lightPositions2[MAX_LIGHTALL_DLIGHTS];
uniform float		u_lightDistances[MAX_LIGHTALL_DLIGHTS];
uniform vec3		u_lightColors[MAX_LIGHTALL_DLIGHTS];

varying vec2		var_TexCoords;

#define textureCubeLod textureLod // UQ1: > ver 140 support


#define LIGHT_THRESHOLD  0.001


vec2 pixel = vec2(1.0) / u_Dimensions;


vec2 encode (vec3 n)
{
    float p = sqrt(n.z*8+8);
    return vec2(n.xy/p + 0.5);
}

vec3 decode (vec2 enc)
{
    vec2 fenc = enc*4-2;
    float f = dot(fenc,fenc);
    float g = sqrt(1-f/4);
    vec3 n;
    n.xy = fenc*g;
    n.z = 1-f/2;
    return n;
}

#ifdef __AMBIENT_OCCLUSION__
float drawObject(in vec3 p){
    p = abs(fract(p)-.5);
    return dot(p, vec3(.5));
}

float cellTile(in vec3 p){
    p /= 5.5;
    // Draw four overlapping objects at various positions throughout the tile.
    vec4 v, d; 
    d.x = drawObject(p - vec3(.81, .62, .53));
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.y = drawObject(p - vec3(.39, .2, .11));
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawObject(p - vec3(.62, .24, .06));
    p.xz = vec2(p.z-p.x, p.z + p.x)*.7071;
    d.w = drawObject(p - vec3(.2, .82, .64));

    v.xy = min(d.xz, d.yw), v.z = min(max(d.x, d.y), max(d.z, d.w)), v.w = max(v.x, v.y); 
   
    d.x =  min(v.z, v.w) - min(v.x, v.y); // Maximum minus second order, for that beveled Voronoi look. Range [0, 1].
    //d.x =  min(v.x, v.y); // First order.
        
    return d.x*2.66; // Normalize... roughly.
}

float map(vec3 p){
    float n = (.5-cellTile(p))*1.5;
    return p.y + dot(sin(p/2. + cos(p.yzx/2. + 3.14159/2.)), vec3(.5)) + n;
}

float calculateAO(in vec3 pos, in vec3 nor)
{
	float sca = 2.0, occ = 0.0;
    for( int i=0; i<5; i++ ){
    
        float hr = 0.01 + float(i)*0.5/4.0;        
        float dd = map(nor * hr + pos);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - occ, 0.0, 1.0 );    
}
#endif //__AMBIENT_OCCLUSION__

//
// Full lighting... Blinn phong and basic lighting as well...
//

// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor, vec3 specularColor) {
	vec3 halfLV = normalize(light + view);
	float spe = pow(max(dot(normal, halfLV), 0.0), 32.0);
	float dif = dot(normal, light) * 0.5 + 0.75;
	return dif*diffuseColor + spe*specularColor;
}

#ifdef __CUBEMAP__
vec3 EnvironmentBRDF(float gloss, float NE, vec3 specular)
{
	vec4 t = vec4( 1/0.96, 0.475, (0.0275 - 0.25 * 0.04)/0.96,0.25 ) * gloss;
	t += vec4( 0.0, 0.0, (0.015 - 0.75 * 0.04)/0.96,0.75 );
	float a0 = t.x * min( t.y, exp2( -9.28 * NE ) ) + t.z;
	float a1 = t.w;
	return clamp( a0 + specular * ( a1 - a0 ), 0.0, 1.0 );
}
#endif //__CUBEMAP__

#ifdef __SSR__
float pw = pixel.x;
float ph = pixel.y;

vec3 AddReflection(vec2 coord, vec4 positionMap, vec3 inColor, float reflectStrength)
{
	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y < 1.0; y += ph * 50.0)
	{
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);

		bool isSameMaterial = false;
		
		if (positionMap.a == pMap.a)
			isSameMaterial = true;

		if (!isSameMaterial)
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0)
	{// Found no non-water surfaces...
		return inColor;
	}
	
	QLAND_Y -= ph * 50.0;

	for (float y = coord.y; y < 1.0; y += ph * 5.0)
	{
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);

		bool isSameMaterial = false;
		
		if (positionMap.a == pMap.a)
			isSameMaterial = true;

		if (!isSameMaterial)
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0)
	{// Found no non-water surfaces...
		return inColor;
	}
	
	QLAND_Y -= ph * 5.0;
	
	// Full scan from within 5 px for the real 1st pixel...
	float upPos = coord.y;
	float LAND_Y = 0.0;

	for (float y = QLAND_Y; y < 1.0; y += ph)
	{
		vec4 pMap = textureLod(u_PositionMap, vec2(coord.x, y), 0.0);

		bool isSameMaterial = false;

		if (positionMap.a == pMap.a)
			isSameMaterial = true;

		if (!isSameMaterial)
		{
			LAND_Y = y;
			break;
		}
	}

	if (LAND_Y <= 0.0)
	{// Found no non-water surfaces...
		return inColor;
	}

	upPos = clamp(coord.y + ((LAND_Y - coord.y) * 2.0), 0.0, 1.0);

	if (upPos > 1.0 || upPos < 0.0)
	{// Not on screen...
		return inColor;
	}

	//vec4 wMap = waterMapAtCoord(vec2(coord.x, upPos));

	//if (wMap.a > 0.0)
	//{// This position is water, or it is closer then the reflection pixel...
	//	return inColor;
	//}

	vec4 landColor = textureLod(u_DiffuseMap, vec2(coord.x, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos + ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x + pw, upPos - ph), 0.0);
	landColor += textureLod(u_DiffuseMap, vec2(coord.x - pw, upPos + ph), 0.0);
	landColor /= 9.0;

	return mix(inColor.rgb, landColor.rgb, vec3(1.0 - pow(upPos, 4.0)) * reflectStrength/*0.28*//*u_Local0.r*/);
}
#endif //__SSR__

#ifdef __BUMP__
vec3 doBump( in vec3 pos, in vec3 nor, in float signal, in float scale )
{
    // build frame	
    vec3  s = dFdx( pos );
    vec3  t = dFdy( pos );
    vec3  u = cross( t, nor );
    vec3  v = cross( nor, s );
    float d = dot( s, u );

    // compute bump	
    float bs = dFdx( signal );
    float bt = dFdy( signal );
	
    // offset normal	
#if 1
    return normalize( nor - scale*(bs*u + bt*v)/d );
#else
    // if you cannot ensure the frame is not null	
    vec3 vSurfGrad = sign( d ) * ( bs * u + bt * v );
    return normalize( abs(d)*nor - scale*vSurfGrad );
#endif
}
#endif //__BUMP__

void main(void)
{
	vec4 color = textureLod(u_DiffuseMap, var_TexCoords, 0.0);
	gl_FragColor = vec4(color.rgb, 1.0);

	vec2 texCoords = var_TexCoords;

#if 0 // Debugging stuff
	if (u_Local3.g >= 2.0)
	{
		vec4 position = textureLod(u_PositionMap, texCoords, 0.0);

		if (u_Local3.g >= 4.0)
			gl_FragColor.rgb = vec3((position.rg / u_Local3.r), 0.0) * 0.5 + 0.5;
		else if (u_Local3.g >= 3.0)
			gl_FragColor.rgb = vec3(position.b / u_Local3.r) * 0.5 + 0.5;
		else
			gl_FragColor.rgb = vec3(position.rgb / u_Local3.r) * 0.5 + 0.5;
		return;
	}

	if (u_Local3.g >= 1.0)
	{
		vec4 position = textureLod(u_PositionMap, texCoords, 0.0);

		float dist = distance(position.xyz, u_ViewOrigin.xyz);
		dist = clamp(dist / 4096.0, 0.0, 1.0);
		gl_FragColor.rgb = vec3(dist);
		return;
	}
#endif

	vec3 viewOrg = u_ViewOrigin.xyz;
	vec4 position = textureLod(u_PositionMap, texCoords, 0.0);

	if (position.a == 1024.0 || position.a == 1025.0)
	{// Skybox... Skip...
		//gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
		return;
	}

	/*if (position.a == 0.0 && position.xyz == vec3(0.0))
	{// Unknown... Skip...
		//gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), 1.0);
		return;
	}*/

	vec4 norm = textureLod(u_NormalMap, texCoords, 0.0);
	norm.xyz = decode(norm.xy);

#ifdef __CUBEMAP__
	vec3 unpackedCubeStuff = decode(norm.zw);
	float enableCubeMap = unpackedCubeStuff.x;
	float cubeStrength = clamp(unpackedCubeStuff.y * 10.0, 0.0, 1.0);
	float specularScale = clamp(unpackedCubeStuff.z * 10.0, 0.0, 1.0);
#endif //__CUBEMAP__

	/*if (position.a != 0.0 && position.a != 1024.0 && position.a != 1025.0 && norm.a == 0.05)
	{// Generic GLSL. Probably a glow or something, ignore the lighting...
		return;
	}*/

	float displacement = clamp(length(color.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	norm.a = clamp((clamp(displacement - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

	norm.a = norm.a * 0.5 + 0.5;
	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	vec3 N = norm.xyz;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);

	//gl_FragColor = vec4(N.xyz * 0.5 + 0.5, 1.0);
	//return;

#ifdef __BUMP__
	//vec3 N2 = N;
#define N2 N

	if (u_Local3.r != 0.0)
	{
		N2.xyz = doBump( position.xyz/*E*/, N.xyz, dot(gl_FragColor.rgb,vec3(0.33)), u_Local3.r );
	}

	if (u_Local3.g != 0.0)
	{
		gl_FragColor.rgb = N2.xyz * 0.5 + 0.5;
		return;
	}

	float occ = 0.5 + 0.5*N2.y;
	occ = 1.0 - clamp(occ * 0.75, 0.0, 1.0);
	//float occ = 1.0;

	if (u_Local3.b != 0.0)
	{
		gl_FragColor.rgb = vec3(occ);
		return;
	}
#endif //__BUMP__

#if defined(USE_SHADOWMAP)
	if (u_Local2.g > 0.0)
	{
		float shadowValue = 0.0;

#ifdef HIGH_QUALITY_SHADOWS
		for (float y = -3.5 ; y <=3.5 ; y+=1.0)
			for (float x = -3.5 ; x <=3.5 ; x+=1.0)
				shadowValue += textureLod(u_ShadowMap, texCoords + (vec2(x, y) * pixel * 3.0), 0.0).r;

		shadowValue /= 64.0;
#else //!HIGH_QUALITY_SHADOWS
		for (float y = -1.75 ; y <=1.75 ; y+=1.0)
			for (float x = -1.75 ; x <=1.75 ; x+=1.0)
				shadowValue += textureLod(u_ShadowMap, texCoords + (vec2(x, y) * pixel * 3.0), 0.0).r;

		shadowValue /= 32.0;
#endif //HIGH_QUALITY_SHADOWS

		gl_FragColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, 1.3);
	}
#endif //defined(USE_SHADOWMAP)

	vec3 PrimaryLightDir = normalize(u_PrimaryLightOrigin.xyz - position.xyz);
	float lambertian2 = dot(PrimaryLightDir.xyz, N);
	float spec2 = 0.0;
	bool noSunPhong = false;
	float phongFactor = u_Local2.r;

	if (phongFactor < 0.0)
	{// Negative phong value is used to tell terrains not to use sunlight (to hide the triangle edge differences)
		noSunPhong = true;
		phongFactor = 0.0;
	}

	if (!noSunPhong && lambertian2 > 0.0)
	{// this is blinn phong
		vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
		float specAngle = max(dot(halfDir2, N), 0.0);
		spec2 = pow(specAngle, 16.0);
		gl_FragColor.rgb += vec3(spec2 * (norm.a)) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor;
	}

	if (noSunPhong)
	{// Invert phong value so we still have non-sun lights...
		phongFactor = -u_Local2.r;
	}

	if (u_lightCount > 0.0)
	{
		vec3 addedLight = vec3(0.0);

		for (int li = 0; li < MAX_LIGHTALL_DLIGHTS; li++)
		{
			if (li > u_lightCount) break;

			vec3 lightPos = u_lightPositions2[li].xyz;

			float lightDist = distance(lightPos, position.xyz);
			float lightMax = u_lightDistances[li];

			if (lightDist < lightMax)
			{
				float lightStrength = clamp(1.0 - clamp(lightDist / lightMax, 0.0, 1.0), 0.0, 1.0);
				lightStrength = pow(lightStrength, 4.0);// * 0.1;

				if (lightStrength > 0.01)
				{
					vec3 lightColor = u_lightColors[li].rgb;
					float lightColorLength = length(lightColor);

					if (lightColorLength > LIGHT_THRESHOLD)
					{
						// Try to maximize light strengths...
						lightColor /= lightColorLength;
						//lightColor.rgb /= (1.0 - (lightColorLength / 3.0));

						// Add some basic light...
						vec3 ambientLight = lightColor * lightStrength;
						addedLight += ambientLight * 0.25;//u_Local3.r; // Always add some basic light...
						addedLight += ambientLight * gl_FragColor.rgb * 0.25;//u_Local3.g; // Always add some basic light...

						vec3 lightDir = normalize(position.xyz - lightPos);
						float lambertian3 = dot(lightDir.xyz, N);

						if (lambertian3 > 0.0)
						{// this is blinn phong
							// Specular...
							vec3 halfDir3 = normalize(lightDir.xyz + E);
							float specAngle3 = max(dot(halfDir3, N), 0.0);
							float spec3 = pow(specAngle3, 16.0);

							float strength = ((1.0 - spec3) * (norm.a)) * lightStrength * 0.25;//u_Local3.b;
							addedLight +=  lightColor * strength * 0.5;
						}
					}
				}
			}
		}

		if (length(addedLight) > 0.0)
		{
			//gl_FragColor.rgb += clamp(addedLight * 0.05, 0.0, 1.0);
			//gl_FragColor.rgb += clamp(addedLight * /*0.3*/u_Local3.a, 0.0, 1.0);
			gl_FragColor.rgb += addedLight * 0.3;
			//gl_FragColor.rgb = clamp(gl_FragColor.rgb, 0.0, 1.0);
		}
	}

#ifdef __CUBEMAP__
	if (u_CubeMapStrength > 0.0 && enableCubeMap * 2.0 > 0.0)
	{
		vec4 specular;
		specular.rgb = gl_FragColor.rgb;
#define specLower ( 64.0 / 255.0)
#define specUpper (255.0 / 192.0)
		specular.rgb = clamp((clamp(specular.rgb - specLower, 0.0, 1.0)) * specUpper, 0.0, 1.0);
		//specular.a = ((clamp(cubeStrength, 0.0, 1.0) + clamp(specularScale, 0.0, 1.0)) / 2.0) * 1.6;
		specular.a = norm.a;

		specular.rgb *= enableCubeMap * 2.0;

		float NE = clamp(dot(N, E), 0.0, 1.0);

		vec3 R = reflect(E, N);
		vec3 reflectance = EnvironmentBRDF(clamp(specular.a, 0.5, 1.0) * 100.0, NE, specular.rgb);
		vec3 parallax = u_CubeMapInfo.xyz + u_CubeMapInfo.w * (u_ViewOrigin.xyz - position.xyz);//viewDir;
		vec3 cubeLightColor = textureCubeLod(u_CubeMap, R + parallax, 7.0 - specular.a * 7.0).rgb * 0.25;
		gl_FragColor.rgb += (cubeLightColor * reflectance * (cubeStrength * specular.a)) * u_CubeMapStrength * 0.5;
	}
#elif defined(__SSR__)
	// Screen space reflections...
	if (enableCubeMap > 0.0)
	{
		gl_FragColor.rgb = AddReflection(texCoords, position, gl_FragColor.rgb, 0.15/*u_CubeMapStrength*/ * cubeStrength);
	}
#endif //__CUBEMAP__ || __SSR__

#ifdef __AMBIENT_OCCLUSION__
	//if (u_Local2.g >= 1.0)
	//if (position.a != 0.0 && position.a != 1024.0 && position.a != 1025.0)
	{
		float ao = calculateAO(position.xyz / 524288.0, -N.xyz);

		ao = clamp(ao * 0.1 + 0.9, 0.0, 1.0);
		float ao2 = clamp(ao + 0.95, 0.0, 1.0);
		ao = (ao + ao2) / 2.0;
		//ao *= ao;
		ao = pow(ao, 4.0);

		gl_FragColor.rgb *= ao;
	}
#endif //__AMBIENT_OCCLUSION__

#ifdef __BUMP__
	gl_FragColor.rgb *= occ;
#endif //__BUMP__

	//gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
}


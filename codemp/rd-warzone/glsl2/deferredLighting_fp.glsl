﻿//#define __AMBIENT_OCCLUSION__
//#define __BUMP__

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_DeluxeMap;
uniform sampler2D	u_GlowMap;
uniform sampler2D	u_ScreenDepthMap;
uniform samplerCube	u_CubeMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D	u_ShadowMap;
#endif //defined(USE_SHADOWMAP)

uniform vec2		u_Dimensions;

uniform vec4		u_Local2; // r_blinnPhong, SHADOWS_ENABLED, SHADOW_MINBRIGHT, SHADOW_MAXBRIGHT
uniform vec4		u_Local3; // r_testShaderValue1, r_testShaderValue2, r_testShaderValue3, r_testShaderValue4

uniform vec3		u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;

uniform vec4		u_CubeMapInfo;
uniform float		u_CubeMapStrength;

uniform vec4		u_ViewInfo; // znear, zfar, zfar / znear, fov

#define MAX_LIGHTALL_DLIGHTS 256//64//16//24

uniform int			u_lightCount;
uniform vec2		u_lightPositions[MAX_LIGHTALL_DLIGHTS];
uniform vec3		u_lightPositions2[MAX_LIGHTALL_DLIGHTS];
uniform float		u_lightDistances[MAX_LIGHTALL_DLIGHTS];
uniform vec3		u_lightColors[MAX_LIGHTALL_DLIGHTS];

varying vec2		var_TexCoords;

#define textureCubeLod textureLod // UQ1: > ver 140 support


#define LIGHT_THRESHOLD  0.001


vec2 pixel = vec2(1.0) / u_Dimensions;


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

	float lightScale = clamp((1.0 - max(max(color.r, color.g), color.b)) - 0.2, 0.0, 1.0);

	vec2 texCoords = var_TexCoords;

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

#if 0 // Debugging stuff
	/*if (u_Local3.g >= 2.0)
	{
		if (u_Local3.g >= 4.0)
			gl_FragColor.rgb = vec3((position.rg / u_Local3.r), 0.0) * 0.5 + 0.5;
		else if (u_Local3.g >= 3.0)
			gl_FragColor.rgb = vec3(position.b / u_Local3.r) * 0.5 + 0.5;
		else
			gl_FragColor.rgb = vec3(position.rgb / u_Local3.r) * 0.5 + 0.5;
		return;
	}

	if (u_Local3.g >= 1.0)
	{*/
		float dist = distance(position.xyz, u_ViewOrigin.xyz);
		dist = clamp(dist / 4096.0, 0.0, 1.0);
		gl_FragColor.rgb = vec3(dist);
		return;
	//}
#endif

	vec4 norm = textureLod(u_NormalMap, texCoords, 0.0);

	norm.rgb = normalize(norm.rgb * 2.0 - 1.0);
	vec3 N = norm.xyz;
	vec3 E = normalize(u_ViewOrigin.xyz - position.xyz);

	norm.a = clamp(length(gl_FragColor.rgb), 0.0, 1.0);
#define const_1 ( 32.0 / 255.0)
#define const_2 (255.0 / 219.0)
	norm.a = clamp((clamp(norm.a - const_1, 0.0, 1.0)) * const_2, 0.0, 1.0);

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

		gl_FragColor.rgb *= clamp(shadowValue + u_Local2.b, u_Local2.b, u_Local2.a * lightScale);
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

	float normStrength = (norm.a * 0.5 + 0.5) * 0.2;//u_Local3.r;

	if (!noSunPhong && lambertian2 > 0.0)
	{// this is blinn phong
		vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
		float specAngle = max(dot(halfDir2, N), 0.0);
		spec2 = pow(specAngle, 16.0);
		gl_FragColor.rgb += (vec3(clamp(spec2, 0.0, 1.0) * normStrength) * gl_FragColor.rgb * u_PrimaryLightColor.rgb * phongFactor) * lightScale;
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
					float lightColorLength = length(lightColor) / 3.0;

					if (lightColorLength > LIGHT_THRESHOLD)
					{
						// Try to maximize light strengths...
						//lightColor /= lightColorLength;

						// Add some basic light...
						vec3 ambientLight = lightColor * lightStrength * lightScale * 0.5;
						vec3 diffuseLight = lightColor * lightStrength * lightScale * gl_FragColor.rgb * 8.0;
						addedLight += ambientLight; // Always add some basic light...
						addedLight += diffuseLight; // Always add some basic diffuse light...
						
						// Specular...
						vec3 lightDir = -normalize(lightPos - position.xyz);
						vec3 R = normalize(-reflect(lightDir,N));
						float specAngle3 = max(-dot(R,E),0.0);
						float spec3 = clamp(pow(specAngle3, 0.7), 0.0, 1.0);
						
						addedLight += lightColor * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 0.5 * (spec3 * (norm.a * 0.5 + 0.5)) * phongFactor * 8.0;
					}
				}
			}
		}

		gl_FragColor.rgb = clamp(gl_FragColor.rgb + clamp(addedLight, 0.0, 1.0) * lightScale, 0.0, 1.0);
	}

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


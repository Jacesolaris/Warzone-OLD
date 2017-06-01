//#define __SSR__
//#define __SSE__
//#define __SSE2__
//#define __AMBIENT_OCCLUSION__
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

#ifdef __SSE__
float pw = pixel.x;
float ph = pixel.y;

vec3 AddScreenSpaceEmissive(vec2 coord, vec3 inColor, float reflectStrength)
{
	// Quick scan for pixel that is not water...
	float QLAND_Y = 0.0;

	for (float y = coord.y; y < 1.0; y += ph * 50.0)
	{
		vec4 pMap = textureLod(u_GlowMap, vec2(coord.x, y), 0.0);

		if (length(pMap.rgb) > 0.0)
		{
			QLAND_Y = y;
			break;
		}
	}

	if (QLAND_Y <= 0.0)
	{// Found noglow surfaces...
		return inColor;
	}
	
	QLAND_Y -= ph * 50.0;

	for (float y = coord.y; y < 1.0; y += ph * 5.0)
	{
		vec4 pMap = textureLod(u_GlowMap, vec2(coord.x, y), 0.0);

		if (length(pMap.rgb) > 0.0)
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
		vec4 pMap = textureLod(u_GlowMap, vec2(coord.x, y), 0.0);

		if (length(pMap.rgb) > 0.0)
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

	vec4 landColor = textureLod(u_GlowMap, vec2(coord.x, upPos), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x + pw, upPos), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x - pw, upPos), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x, upPos + ph), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x, upPos - ph), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x + pw, upPos + ph), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x - pw, upPos - ph), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x + pw, upPos - ph), 0.0);
	landColor += textureLod(u_GlowMap, vec2(coord.x - pw, upPos + ph), 0.0);
	landColor /= 9.0;

	return mix(inColor.rgb, inColor.rgb + landColor.rgb, (1.0 - pow(upPos, 4.0)) * reflectStrength);
}
#endif //__SSE__

#ifdef __SSE2__

#define u_DiffuseMapWidth		u_Dimensions.x
#define u_DiffuseMapHeight		u_Dimensions.y

#define width					u_Dimensions.x
#define height					u_Dimensions.y

//#######################################
//these MUST match your current settings
float znear = u_ViewInfo.r;//1.0;                      //camera clipping start
float zfar = u_ViewInfo.g;//50.0;                      //camera clipping end
float fov = 1.0;//90.0 / u_Local3.g;//u_ViewInfo.a;//90.0;                //check your camera settings, set this to (90.0 / fov) (make sure you put a ".0" after your number)
float aspectratio = u_Dimensions.x/u_Dimensions.y;//16.0/9.0;           //width / height (make sure you put a ".0" after your number)
vec3 skycolor = vec3(0.0,0.0,0.0);   // use the horizon color under world properties, fallback when reflections fail

//tweak these to your liking
//float reflectStrength = 0.04;    //reflectivity of surfaced that you face head-on
float stepSize = 0.003;//u_Local3.b;//0.03;      //reflection choppiness, the lower the better the quality, and worse the performance 
int samples = 100;          //reflection distance, the higher the better the quality, and worse the performance
float startScale = 4.0;     //first value for variable scale calculations, the higher this value is, the faster the filter runs but it gets you staircase edges, make sure it is a power of 2

//#######################################

float getDepth(vec2 coord){
    float zdepth = texture(u_ScreenDepthMap, coord).x;
    return -zfar * znear / (zdepth * (zfar - znear) - zfar);
	//return 1.0 / mix(u_ViewInfo.x, 1.0, zdepth);
}

vec3 getViewPosition(vec2 coord){
    vec3 pos = vec3((coord.s * 2.0 - 1.0) / fov, (coord.t * 2.0 - 1.0) / aspectratio / fov, 1.0);
    return (pos * getDepth(coord));
}

vec3 getViewNormal(vec2 coord){
    
    vec3 p0 = getViewPosition(coord);
    vec3 p1 = getViewPosition(coord + vec2(1.0 / width, 0.0));
    vec3 p2 = getViewPosition(coord + vec2(0.0, 1.0 / height));
  
    vec3 dx = p1 - p0;
    vec3 dy = p2 - p0;
    return normalize(cross( dy , dx ));
}

vec2 getViewCoord(vec3 pos){
    vec3 norm = pos / pos.z;
    vec2 view = vec2((norm.x / fov + 1.0) / 2.0, (norm.y / fov * aspectratio + 1.0) / 2.0);
    return view;
}

float lenCo(vec3 vector){
    return pow(pow(vector.x,2.0) + pow(vector.y,2.0) + pow(vector.z,2.0), 0.5);
}

vec3 rayTrace(vec3 startpos, vec3 dir){
    vec3 pos = startpos;
    float olz = pos.z;      //previous z
    float scl = startScale; //step scale
    vec2 psc;               // Pixel Space Coordinate of the ray's' current viewspace position 
    vec3 ssg;               // Screen Space coordinate of the existing Geometry at that pixel coordinate
    
    for(int i = 0; i < samples; i++){
        olz = pos.z; //previous z
        pos = pos + dir * stepSize * pos.z * scl;
        psc = getViewCoord(pos); 
        ssg = getViewPosition(psc); 
        if(psc.x < 0.0 || psc.x > 1.0 || psc.y < 0.0 || psc.y > 1.0 || pos.z < 0.0 || pos.z >= zfar){
            //out of bounds
            break;
        }
        if(scl == 1 && lenCo(pos) > lenCo(ssg) && lenCo(pos) - lenCo(ssg) < stepSize * 40){
            //collided
            return pos;
        }
        if(scl > 1 && lenCo(pos) - lenCo(ssg) > stepSize * scl * -1){
            //lower step scale
            pos = pos - dir * stepSize * olz * scl;
            scl = scl * 0.5;
        }
    }
    // this will only run if loop ends before return or after break statement
    return vec3(0.0, 0.0, 0.0);
}
float schlick(float r0, vec3 n, vec3 i){
    return r0 + (1.0 - r0) * pow(1.0 - dot(-i,n),5.0);
}

vec3 AddScreenSpaceEmissive2(vec2 coord, vec3 inColor, float reflectStrength)
{
    
    //fragment color data
    vec3 reflection;
    
    //fragment geometry data
    vec3 position = getViewPosition(var_TexCoords);
    vec3 normal   = getViewNormal(var_TexCoords);
    vec3 viewVec  = normalize(position);
    vec3 reflect  = reflect(viewVec,normal);
    
    //raytrace collision
    vec3 collision = rayTrace(position, reflect);
    
    //choose method
    if (collision.z != 0.0) {
        vec2 sampler = getViewCoord(collision);
        reflection  = clamp(texture(u_DiffuseMap, sampler).rgb, 0.0, 1.0);

		//reflection  = clamp(texture(u_GlowMap, sampler).rgb, 0.0, 1.0);

		reflection.rgb = clamp(reflection.rgb - 0.3, 0.0, 1.0);
		reflection.rgb *= 1.4285;

		if (length(reflection.rgb) <= 0.0)
		{
			return inColor;
		}

		// Try to maximize light strengths...
		//float lightColorLength = length(reflection) / 3.0;
		//reflection /= lightColorLength;
		reflection *=  u_Local3.g;
    } else {
        //reflection = pow(skycolor,vec3(0.455));
		//reflection = inColor;
		return inColor;
    }
    
	/*
	if (u_Local3.g >= 9.0)
		return vec3(getDepth(var_TexCoords));
	else if (u_Local3.g >= 8.0)
		return collision;
	else if (u_Local3.g >= 7.0)
		return reflect;
	else if (u_Local3.g >= 6.0)
		return viewVec;
	else if (u_Local3.g >= 5.0)
		return position;
	else if (u_Local3.g >= 4.0)
		return normal;
	else if (u_Local3.g >= 3.0)
		return vec3(schlick(reflectStrength, normal, viewVec));
	else if (u_Local3.g >= 2.0)
		return inColor;
	else if (u_Local3.g >= 1.0)
		return reflection;
	else*/
		return mix(inColor, inColor + reflection, schlick(reflectStrength, normal, viewVec)).rgb;
}
#endif //__SSE2__

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
						vec3 ambientLight = lightColor * lightStrength * lightScale * 0.15;//u_Local3.r;//0.1;
						vec3 diffuseLight = lightColor * lightStrength * lightScale * gl_FragColor.rgb * 3.0;// u_Local3.g;//0.5;
						addedLight += ambientLight; // Always add some basic light...
						addedLight += diffuseLight; // Always add some basic diffuse light...
						
						// Specular...
						vec3 lightDir = -normalize(lightPos - position.xyz);
						vec3 R = normalize(-reflect(lightDir,N));
						float specAngle3 = max(-dot(R,E),0.0);
						float spec3 = clamp(pow(specAngle3, 0.7), 0.0, 1.0);
						
						addedLight += lightColor * lightStrength * lightScale * (length(gl_FragColor.rgb) / 3.0) * 0.5 * (spec3 * (norm.a * 0.5 + 0.5)) * phongFactor * 256.0;//u_Local3.b;//48.0;
					}
				}
			}
		}

		gl_FragColor.rgb = clamp(gl_FragColor.rgb + clamp(addedLight, 0.0, 1.0) * lightScale, 0.0, 1.0);
	}

#if defined(__SSR__)
	// Screen space reflections...
	if (enableCubeMap > 0.0)
	{
		gl_FragColor.rgb = AddReflection(texCoords, position, gl_FragColor.rgb, 0.15/*u_CubeMapStrength*/ * cubeStrength);
	}
#endif //__SSR__

#if defined(__SSE__)
	gl_FragColor.rgb = AddScreenSpaceEmissive(texCoords, gl_FragColor.rgb, u_Local3.r);
#endif //__SSE__

#if defined(__SSE2__)
	gl_FragColor.rgb = AddScreenSpaceEmissive2(texCoords, gl_FragColor.rgb, u_Local3.r);
#endif //__SSE2__

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


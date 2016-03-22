#define ORIGINAL_WATER
//#define SECOND_WATER




#if defined(ORIGINAL_WATER)





uniform sampler2D u_DiffuseMap;
//uniform sampler2D u_OverlayMap; // sky diffuse

varying vec4		var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec4		u_Local2; // ExtinctionCoefficient
varying vec4		u_Local3; // RimScalar, MaterialThickness, subSpecPower
uniform vec4		u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4		u_Local6; // useSunLightSpecular
uniform vec4		u_Local9; // 

varying vec2		var_Dimensions;

varying float  var_Time;

uniform sampler2D u_NormalMap;

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

#if defined(USE_CUBEMAP)
#define textureCubeLod textureLod // UQ1: > ver 140 support
uniform samplerCube u_CubeMap;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 
uniform vec4		u_PrimaryLightOrigin;
uniform vec3		u_PrimaryLightColor;
uniform vec3		u_PrimaryLightAmbient;
uniform float		u_PrimaryLightRadius;
uniform vec4		u_ViewOrigin;

uniform int			u_lightCount;
uniform vec3		u_lightPositions2[16];
uniform float		u_lightDistances[16];
uniform vec3		u_lightColors[16];

#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR) || defined(USE_SHADOWMAP) 


varying vec2		var_TexCoords;
varying vec4		var_Color;
varying vec3		var_ViewDir;
varying vec3		var_Normal;
varying vec3		var_N;
varying float		var_Fresnel;
varying vec3		var_vertPos;

out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_DetailedNormal;
out vec4 out_PositionMap;
out vec4 out_FoliageMap;




mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
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

void main()
{
	vec3 viewDir, lightColor, ambientColor;
	vec4 specular = vec4(1.0);
	vec3 N, E;
	vec2 tex_offset = vec2(1.0 / var_Dimensions.x, 1.0 / var_Dimensions.y);

	mat3 tangentToWorld = cotangent_frame(var_Normal.xyz, -var_ViewDir, var_TexCoords.xy);
	viewDir = var_ViewDir;

	E = normalize(viewDir);

	lightColor	= var_Color.rgb;

	vec2 texCoords = var_TexCoords.xy;
	

	//
	// Water
	//
	
	vec2 cPos = -1.0 + 2.0 * texCoords.xy;
	float cLength = length(cPos);
	
	vec2 uvOff = (cPos/cLength)*cos(cLength*12.0-var_Time*4.0)*0.03;

	vec2 uv = texCoords.xy+uvOff;
	vec4 diffuse = texture2D(u_DiffuseMap, uv);
	vec4 orig_diffuse = diffuse;
	gl_FragColor = vec4(diffuse.rgb,1.0);
	


	/*
	float shininess = 0.5;
	vec3 mirrorEye = vec3(2.0) * dot(normalize(L), var_Normal.xyz) * var_Normal.xyz - normalize(L);
	float dotSpec = clamp(dot(mirrorEye.xyz, -L), 0.0, 1.0) * 0.5 + 0.5;
	specular.rgb = vec3(1.0 - var_Fresnel) * clamp(-L.y, 0.0, 1.0) * (pow(dotSpec, 512.0) * (shininess * 1.8 + 0.2));
	specular.rgb += specular.rgb * 25.0 * clamp(shininess - 0.05, 0.0, 1.0);
	gl_FragColor.rgb += specular.rgb;
	*/

	//
	//
	//


	vec3 norm1 = -vec3(clamp((vec3(uvOff, 0.0) * 1.0/*u_Local9.b*/), 0.0, 1.0) * 2.0 - 1.0);
//	vec3 m_Normal = tangentToWorld * norm1;

	vec4 norm = texture2D(u_NormalMap, uv);
	vec3 m_Normal = norm.xyz * 2.0 - 1.0;
	m_Normal.z = sqrt(clamp((0.25 - m_Normal.x * m_Normal.x) - m_Normal.y * m_Normal.y, 0.0, 1.0));
	m_Normal = tangentToWorld * ((m_Normal + norm1) / 2.0);
	m_Normal = normalize(m_Normal);

	/*if (u_Local9.a > 0.0)
	{
		gl_FragColor = vec4(m_Normal.xyz * 0.5 + 0.5, 1.0);
		out_Glow = vec4(0.0);
		out_DetailedNormal = vec4(m_Normal.xyz, 0.75);
		out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
		return;
	}*/

	#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		if (u_Local6.r > 0.0)
		{
			vec3 E = normalize(-(u_ViewOrigin.xyz - var_vertPos.xyz));
			vec3 PrimaryLightDir = u_PrimaryLightOrigin.xyz - var_vertPos.xyz;
			float lambertian2 = dot(PrimaryLightDir.xyz, m_Normal);

			if(lambertian2 > 0.0)
			{// this is blinn phong
				vec3 halfDir2 = normalize(PrimaryLightDir.xyz + E);
				float specAngle = max(dot(halfDir2, m_Normal), 0.0);
				float spec2 = pow(specAngle, 16.0);
				gl_FragColor.rgb += vec3(spec2 * (1.0 - length(gl_FragColor.rgb) / 3.0)) * u_PrimaryLightColor.rgb * 0.2/*u_Local9.g*/ * u_Local5.b;
			}

			for (int li = 0; li < u_lightCount; li++)
			{
				vec3 lightDir = u_lightPositions2[li] - var_vertPos.xyz;
				float lambertian3 = dot(lightDir.xyz, m_Normal);
				float spec3 = 0.0;

				if(lambertian3 > 0.0)
				{
					float lightStrength = clamp(1.0 - (length(lightDir) * (1.0 / u_lightDistances[li])), 0.0, 1.0) * 0.5;

					if(lightStrength > 0.0)
					{// this is blinn phong
						vec3 halfDir3 = normalize(lightDir.xyz + E);
						float specAngle3 = max(dot(halfDir3, m_Normal), 0.0);
						spec3 = pow(specAngle3, 16.0);
						gl_FragColor.rgb += vec3(spec3 * (1.0 - length(gl_FragColor.rgb) / 3.0)) * u_lightColors[li].rgb * lightStrength * u_Local5.b;
					}
				}
			}
		}
	#endif //defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)


	gl_FragColor.a = 0.7;//1.0;
	out_Glow = vec4(0.0);


	out_DetailedNormal = vec4(m_Normal.xyz * 0.5 + 0.5, 0.75);
	out_PositionMap = vec4(var_vertPos, 0.0);

	out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
}







#elif defined(SECOND_WATER)





const vec4 WATER_COLOR = vec4(0.0, 0.0, 0.5, 1.0);
const float sca = 0.005;
const float sca2 = 0.02;
const float tscale = 0.1;//0.25;
const vec4 two = vec4(2.0, 2.0, 2.0, 1.0);
const vec4 mone = vec4(-1.0, -1.0, -1.0, 1.0);

const vec4 ofive = vec4(0.5,0.5,0.5,1.0);
const float exponent = 64.0;

uniform sampler2D u_NormalMap;		// normal map
uniform sampler2D u_DiffuseMap;		// reflection
uniform sampler2D u_DeluxeMap;		// dudvmap

varying vec3 var_vertPos;
//varying vec4 lightDir;				//lightpos
varying vec2 waterFlow;				//moving texcoords
varying vec2 waterRipple;			//moving texcoords
varying vec4 projCoords;			//for projection
//varying vec4 eyeDir;				//viewts


out vec4 out_Glow;
out vec4 out_DetailedNormal;
out vec4 out_PositionMap;
out vec4 out_FoliageMap;

const vec4 tangent = vec4(1.0, 0.0, 0.0, 0.0);
const vec4 norm = vec4(0.0, 1.0, 0.0, 0.0);
const vec4 binormal = vec4(0.0, 0.0, 1.0, 0.0);

const mat4 lightTransformation = mat4(tangent, binormal, norm, vec4(0.0));

uniform vec4	u_ViewOrigin;
uniform vec4		u_PrimaryLightOrigin;

//varying vec2 UV;

void main(void)
{
	vec4 temp = vec4(u_ViewOrigin.xyz - gl_FragCoord.xyz, 0.0);
	vec4 eye = temp * lightTransformation;
	vec4 viewt = normalize(eye);

	//vec4 viewt = normalize(eyeDir);

	temp = vec4(u_PrimaryLightOrigin.xyz - gl_FragCoord.xyz, 0.0); // Directional light from the sun
	//temp = vec4(u_PrimaryLightOrigin.xyz - (gl_FragCoord.xyz * u_PrimaryLightOrigin.w), 0.0);
	vec4 lightDir = temp * lightTransformation;

	vec2 rippleEffect = sca2 * texture2D(u_DeluxeMap, waterRipple * tscale).xy;
	vec4 normal = texture2D(u_NormalMap, waterFlow + rippleEffect);

	// Reflection distortion
	vec2 fdist = texture2D(u_DeluxeMap, waterFlow + rippleEffect).xy;
	fdist = fdist * 2.0 - vec2(1.0);
	fdist *= sca;

	//load normalmap
	vec4 vNorm = (normal-ofive) * two;
	vNorm = normalize(-vNorm); // superfluous?

	//calculate specular highlight
	vec4 lightTS = normalize(lightDir);
	vec4 vRef = normalize(reflect(-lightTS, vNorm));
	float stemp = clamp(dot(viewt, vRef), 0.0, 1.0);
	stemp = pow(stemp, exponent); 
	vec4 specular = vec4(stemp);

	//calculate fresnel and inverted fresnel 
	float invfres = dot(vNorm, viewt);
	float fres = 1.0 - invfres;

	//get projective texcoords
	vec2 projCoord = projCoords.xy / projCoords.w;
	projCoord = projCoord * 0.5 + 0.5;
	projCoord += fdist.xy;
	
	//load and calculate reflection
	//vec4 refl = texture2D(u_DiffuseMap, projCoord);
	//refl *= fres;

	// Set the water color
	vec4 waterColor = WATER_COLOR * invfres;

	//add it all up for the effect
	gl_FragColor = /*clamp(refl, 0.0, 1.0) +*/ waterColor + specular;
	gl_FragColor.a = 0.6;

	out_Glow = vec4(0.0);
	out_DetailedNormal = vec4(vNorm.xyz * 0.5 + 0.5, 0.75);
	out_PositionMap = vec4(gl_FragCoord.xyz, 0.0);
	out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
}






#else //!defined(ORIGINAL_WATER)





// Source: http://trederia.blogspot.com/2014/09/water-in-opengl-and-gles-20-part-4.html




uniform sampler2D u_DiffuseMap;		// reflection

uniform sampler2D u_NormalMap;

#define u_refractionTexture u_DiffuseMap
#define u_reflectionTexture u_DiffuseMap

uniform float u_Time;
uniform vec2	u_Dimensions;

uniform vec4		u_Local9; // 

//////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;
varying vec4 v_vertexReflectionPosition;

varying vec2 v_texCoord;

varying vec3 v_eyePosition;

varying vec3 var_Normal;

//////////////////////////

out vec4 out_Glow;
out vec4 out_DetailedNormal;
out vec4 out_PositionMap;
out vec4 out_FoliageMap;


const float distortAmount = 0.05;
const float specularAmount = 2.5;
const float textureRepeat = 2.0;

const vec4 tangent = vec4(1.0, 0.0, 0.0, 0.0);
const vec4 viewNormal = vec4(0.0, 1.0, 0.0, 0.0);
const vec4 bitangent = vec4(0.0, 0.0, 1.0, 0.0);

const vec4 waterColour = vec4(0.1, 0.1, 0.5,0.4);

vec2 fromClipSpace(vec4 position)
{
	return position.xy / position.w / 2.0 + 0.5;
}

vec2 fromClipSpaceFlip(vec4 position)
{
	return vec2(position.x, -position.y) / position.w / 2.0 + 0.5;
}

void main()
{	
	//get normal
	vec4 normal = texture2D(u_NormalMap, v_texCoord * textureRepeat + u_Time);
	normal = normalize(normal * 2.0 - 1.0);
		
	//put view in tangent space
	vec4 viewDir = normalize(vec4(v_eyePosition, 1.0));
	vec4 viewTanSpace = normalize(vec4(dot(viewDir, tangent), dot(viewDir, bitangent), dot(viewDir, viewNormal), 1.0));	
	vec4 viewReflection = normalize(reflect(-1.0 * viewTanSpace, normal));
	float fresnel = dot(normal, viewReflection);

	//distortion offset
	vec4 dudv = normal * distortAmount;
	
	//refraction sample
	vec2 textureCoord = fromClipSpace(v_vertexRefractionPosition) + dudv.rg;
	//float fromCenter = textureCoord.y - textureCoordOrig.y;
	//float textureCoordY = textureCoordOrig.y - (fromCenter*u_Local9.g);
	//textureCoord.y += textureCoordY;
	textureCoord = clamp(textureCoord, 0.001, 0.999);
	vec4 refractionColour = texture2D(u_refractionTexture, textureCoord) * waterColour;
	
	//calc fog distance
	//----version 1 (exponential)----//
	//float z = gl_FragCoord.z / gl_FragCoord.w;
	//const float fogDensity = 0.0005;
	//float fogAmount = exp2(-fogDensity * fogDensity * z * z * 1.442695);
	//fogAmount = clamp(fogAmount, 0.0, 0.7);
	
	//----version 2 (linear)----//
	float z = (gl_FragCoord.z / gl_FragCoord.w) / 300.0; //const is max fog distance
	const float fogDensity = 6.0;
	float fogAmount = z * fogDensity;	
	fogAmount = clamp(fogAmount, 0.0, 1.0);
	
	refractionColour = mix(refractionColour, waterColour, fogAmount);
	
#if 0
	//reflection sample
	textureCoord = fromClipSpace(v_vertexReflectionPosition) + dudv.rg;
	/*
	vec2 textureCoordOrig = fromClipSpaceOrig(v_vertexReflectionPosition) + dudv.rg;
	//float fromCenter = ((gl_FragCoord.y/u_Dimensions.y) - 0.5);
	float fromCenter = textureCoord.y - textureCoordOrig.y;
	float textureCoordY = textureCoordOrig.y - (fromCenter*u_Local9.g);
	textureCoord.y -= (0.5 - textureCoordY);
	*/
	
	/*float Ypix = (gl_FragCoord.y/u_Dimensions.y);
	
	if (Ypix > 0.5)
	{
		float fromCenter = Ypix - 0.5;
		float invert = 0.5 - fromCenter;
		textureCoord.y = 1.0 - invert;
	}*/
#else
	vec4 reflected;
	/*
	if (u_Local9.a <= 0) reflected = normalize(reflect(normalize(v_vertexRefractionPosition), normalize(normal)));
	else if (u_Local9.a <= 1) reflected = normalize(reflect(normalize(v_vertexRefractionPosition), normalize(vec4(var_Normal, 0.0))));
	else if (u_Local9.a <= 2) reflected = reflect(normalize(v_vertexRefractionPosition), normalize(normal));
	else if (u_Local9.a <= 3) reflected = reflect(normalize(v_vertexRefractionPosition), normalize(vec4(var_Normal, 0.0)));
	else if (u_Local9.a <= 4) reflected = normalize(reflect(viewDir, normalize(normal)));
	else if (u_Local9.a <= 5) reflected = normalize(reflect(viewDir, normalize(vec4(var_Normal, 0.0))));
	else if (u_Local9.a <= 6) reflected = normalize(reflect(-viewDir, normalize(normal)));
	else if (u_Local9.a <= 7) reflected = normalize(reflect(-viewDir, normalize(vec4(var_Normal, 0.0))));
	else if (u_Local9.a <= 8) reflected = normalize(reflect(normalize(v_vertexRefractionPosition), normalize(vec4(u_Local9.rgb, 0.0))));
	*/
	if (u_Local9.a <= 0) reflected = normalize(reflect(normalize(v_vertexRefractionPosition), vec4(0.0, (viewDir.y - 0.5), 0.0, 0.0)));
	else if (u_Local9.a <= 1) reflected = normalize(reflect(normalize(v_vertexRefractionPosition), normalize(vec4(0.0, 1.0, 0.0, 0.0) * vec4(viewDir.xyz, 0.0))));
	else if (u_Local9.a <= 2) reflected = normalize(refract(normalize(v_vertexRefractionPosition), normalize(vec4(0.0, -1.0, 0.0, 0.0)), 1.0));
	else if (u_Local9.a <= 3) reflected = normalize(reflect(normalize(v_vertexRefractionPosition), normalize(-v_vertexRefractionPosition)));
	else
	{
		vec4 viewDir = normalize(vec4(v_eyePosition, 1.0));
		vec4 viewTanSpace = normalize(vec4(dot(viewDir, tangent), dot(viewDir, bitangent), dot(viewDir, viewNormal), 1.0));	
		reflected = normalize(reflect(-1.0 * viewTanSpace, normal));
	}

	textureCoord = fromClipSpace(reflected) + dudv.rg;
	//textureCoord.y = 1.0 - textureCoord.y;
#endif

	textureCoord = clamp(textureCoord, 0.001, 0.999);
	vec4 reflectionColour = texture2D(u_reflectionTexture, textureCoord);
	
	vec3 specular = vec3(clamp(pow(dot(viewTanSpace, normal), 255.0), 0.0, 1.0)) * specularAmount;
	
	gl_FragColor = mix(reflectionColour, refractionColour, fresnel) + vec4(specular, 1.0);

	out_Glow = vec4(0.0);
	out_DetailedNormal = vec4(normal.xyz * 0.5 + 0.5, 0.75);
	out_PositionMap = vec4(gl_FragCoord.xyz, 0.0);
	out_FoliageMap = vec4(0.0, 0.0, 1.0, 0.0);
}


#endif //defined(ORIGINAL_WATER)
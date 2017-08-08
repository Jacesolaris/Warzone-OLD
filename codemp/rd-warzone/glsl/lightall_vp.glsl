attribute vec2 attr_TexCoord0;

attribute vec2 attr_TexCoord1;
attribute vec4 attr_Color;

attribute vec3 attr_Position;
attribute vec3 attr_Normal;
attribute vec4 attr_Tangent;

attribute vec3 attr_Position2;
attribute vec3 attr_Normal2;
attribute vec4 attr_Tangent2;
attribute vec4 attr_BoneIndexes;
attribute vec4 attr_BoneWeights;

uniform vec4				u_Settings0; // useTC, useDeform, useRGBA, isTextureClamped
uniform vec4				u_Settings1; // useVertexAnim, useSkeletalAnim, useFog, is2D
uniform vec4				u_Settings2; // LIGHTDEF_USE_LIGHTMAP, LIGHTDEF_USE_GLOW_BUFFER, LIGHTDEF_USE_CUBEMAP, LIGHTDEF_USE_TRIPLANAR
uniform vec4				u_Settings3; // LIGHTDEF_USE_REGIONS, LIGHTDEF_IS_DETAIL

#define USE_TC				u_Settings0.r
#define USE_DEFORM			u_Settings0.g
#define USE_RGBA			u_Settings0.b
#define USE_TEXTURECLAMP	u_Settings0.a

#define USE_VERTEX_ANIM		u_Settings1.r
#define USE_SKELETAL_ANIM	u_Settings1.g
#define USE_FOG				u_Settings1.b
#define USE_IS2D			u_Settings1.a

#define USE_LIGHTMAP		u_Settings2.r
#define USE_GLOW_BUFFER		u_Settings2.g
#define USE_CUBEMAP			u_Settings2.b
#define USE_TRIPLANAR		u_Settings2.a

#define USE_REGIONS			u_Settings3.r
#define USE_ISDETAIL		u_Settings3.g


uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4	u_Local6; // useSunLightSpecular
uniform vec4	u_Local7; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4	u_Local9;

uniform sampler2D			u_DiffuseMap;

uniform float	u_Time;

uniform vec2	u_textureScale;

uniform vec4   u_EnableTextures; // x = normal, y = deluxe, z = specular, w = cube

uniform vec3   u_ViewOrigin;

uniform int    u_TCGen0;
uniform vec3   u_TCGen0Vector0;
uniform vec3   u_TCGen0Vector1;

uniform vec3   u_LocalViewOrigin;

uniform vec4   u_DiffuseTexMatrix;
uniform vec4   u_DiffuseTexOffTurb;

uniform int    u_ColorGen;
uniform int    u_AlphaGen;
uniform vec3   u_AmbientLight;
uniform vec3   u_DirectedLight;
uniform vec3   u_ModelLightDir;
uniform float  u_PortalRange;

uniform int    u_DeformGen;
uniform float  u_DeformParams[5];

uniform vec4   u_FogDistance;
uniform vec4   u_FogDepth;
uniform float  u_FogEyeT;
uniform vec4   u_FogColorMask;

uniform mat4   u_ModelViewProjectionMatrix;
//uniform mat4	u_ViewProjectionMatrix;
uniform mat4   u_ModelMatrix;
uniform mat4	u_NormalMatrix;

uniform vec4   u_BaseColor;
uniform vec4   u_VertColor;

uniform float  u_VertexLerp;
uniform mat4   u_BoneMatrices[20];

uniform vec4  u_PrimaryLightOrigin;
uniform float u_PrimaryLightRadius;

#if defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)
out vec3 Normal_CS_in;
out vec2 TexCoord_CS_in;
out vec4 WorldPos_CS_in;
out vec3 ViewDir_CS_in;
//out vec4 Tangent_CS_in;
//out vec4 Bitangent_CS_in;
out vec4 Color_CS_in;
out vec4 PrimaryLightDir_CS_in;
out vec2 TexCoord2_CS_in;
out vec3 Blending_CS_in;
out float Slope_CS_in;
out float usingSteepMap_CS_in;
#endif

varying vec2	var_TexCoords;
varying vec2	var_TexCoords2;

varying vec4	var_Color;

varying vec3	var_N;
varying vec3	var_Normal;
//varying vec4	var_Tangent;
//varying vec4	var_Bitangent;
varying vec3	var_ViewDir;

varying vec4	var_PrimaryLightDir;

varying vec3	var_vertPos;

varying vec3	var_Blending;
varying float	var_Slope;
varying float	var_usingSteepMap;

varying vec2   var_nonTCtexCoords; // for steep maps

vec3 DeformPosition(const vec3 pos, const vec3 normal, const vec2 st)
{
	float base =      u_DeformParams[0];
	float amplitude = u_DeformParams[1];
	float phase =     u_DeformParams[2];
	float frequency = u_DeformParams[3];
	float spread =    u_DeformParams[4];

	if (u_DeformGen == DGEN_BULGE)
	{
		phase *= st.x;
	}
	else // if (u_DeformGen <= DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		phase += dot(pos.xyz, vec3(spread));
	}

	float value = phase + (u_Time * frequency);
	float func;

	if (u_DeformGen == DGEN_WAVE_SIN)
	{
		func = sin(value * 2.0 * M_PI);
	}
	else if (u_DeformGen == DGEN_WAVE_SQUARE)
	{
		func = sign(fract(0.5 - value));
	}
	else if (u_DeformGen == DGEN_WAVE_TRIANGLE)
	{
		func = abs(fract(value + 0.75) - 0.5) * 4.0 - 1.0;
	}
	else if (u_DeformGen == DGEN_WAVE_SAWTOOTH)
	{
		func = fract(value);
	}
	else if (u_DeformGen == DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		func = (1.0 - fract(value));
	}
	else // if (u_DeformGen == DGEN_BULGE)
	{
		func = sin(value);
	}

	return pos + normal * (base + func * amplitude);
}

vec2 GenTexCoords(int TCGen, vec3 position, vec3 normal, vec3 TCGenVector0, vec3 TCGenVector1)
{
	vec2 tex = attr_TexCoord0;

	switch (TCGen)
	{
		case TCGEN_LIGHTMAP:
		case TCGEN_LIGHTMAP1:
		case TCGEN_LIGHTMAP2:
		case TCGEN_LIGHTMAP3:
			tex = attr_TexCoord1;
		break;

		case TCGEN_ENVIRONMENT_MAPPED:
		{
			vec3 viewer = normalize(u_LocalViewOrigin - position);
			vec2 ref = reflect(viewer, normal).yz;
			tex.s = ref.x * -0.5 + 0.5;
			tex.t = ref.y *  0.5 + 0.5;
		}
		break;

		case TCGEN_VECTOR:
		{
			tex = vec2(dot(position, TCGenVector0), dot(position, TCGenVector1));
		}
		break;
	}

	return tex;
}

vec2 ModTexCoords(vec2 st, vec3 position, vec4 texMatrix, vec4 offTurb)
{
	float amplitude = offTurb.z;
	float phase = offTurb.w * 2.0 * M_PI;
	vec2 st2;
	st2.x = st.x * texMatrix.x + (st.y * texMatrix.z + offTurb.x);
	st2.y = st.x * texMatrix.y + (st.y * texMatrix.w + offTurb.y);

	vec2 offsetPos = vec2(position.x + position.z, position.y);

	vec2 texOffset = sin(offsetPos * (2.0 * M_PI / 1024.0) + vec2(phase));

	return st2 + texOffset * amplitude;
}


vec4 CalcColor(vec3 position, vec3 normal)
{
	vec4 color = u_VertColor * attr_Color + u_BaseColor;
	
	if (USE_RGBA > 0.0)
	{
		if (u_ColorGen == CGEN_LIGHTING_DIFFUSE)
		{
			float incoming = clamp(dot(normal, u_ModelLightDir), 0.0, 1.0);
			color.rgb = clamp(u_DirectedLight * incoming + u_AmbientLight, 0.0, 1.0);
		}
	
		if (u_AlphaGen == AGEN_LIGHTING_SPECULAR)
		{
			//vec3 viewer = u_LocalViewOrigin - position;
			vec3 viewer = u_ViewOrigin - position;
			//vec3 lightDir = normalize(vec3(-960.0, 1980.0, 96.0) - position);
			//vec3 lightDir = normalize((u_ModelMatrix * vec4(u_PrimaryLightOrigin.xyz, 1.0)).xyz - position);
			//vec3 lightDir = normalize((u_ModelViewProjectionMatrix * vec4(u_PrimaryLightOrigin.xyz, 1.0)).xyz - position);
			vec3 lightDir = normalize(u_PrimaryLightOrigin.xyz - position);
			vec3 reflected = -reflect(lightDir, normal);
		
			color.a = clamp(dot(reflected, normalize(viewer)), 0.0, 1.0);
			color.a *= color.a;
			color.a *= color.a;
		}
		else if (u_AlphaGen == AGEN_PORTAL)
		{
			//vec3 viewer = u_LocalViewOrigin - position;
			vec3 viewer = u_ViewOrigin - position;
			color.a = clamp(length(viewer) / u_PortalRange, 0.0, 1.0);
		}
	}
	
	return color;
}

#if 0
float CalcFog(vec3 position)
{
	float s = dot(vec4(position, 1.0), u_FogDistance) * 8.0;
	float t = dot(vec4(position, 1.0), u_FogDepth);

	float eyeOutside = float(u_FogEyeT < 0.0);
	float fogged = float(t < eyeOutside);

	t += 1e-6;
	t *= fogged / (t - u_FogEyeT * eyeOutside);

	return s * t;
}
#endif

void GetBlending(vec3 normal)
{
	vec3 blend_weights = abs(normalize(normal.xyz));   // Tighten up the blending zone:
	blend_weights = (blend_weights - 0.2) * 7.0;
	blend_weights = max(blend_weights, 0.0);      // Force weights to sum to 1.0 (very important!)
	blend_weights /= vec3(blend_weights.x + blend_weights.y + blend_weights.z);
	var_Blending = blend_weights;
}

vec3 vectoangles(in vec3 value1) {
	float	forward;
	float	yaw, pitch;
	vec3	angles;

	if (value1.g == 0 && value1.r == 0) {
		yaw = 0;
		if (value1.b > 0) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		if (value1.r > 0) {
			yaw = (atan(value1.g, value1.r) * 180 / M_PI);
		}
		else if (value1.g > 0) {
			yaw = 90;
		}
		else {
			yaw = 270;
		}
		if (yaw < 0) {
			yaw += 360;
		}

		forward = sqrt(value1.r*value1.r + value1.g*value1.g);
		pitch = (atan(value1.b, forward) * 180 / M_PI);
		if (pitch < 0) {
			pitch += 360;
		}
	}

	angles.r = -pitch;
	angles.g = yaw;
	angles.b = 0.0;

	return angles;
}

/*
vec3 TangentFromNormal ( vec3 normal )
{
	vec3 tangent;
	vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0)); 
	vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0)); 

	if( length(c1) > length(c2) )
	{
		tangent = c1;
	}
	else
	{
		tangent = c2;
	}

	return normalize(tangent);
}
*/

void main()
{
	vec3 position;
	vec3 normal;
	//vec3 tangent;

	if (USE_VERTEX_ANIM == 1.0)
	{
		//position  = mix(attr_Position,    attr_Position2,    u_VertexLerp);
		//normal    = mix(attr_Normal,      attr_Normal2,      u_VertexLerp) * 2.0 - 1.0;
		//tangent   = mix(attr_Tangent.xyz, attr_Tangent2.xyz, u_VertexLerp) * 2.0 - 1.0;
		position  = attr_Position;
		normal    = attr_Normal * 2.0 - 1.0;
	}
	else if (USE_SKELETAL_ANIM == 1.0)
	{
		vec4 position4 = vec4(0.0);
		vec4 normal4 = vec4(0.0);
		//vec4 tangent4 = vec4(0.0);
		vec4 originalPosition = vec4(attr_Position, 1.0);
		//vec4 originalNormal = vec4(attr_Normal - vec3(0.5), 0.0);
		vec4 originalNormal = vec4(attr_Normal * 2.0 - 1.0, 0.0);
		//vec4 originalTangent = vec4(attr_Tangent.xyz - vec3(0.5), 0.0);

		for (int i = 0; i < 4; i++)
		{
			int boneIndex = int(attr_BoneIndexes[i]);

			position4 += (u_BoneMatrices[boneIndex] * originalPosition) * attr_BoneWeights[i];
			normal4 += (u_BoneMatrices[boneIndex] * originalNormal) * attr_BoneWeights[i];
			//tangent4 += (u_BoneMatrices[boneIndex] * originalTangent) * attr_BoneWeights[i];
		}

		position = position4.xyz;
		normal = normalize(normal4.xyz);
		//tangent = normalize(tangent4.xyz);
	}
	else
	{
		position  = attr_Position;
		normal    = attr_Normal * 2.0 - 1.0;
		//tangent   = attr_Tangent.xyz * 2.0 - 1.0;
	}


	vec2 texCoords = attr_TexCoord0.st;

	if (USE_DEFORM == 1.0)
	{
		position = DeformPosition(position, normal, attr_TexCoord0.st);
	}

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);

	vec3 preMMPos = position.xyz;

	// Because rend2 tangents are all fucked - re-calculate them.
	//tangent = TangentFromNormal(normal);

	//if (USE_VERTEX_ANIM == 1.0 || USE_SKELETAL_ANIM == 1.0)
	{
		position = (u_ModelMatrix * vec4(position, 1.0)).xyz;
		normal = (u_ModelMatrix * vec4(normal, 0.0)).xyz;
		//tangent = (u_ModelMatrix * vec4(tangent, 0.0)).xyz;
	}

	////vec3 bitangent = cross(normal, tangent) * (attr_Tangent.w * 2.0 - 1.0);
	//vec3 bitangent = normalize(cross(normal, tangent));

	var_nonTCtexCoords = attr_TexCoord0.st;

	if (USE_TC == 1.0)
	{
		texCoords = GenTexCoords(u_TCGen0, position, normal, u_TCGen0Vector0, u_TCGen0Vector1);
		texCoords = ModTexCoords(texCoords, position, u_DiffuseTexMatrix, u_DiffuseTexOffTurb);
	}


	if (!(u_textureScale.x <= 0.0 && u_textureScale.y <= 0.0) && !(u_textureScale.x == 1.0 && u_textureScale.y == 1.0))
	{
		texCoords *= u_textureScale;
	}

	var_Color = CalcColor(position, normal);

#if 0
	if (USE_FOG == 1.0)
	{
		var_Color *= vec4(1.0) - u_FogColorMask * sqrt(clamp(CalcFog(position), 0.0, 1.0));
	}
#endif

	var_TexCoords = texCoords;


	if (USE_LIGHTMAP > 0.0)
	{
		var_TexCoords2 = attr_TexCoord1.st;
	}
	else
	{
		var_TexCoords2 = vec2(0.0);
	}

	var_PrimaryLightDir.xyz = u_PrimaryLightOrigin.xyz - (position * u_PrimaryLightOrigin.w);

	var_ViewDir = u_ViewOrigin - position;

	// store view direction in tangent space to save on varyings
	var_Normal = normal.xyz;
	//var_Tangent = vec4(tangent, var_ViewDir.y);
	//var_Bitangent = vec4(bitangent, var_ViewDir.z);

	var_usingSteepMap = 0.0;
	var_Slope = 0.0;


	if (USE_TRIPLANAR > 0.0)
	{
		//
		// Steep Maps...
		//

		if (u_Local5.a > 0.0)
		{// Steep maps...
			float pitch = vectoangles(normalize(normal.xyz)).r;

			if (pitch > 180)
				pitch -= 360;

			if (pitch < -180)
				pitch += 360;

			pitch += 90.0f;

			if (pitch > 46.0 || pitch < -46.0)
			{
				var_usingSteepMap = 1.0;
				var_Slope = 1.0;
			}
			else if (pitch > 26.0 || pitch < -26.0)
			{// do not add to foliage map on this slope, but still do original texture
				var_usingSteepMap = 1.0;
				var_Slope = 0.0;
			}
			else
			{
				var_usingSteepMap = 0.0;
				var_Slope = 0.0;
			}
		}
	}

	var_Blending = vec3(0.0);

	if (USE_REGIONS > 0.0 || USE_TRIPLANAR > 0.0)
	{
		GetBlending(normalize(attr_Normal.xyz * 2.0 - 1.0));
	}

#if defined(USE_TESSELLATION) || defined(USE_ICR_CULLING)
	WorldPos_CS_in = vec4(position.xyz, 1.0);
	TexCoord_CS_in = var_TexCoords.xy;
	Normal_CS_in = var_Normal.xyz;
	ViewDir_CS_in = var_ViewDir;
	//Tangent_CS_in = var_Tangent;
	//Bitangent_CS_in = var_Bitangent;
	Color_CS_in = var_Color;
	PrimaryLightDir_CS_in = var_PrimaryLightDir;
	TexCoord2_CS_in = var_TexCoords2;
	Blending_CS_in = var_Blending;
	Slope_CS_in = var_Slope;
	usingSteepMap_CS_in = var_usingSteepMap;
	gl_Position = vec4(position.xyz/*preMMPos*/, 1.0);
#endif

	var_vertPos = position.xyz;

}
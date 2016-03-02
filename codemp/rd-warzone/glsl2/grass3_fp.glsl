//precision highp float;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SteepMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap

//#define USE_LIGHTING

varying float  var_Time;

#if defined(USE_LIGHTMAP)
uniform sampler2D u_LightMap;
#endif

//#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
//#endif

#if defined(USE_DELUXEMAP)
uniform sampler2D u_DeluxeMap;
#endif

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

#if defined(USE_CUBEMAP)
#define textureCubeLod textureLod // UQ1: > ver 140 support
uniform samplerCube u_CubeMap;
#endif

#if defined(SUBSURFACE_SCATTER)
uniform sampler2D u_SubsurfaceMap;
#endif

uniform sampler2D u_OverlayMap;

uniform vec3  u_PrimaryLightColor;
uniform vec3  u_PrimaryLightAmbient;

uniform vec4      u_NormalScale;
uniform vec4      u_SpecularScale;


varying vec2      var_TexCoords;


varying vec4   var_Normal;
varying vec4   var_Tangent;
varying vec4   var_Bitangent;
#define var_Normal2 var_Normal.w

varying vec4      var_PrimaryLightDir;

varying vec3      var_ViewDir;

#define m_TexCoords vec2(1.0)
#define m_Normal vec3(1.0)
#define m_vertPos var_vertPos
#define m_ViewDir var_ViewDir





out vec4 out_Glow;
//out vec4 out_Normal;
out vec4 out_DetailedNormal;
out vec4 out_FoliageMap;




in vec4 Color;

void main() 
{
	gl_FragColor = vec4(/*Color.rgb*/0.0, 0.0, 1.0, 1.0);

#if 0
#if defined(USE_PRIMARY_LIGHT) || defined(USE_PRIMARY_LIGHT_SPECULAR)
		vec3 viewDir = vec3(0.0);

		mat3 tangentToWorld = mat3(var_Tangent.xyz, var_Bitangent.xyz, m_Normal.xyz);
		viewDir = vec3(var_Normal2, var_Tangent.w, var_Bitangent.w);

		vec3 E = normalize(viewDir);

		float lambertian2 = dot(var_PrimaryLightDir.xyz, m_Normal.xyz);
		float spec2 = 0.0;

		if(lambertian2 > 0.0)
		{// this is blinn phong
			vec3 halfDir2 = normalize(var_PrimaryLightDir.xyz + E);
			float specAngle = max(dot(halfDir2, m_Normal.xyz), 0.0);
			spec2 = pow(specAngle, 16.0);
			gl_FragColor.rgb += vec3(spec2 * 0.25) * gl_FragColor.rgb * u_PrimaryLightColor.rgb;
		}
#endif
#endif

	out_Glow = vec4(0.0);

	out_DetailedNormal = vec4(m_Normal.xyz, 0.1);

	out_FoliageMap.r = 1.0;
	out_FoliageMap.g = 1.0;
}

uniform sampler2D u_DiffuseMap;

uniform vec2	u_Dimensions;
uniform vec4	u_Local1; // parallaxScale, haveSpecular, specularScale, materialType
uniform vec4	u_Local2; // ExtinctionCoefficient
uniform vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower, cubemapScale
uniform vec4	u_Local4; // haveNormalMap, isMetalic, hasRealSubsurfaceMap, sway
uniform vec4	u_Local5; // hasRealOverlayMap, overlaySway, blinnPhong, hasSteepMap
uniform vec4	u_Local6; // useSunLightSpecular, 0, 0, shadowPass
uniform vec4	u_Local9;


varying vec2      var_TexCoords;


void main()
{
	vec2 texCoords = var_TexCoords;

	if (u_Local4.a > 0.0)
	{// Sway...
		texCoords += vec2(u_Local5.y * u_Local4.a * ((1.0 - texCoords.y) + 1.0), 0.0);
	}

	gl_FragColor = texture(u_DiffuseMap, texCoords);
}

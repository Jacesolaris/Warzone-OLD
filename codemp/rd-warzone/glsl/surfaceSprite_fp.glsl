uniform sampler2D u_DiffuseMap;
varying vec4	var_Local1; // parallaxScale, haveSpecular, specularScale, materialType
varying vec4	u_Local2; // ExtinctionCoefficient
varying vec4	u_Local3; // RimScalar, MaterialThickness, subSpecPower
varying vec2	var_Dimensions;
varying vec2	var_TexCoords;

out vec4 out_Glow;
out vec4 out_Normal;

#if 0
void main()
{
	gl_FragColor = texture2D(u_DiffuseMap, var_TexCoords.xy);
	//gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);

#if defined(USE_GLOW_BUFFER)
	out_Glow = vec4(0.0);
#else
	out_Glow = vec4(0.0);
#endif
}
#else
varying lowp vec4 vColor;
varying highp vec2 vTexCoord;

void main()
{
    lowp vec4 col = texture2D( u_DiffuseMap, vTexCoord );
    if(col.r > 0.5) discard;
    gl_FragColor = col;

#if defined(USE_GLOW_BUFFER)
	out_Glow = vec4(0.0);
#else
	out_Glow = vec4(0.0);
#endif

	//out_Normal = vec4(0.0);
}
#endif
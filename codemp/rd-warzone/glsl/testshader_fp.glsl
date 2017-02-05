uniform sampler2D u_DiffuseMap;

varying vec2 var_TexCoords;

// 'threshold ' is constant , 'value ' is smoothly varying
float aastep ( float threshold , float value )
{
	float afwidth = 0.7 * length ( vec2 ( dFdx ( value ) , dFdy ( value ))) ;
	// GLSL 's fwidth ( value ) is abs ( dFdx ( value )) + abs ( dFdy ( value ))
	return smoothstep ( threshold - afwidth , threshold + afwidth , value ) ;
}

void main(void)
{
	gl_FragColor = texture(u_DiffuseMap, var_TexCoords);
}

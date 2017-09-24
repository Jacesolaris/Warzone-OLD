uniform sampler2D u_DiffuseMap;

varying vec2      var_Tex1;
varying vec4	  var_Color;

void main()
{
	gl_FragColor = texture(u_DiffuseMap, var_Tex1) * var_Color;
}

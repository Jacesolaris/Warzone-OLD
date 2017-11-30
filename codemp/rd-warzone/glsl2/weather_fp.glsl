uniform sampler2D u_DiffuseMap;

out vec4 out_Glow;
out vec4 out_Position;
out vec4 out_Normal;
out vec4 out_NormalDetail;

varying vec2      var_Tex1;
varying vec4	  var_Color;

void main()
{
	gl_FragColor = texture2D(u_DiffuseMap, var_Tex1) * var_Color;
	out_Glow = vec4(0.0);
	out_Position = vec4(0.0);
	out_Normal = vec4(0.0);
	out_NormalDetail = vec4(0.0);
}

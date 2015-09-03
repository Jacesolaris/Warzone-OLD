uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;
varying vec2 var_ScreenTex;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1 = texture2D (u_DiffuseMap, var_ScreenTex);
  gl_FragColor.w = tmpvar_1.w;
  gl_FragColor.xyz = ((tmpvar_1.xyz + (texture2D (u_NormalMap, var_ScreenTex).xyz * tmpvar_1.xyz)) / 2.0);
}


// stats: 3 alu 2 tex 0 flow
// inputs: 1
//  #0: var_ScreenTex (high float) 2x1 [-1]
// textures: 2
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]
//  #1: u_NormalMap (high 2d) 0x0 [-1]

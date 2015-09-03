uniform sampler2D u_TextureMap;
uniform vec2 u_InvTexRes;
varying vec2 var_TexCoords;
void main ()
{
  vec2 tc_1;
  vec4 color_2;
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-1.5, -1.5)));
  vec4 tmpvar_3;
  tmpvar_3 = texture2D (u_TextureMap, tc_1);
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.5, -1.5)));
  color_2 = (tmpvar_3 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.5, -1.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(1.5, -1.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-1.5, -0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.5, -0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.5, -0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(1.5, -0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-1.5, 0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.5, 0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.5, 0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(1.5, 0.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-1.5, 1.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.5, 1.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.5, 1.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(1.5, 1.5)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  color_2 = (color_2 * 0.0625);
  gl_FragColor = color_2;
}


// stats: 48 alu 16 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_InvTexRes (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

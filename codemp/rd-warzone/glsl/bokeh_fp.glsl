uniform sampler2D u_TextureMap;
uniform vec4 u_Color;
uniform vec2 u_InvTexRes;
varying vec2 var_TexCoords;
void main ()
{
  vec2 tc_1;
  vec4 color_2;
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(1.0, 0.0)));
  vec4 tmpvar_3;
  tmpvar_3 = texture2D (u_TextureMap, tc_1);
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.9238795, 0.3826834)));
  color_2 = (tmpvar_3 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.7071068, 0.7071068)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.3826834, 0.9238795)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.0, 1.0)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.9238795, -0.3826834)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.7071068, -0.7071068)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.3826834, -0.9238795)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(0.0, -1.0)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-1.0, 0.0)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.9238795, 0.3826834)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.7071068, 0.7071068)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.3826834, 0.9238795)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.9238795, -0.3826834)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.7071068, -0.7071068)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  tc_1 = (var_TexCoords + (u_InvTexRes * vec2(-0.3826834, -0.9238795)));
  color_2 = (color_2 + texture2D (u_TextureMap, tc_1));
  gl_FragColor = ((color_2 * 0.0625) * u_Color);
}


// stats: 49 alu 16 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 2 (total size: 0)
//  #0: u_Color (high float) 4x1 [-1]
//  #1: u_InvTexRes (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
void main ()
{
  float invscreensize_1;
  vec4 tcol_2;
  vec4 coord_3;
  vec4 res_4;
  coord_3.z = 0.0;
  coord_3.w = 0.0;
  vec4 tmpvar_5;
  tmpvar_5 = texture2D (u_TextureMap, var_TexCoords);
  invscreensize_1 = (1.0/(var_Dimensions.x));
  coord_3.xy = (var_TexCoords + vec2(invscreensize_1));
  tcol_2 = (tmpvar_5 + texture2D (u_TextureMap, coord_3.xy));
  coord_3.xy = (var_TexCoords + vec2(-(invscreensize_1)));
  tcol_2 = (tcol_2 + texture2D (u_TextureMap, coord_3.xy));
  coord_3.xy = (var_TexCoords + (vec2(-1.0, 1.0) * invscreensize_1));
  tcol_2 = (tcol_2 + texture2D (u_TextureMap, coord_3.xy));
  coord_3.xy = (var_TexCoords + (vec2(1.0, -1.0) * invscreensize_1));
  tcol_2 = (tcol_2 + texture2D (u_TextureMap, coord_3.xy));
  tcol_2 = (tcol_2 * 0.2);
  res_4 = (tmpvar_5 * (1.0 + (
    (tmpvar_5 - tcol_2)
   * 2.3)));
  res_4.xyz = mix (res_4, tmpvar_5, clamp (pow (tmpvar_5.z, 3.0), 0.0, 1.0)).xyz;
  res_4.w = 1.0;
  gl_FragColor = res_4;
}


// stats: 23 alu 5 tex 0 flow
// inputs: 2
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

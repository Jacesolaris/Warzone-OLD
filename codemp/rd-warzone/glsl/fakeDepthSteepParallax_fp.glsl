uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
varying vec4 var_Local0;
void main ()
{
  vec2 Coord_1;
  vec2 tmpvar_2;
  tmpvar_2 = (1.0/(var_Dimensions));
  Coord_1 = var_TexCoords;
  float combined_color_3;
  vec3 tmpvar_4;
  tmpvar_4 = clamp ((texture2D (u_TextureMap, var_TexCoords).xyz * 1.33333), 0.0, 1.0);
  float tmpvar_5;
  tmpvar_5 = ((tmpvar_4.x + tmpvar_4.y) + tmpvar_4.z);
  combined_color_3 = tmpvar_5;
  if ((tmpvar_5 > 3.0)) {
    combined_color_3 = (tmpvar_5 / 10.0);
  } else {
    if ((combined_color_3 > 2.0)) {
      combined_color_3 = (combined_color_3 / 7.0);
    } else {
      if ((combined_color_3 > 1.0)) {
        combined_color_3 = (combined_color_3 / 5.0);
      };
    };
  };
  float combined_color_6;
  vec3 tmpvar_7;
  tmpvar_7 = clamp ((texture2D (u_TextureMap, var_TexCoords).xyz * 1.33333), 0.0, 1.0);
  float tmpvar_8;
  tmpvar_8 = ((tmpvar_7.x + tmpvar_7.y) + tmpvar_7.z);
  combined_color_6 = tmpvar_8;
  if ((tmpvar_8 > 3.0)) {
    combined_color_6 = (tmpvar_8 / 10.0);
  } else {
    if ((combined_color_6 > 2.0)) {
      combined_color_6 = (combined_color_6 / 7.0);
    } else {
      if ((combined_color_6 > 1.0)) {
        combined_color_6 = (combined_color_6 / 5.0);
      };
    };
  };
  Coord_1 = (var_TexCoords + ((tmpvar_2 * 
    (((-(combined_color_6) * 2.0) - 1.0) * 0.33333)
  ) * var_Local0.y));
  gl_FragColor = texture2D (u_TextureMap, Coord_1);
}


// stats: 28 alu 3 tex 6 flow
// inputs: 3
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_Local0 (high float) 4x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

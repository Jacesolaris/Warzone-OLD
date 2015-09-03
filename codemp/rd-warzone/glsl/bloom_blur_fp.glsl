uniform sampler2D u_DiffuseMap;
varying vec4 var_Local0;
varying vec2 var_Dimensions;
varying vec2 var_TexCoords;
void main ()
{
  vec3 color2_2;
  vec3 col0_4;
  vec3 color_5;
  vec2 PIXEL_OFFSET_6;
  PIXEL_OFFSET_6 = (1.0/(var_Dimensions));
  color_5 = vec3(0.0, 0.0, 0.0);
  col0_4 = texture2D (u_DiffuseMap, var_TexCoords).xyz;
  for (float width_3 = 0.0; width_3 < var_Local0.z; width_3 += 1.0) {
    color_5 = (color_5 + ((col0_4 / 2.0) + (
      (texture2D (u_DiffuseMap, (var_TexCoords + ((vec2(1.0, 0.0) * width_3) * PIXEL_OFFSET_6))).xyz + texture2D (u_DiffuseMap, (var_TexCoords - ((vec2(1.0, 0.0) * width_3) * PIXEL_OFFSET_6))).xyz)
     / 4.0)));
  };
  color_5 = (color_5 / var_Local0.z);
  color2_2 = vec3(0.0, 0.0, 0.0);
  for (float width_1 = 0.0; width_1 < var_Local0.z; width_1 += 1.0) {
    color2_2 = (color2_2 + ((col0_4 / 2.0) + (
      (texture2D (u_DiffuseMap, (var_TexCoords + ((vec2(0.0, 1.0) * width_1) / var_Dimensions))).xyz + texture2D (u_DiffuseMap, (var_TexCoords - ((vec2(0.0, 1.0) * width_1) / var_Dimensions))).xyz)
     / 4.0)));
  };
  color2_2 = (color2_2 / var_Local0.z);
  gl_FragColor.xyz = ((color_5 + color2_2) / 2.0);
  gl_FragColor.w = 1.0;
}


// stats: 36 alu 5 tex 4 flow
// inputs: 3
//  #0: var_Local0 (high float) 4x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_TexCoords (high float) 2x1 [-1]
// textures: 1
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]

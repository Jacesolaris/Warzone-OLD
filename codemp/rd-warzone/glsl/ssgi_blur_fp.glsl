uniform sampler2D u_DiffuseMap;
varying vec4 var_Local0;
varying vec2 var_Dimensions;
varying vec2 var_TexCoords;
void main ()
{
  vec3 col0_2;
  vec2 PIXEL_OFFSET_3;
  float NUM_VALUES_4;
  NUM_VALUES_4 = 1.0;
  PIXEL_OFFSET_3 = (1.0/(var_Dimensions));
  vec3 tmpvar_5;
  tmpvar_5 = texture2D (u_DiffuseMap, var_TexCoords).xyz;
  col0_2 = tmpvar_5;
  gl_FragColor.xyz = tmpvar_5;
  for (float width_1 = 1.0; width_1 <= var_Local0.z; width_1 += 1.0) {
    vec3 tmpvar_6;
    tmpvar_6 = (((col0_2 / 2.0) + (
      (texture2D (u_DiffuseMap, (var_TexCoords + ((var_Local0.xy * width_1) * PIXEL_OFFSET_3))).xyz + texture2D (u_DiffuseMap, (var_TexCoords - ((var_Local0.xy * width_1) * PIXEL_OFFSET_3))).xyz)
     * 
      (clamp ((1.0 - (width_1 / var_Local0.z)), 0.0, 1.0) * 2.0)
    )) / 4.0);
    gl_FragColor.xyz = (gl_FragColor.xyz + clamp ((tmpvar_6 * 
      clamp (((3.0 - (
        (tmpvar_6.x + tmpvar_6.y)
       + tmpvar_6.z)) + 0.1), 0.0, 3.0)
    ), 0.0, 1.0));
    NUM_VALUES_4 += 1.0;
  };
  gl_FragColor.xyz = (gl_FragColor.xyz / NUM_VALUES_4);
  gl_FragColor.w = 1.0;
}


// stats: 31 alu 3 tex 2 flow
// inputs: 3
//  #0: var_Local0 (high float) 4x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_TexCoords (high float) 2x1 [-1]
// textures: 1
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]

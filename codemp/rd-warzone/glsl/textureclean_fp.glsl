uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
varying vec4 var_Local0;
void main ()
{
  float bZ_2;
  float factor_3;
  float Z_4;
  vec3 final_colour_5;
  float kernel_6[4];
  vec3 c_7;
  vec2 offset_8;
  vec2 coord_9;
  float BSIGMA_10;
  BSIGMA_10 = var_Local0.y;
  coord_9 = var_TexCoords;
  vec2 tmpvar_11;
  tmpvar_11 = (1.0/(var_Dimensions));
  offset_8 = tmpvar_11;
  vec4 tmpvar_12;
  tmpvar_12 = texture2D (u_TextureMap, (var_TexCoords - (vec2(0.0, -1.0) * tmpvar_11)));
  c_7 = tmpvar_12.xyz;
  final_colour_5 = vec3(0.0, 0.0, 0.0);
  Z_4 = 0.0;
  float tmpvar_13;
  float tmpvar_14;
  tmpvar_14 = (var_Local0.x * var_Local0.x);
  tmpvar_13 = ((0.39894 * exp(
    (-0.0 / tmpvar_14)
  )) / var_Local0.x);
  kernel_6[1] = tmpvar_13;
  kernel_6[1] = tmpvar_13;
  float tmpvar_15;
  tmpvar_15 = ((0.39894 * exp(
    (-0.5 / tmpvar_14)
  )) / var_Local0.x);
  kernel_6[(1 - 1)] = tmpvar_15;
  kernel_6[(1 + 1)] = tmpvar_15;
  bZ_2 = (1.0/(((0.39894 * 
    exp((-0.0 / (var_Local0.y * var_Local0.y)))
  ) / var_Local0.y)));
  for (int i_1 = -1; i_1 <= 1; i_1++) {
    vec2 tmpvar_16;
    tmpvar_16.x = float(i_1);
    tmpvar_16.y = -1.0;
    vec4 tmpvar_17;
    tmpvar_17 = texture2D (u_TextureMap, ((coord_9 + (tmpvar_16 * offset_8)) - (vec2(0.0, 1.0) * offset_8)));
    vec3 v_18;
    v_18 = (tmpvar_17.xyz - c_7);
    factor_3 = (((
      ((0.39894 * exp((
        (-0.5 * dot (v_18, v_18))
       / 
        (BSIGMA_10 * BSIGMA_10)
      ))) / BSIGMA_10)
     * bZ_2) * kernel_6[0]) * kernel_6[(1 + i_1)]);
    Z_4 = (Z_4 + factor_3);
    final_colour_5 = (final_colour_5 + (factor_3 * tmpvar_17.xyz));
    vec2 tmpvar_19;
    tmpvar_19.x = float(i_1);
    tmpvar_19.y = 0.0;
    vec4 tmpvar_20;
    tmpvar_20 = texture2D (u_TextureMap, ((coord_9 + (tmpvar_19 * offset_8)) - (vec2(0.0, 1.0) * offset_8)));
    vec3 v_21;
    v_21 = (tmpvar_20.xyz - c_7);
    factor_3 = (((
      ((0.39894 * exp((
        (-0.5 * dot (v_21, v_21))
       / 
        (BSIGMA_10 * BSIGMA_10)
      ))) / BSIGMA_10)
     * bZ_2) * kernel_6[1]) * kernel_6[(1 + i_1)]);
    Z_4 = (Z_4 + factor_3);
    final_colour_5 = (final_colour_5 + (factor_3 * tmpvar_20.xyz));
    vec2 tmpvar_22;
    tmpvar_22.x = float(i_1);
    tmpvar_22.y = 1.0;
    vec4 tmpvar_23;
    tmpvar_23 = texture2D (u_TextureMap, ((coord_9 + (tmpvar_22 * offset_8)) - (vec2(0.0, 1.0) * offset_8)));
    vec3 v_24;
    v_24 = (tmpvar_23.xyz - c_7);
    factor_3 = (((
      ((0.39894 * exp((
        (-0.5 * dot (v_24, v_24))
       / 
        (BSIGMA_10 * BSIGMA_10)
      ))) / BSIGMA_10)
     * bZ_2) * kernel_6[2]) * kernel_6[(1 + i_1)]);
    Z_4 = (Z_4 + factor_3);
    final_colour_5 = (final_colour_5 + (factor_3 * tmpvar_23.xyz));
  };
  vec4 tmpvar_25;
  tmpvar_25.xyz = (final_colour_5 / Z_4);
  tmpvar_25.w = tmpvar_12.w;
  gl_FragColor = tmpvar_25;
}


// stats: 89 alu 4 tex 2 flow
// inputs: 3
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_Local0 (high float) 4x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

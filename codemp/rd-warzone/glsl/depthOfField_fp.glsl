uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;
in vec2 var_TexCoords;
in vec2 var_Dimensions;
in vec4 var_ViewInfo;
in vec4 var_Local0;
vec2 texel;
bool autofocus;
void main ()
{
  texel = (1.0/(var_Dimensions));
  autofocus = bool(0);
  vec3 col_1;
  float h_2;
  float w_3;
  float blur_4;
  float fDepth_5;
  if ((var_Local0.x >= 2.0)) {
    autofocus = bool(1);
  };
  float tmpvar_6;
  tmpvar_6 = ((-(var_ViewInfo.y) * var_ViewInfo.x) / ((
    (texture (u_ScreenDepthMap, var_TexCoords).x * 255.0)
   * 
    (var_ViewInfo.y - var_ViewInfo.x)
  ) - var_ViewInfo.y));
  fDepth_5 = -0.01581;
  if (autofocus) {
    fDepth_5 = ((-(var_ViewInfo.y) * var_ViewInfo.x) / ((
      (texture (u_ScreenDepthMap, vec2(0.5, 0.5)).x * 255.0)
     * 
      (var_ViewInfo.y - var_ViewInfo.x)
    ) - var_ViewInfo.y));
  };
  if ((!(autofocus) && (tmpvar_6 <= fDepth_5))) {
    gl_FragColor.xyz = texture (u_TextureMap, var_TexCoords).xyz;
    gl_FragColor.w = 1.0;
    return;
  };
  float tmpvar_7;
  tmpvar_7 = (fDepth_5 * 1000.0);
  float tmpvar_8;
  tmpvar_8 = (tmpvar_6 * 1000.0);
  blur_4 = (abs((
    ((tmpvar_8 * 12.0) / (tmpvar_8 - 12.0))
   - 
    ((tmpvar_7 * 12.0) / (tmpvar_7 - 12.0))
  )) * ((tmpvar_7 - 12.0) / (0.06 * tmpvar_7)));
  if ((tmpvar_6 <= fDepth_5)) {
    blur_4 = (blur_4 / 12.0);
  } else {
    if (!(autofocus)) {
      float tmpvar_9;
      tmpvar_9 = (tmpvar_6 / fDepth_5);
      blur_4 = (blur_4 * ((
        (tmpvar_9 + tmpvar_9)
       / 0.5) / tmpvar_9));
    } else {
      float tmpvar_10;
      tmpvar_10 = (tmpvar_6 / fDepth_5);
      blur_4 = (blur_4 * ((tmpvar_10 + tmpvar_10) / 3.0));
    };
  };
  if ((autofocus && (-(tmpvar_6) <= 0.01578))) {
    float blur2_11;
    blur2_11 = (0.5 * (-0.011 / tmpvar_6));
    if ((blur2_11 > blur_4)) {
      blur_4 = blur2_11;
    };
  };
  float tmpvar_12;
  tmpvar_12 = clamp (blur_4, 0.0, 1.0);
  blur_4 = tmpvar_12;
  vec2 tmpvar_13;
  float tmpvar_14;
  tmpvar_14 = (var_Dimensions.x / 2.0);
  float tmpvar_15;
  tmpvar_15 = (var_Dimensions.y / 2.0);
  tmpvar_13.x = (((
    (fract((1.0 - (var_TexCoords.x * tmpvar_14))) * 0.25)
   + 
    (fract((var_TexCoords.y * tmpvar_15)) * 0.75)
  ) * 2.0) - 1.0);
  tmpvar_13.y = (((
    (fract((1.0 - (var_TexCoords.x * tmpvar_14))) * 0.75)
   + 
    (fract((var_TexCoords.y * tmpvar_15)) * 0.25)
  ) * 2.0) - 1.0);
  vec2 tmpvar_16;
  tmpvar_16 = ((tmpvar_13 * 0.0001) * tmpvar_12);
  w_3 = (((1.0/(var_Dimensions.x)) * tmpvar_12) + tmpvar_16.x);
  h_2 = (((1.0/(var_Dimensions.y)) * tmpvar_12) + tmpvar_16.y);
  col_1 = vec3(0.0, 0.0, 0.0);
  if ((tmpvar_12 < 0.05)) {
    col_1 = texture (u_TextureMap, var_TexCoords).xyz;
  } else {
    int ringsamples_18;
    float s_19;
    col_1 = texture (u_TextureMap, var_TexCoords).xyz;
    s_19 = 1.0;
    for (int i_17 = 1; i_17 <= 4; i_17++) {
      ringsamples_18 = (i_17 * 4);
      for (int j_20 = 0; j_20 < ringsamples_18; j_20++) {
        float tmpvar_21;
        tmpvar_21 = (6.283185 / float(ringsamples_18));
        float tmpvar_22;
        tmpvar_22 = (cos((
          float(j_20)
         * tmpvar_21)) * float(i_17));
        float tmpvar_23;
        tmpvar_23 = (sin((
          float(j_20)
         * tmpvar_21)) * float(i_17));
        vec2 tmpvar_24;
        tmpvar_24.x = tmpvar_22;
        tmpvar_24.y = tmpvar_23;
        float inorout_25;
        vec4 dist_26;
        vec4 tmpvar_27;
        tmpvar_27.zw = vec2(2.7, 2.7);
        tmpvar_27.xy = tmpvar_24;
        dist_26.x = dot (tmpvar_27, vec4(1.0, 0.0, 0.0, 1.0));
        dist_26.y = dot (tmpvar_27, vec4(0.309017, 0.9510565, 0.0, 1.0));
        dist_26.z = dot (tmpvar_27, vec4(-0.809017, 0.5877852, 0.0, 1.0));
        dist_26.w = dot (tmpvar_27, vec4(-0.809017, -0.5877852, 0.0, 1.0));
        vec4 tmpvar_28;
        vec4 tmpvar_29;
        tmpvar_29 = clamp (((dist_26 - -0.4) / 0.8), 0.0, 1.0);
        tmpvar_28 = (tmpvar_29 * (tmpvar_29 * (3.0 - 
          (2.0 * tmpvar_29)
        )));
        dist_26.zw = tmpvar_28.zw;
        inorout_25 = (-4.0 + dot (tmpvar_28, vec4(1.0, 1.0, 1.0, 1.0)));
        dist_26.x = dot (tmpvar_27, vec4(0.309017, -0.9510565, 0.0, 1.0));
        dist_26.y = -1.7;
        vec4 tmpvar_30;
        vec4 tmpvar_31;
        tmpvar_31 = clamp (((dist_26 - -0.4) / 0.8), 0.0, 1.0);
        tmpvar_30 = (tmpvar_31 * (tmpvar_31 * (3.0 - 
          (2.0 * tmpvar_31)
        )));
        dist_26 = tmpvar_30;
        inorout_25 = (inorout_25 + tmpvar_30.x);
        float tmpvar_32;
        tmpvar_32 = clamp (inorout_25, 0.0, 1.0);
        vec2 tmpvar_33;
        tmpvar_33.x = (tmpvar_22 * w_3);
        tmpvar_33.y = (tmpvar_23 * h_2);
        vec2 coords_34;
        coords_34 = (var_TexCoords + tmpvar_33);
        vec3 col_35;
        col_35.x = texture (u_TextureMap, (coords_34 + ((vec2(0.0, 0.7) * texel) * blur_4))).x;
        col_35.y = texture (u_TextureMap, (coords_34 + ((vec2(-0.6062, -0.35) * texel) * blur_4))).y;
        col_35.z = texture (u_TextureMap, (coords_34 + ((vec2(0.6062, -0.35) * texel) * blur_4))).z;
        col_1 = (col_1 + ((
          (col_35 + (col_35 * (max (
            ((dot (col_35, vec3(0.299, 0.587, 0.114)) - 0.95) * 100.0)
          , 0.0) * blur_4)))
         * 
          mix (1.0, (float(i_17) / 4.0), 0.5)
        ) * tmpvar_32));
        s_19 = (s_19 + (mix (1.0, 
          (float(i_17) / 4.0)
        , 0.5) * tmpvar_32));
      };
    };
    col_1 = (col_1 / s_19);
  };
  gl_FragColor.xyz = col_1;
  gl_FragColor.w = 1.0;
}


// stats: 165 alu 8 tex 13 flow
// inputs: 4
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_ViewInfo (high float) 4x1 [-1]
//  #3: var_Local0 (high float) 4x1 [-1]
// textures: 2
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]

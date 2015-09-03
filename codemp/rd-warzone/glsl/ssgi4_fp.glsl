precision mediump float;
precision lowp sampler2D;
uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;
uniform sampler2D u_GlowMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
varying vec4 var_ViewInfo;
varying vec4 var_Local0;
vec2 offset1;
vec2 offset2;
void main ()
{
  vec2 tmpvar_1;
  vec2 tmpvar_2;
  float tmpvar_3;
  tmpvar_3 = (1.0/(var_Dimensions.x));
  float tmpvar_4;
  tmpvar_4 = (1.0/(var_Dimensions.y));
  tmpvar_2.x = 0.0;
  tmpvar_2.y = tmpvar_4;
  offset1 = tmpvar_2;
  tmpvar_1.y = 0.0;
  tmpvar_1.x = tmpvar_3;
  offset2 = tmpvar_1;
  float incy2_6;
  float incx2_7;
  float incy_8;
  float incx_9;
  float hf_10;
  vec3 norm_11;
  float prof_12;
  float zNear_13;
  float zFar_14;
  vec3 fcolor_15;
  float sum_16;
  float NUM_SAMPLES_17;
  NUM_SAMPLES_17 = var_Local0.y;
  if ((var_Local0.y < 1.0)) {
    NUM_SAMPLES_17 = 1.0;
  };
  if ((var_Local0.x >= 5.0)) {
    gl_FragColor = texture2D (u_GlowMap, var_TexCoords);
    return;
  };
  sum_16 = 0.0;
  fcolor_15 = vec3(0.0, 0.0, 0.0);
  float tmpvar_18;
  tmpvar_18 = (var_ViewInfo.y / var_Dimensions.y);
  zFar_14 = tmpvar_18;
  float tmpvar_19;
  tmpvar_19 = (var_ViewInfo.x / var_Dimensions.x);
  zNear_13 = tmpvar_19;
  vec4 tmpvar_20;
  tmpvar_20 = texture2D (u_ScreenDepthMap, var_TexCoords);
  float tmpvar_21;
  tmpvar_21 = roundEven((NUM_SAMPLES_17 / (0.5 + tmpvar_20.x)));
  prof_12 = ((tmpvar_18 * tmpvar_19) / ((tmpvar_20.x * 
    (tmpvar_18 - tmpvar_19)
  ) - tmpvar_18));
  float tmpvar_22;
  tmpvar_22 = (tmpvar_20.x * 255.0);
  vec3 normal_23;
  vec3 tmpvar_24;
  tmpvar_24.xy = tmpvar_2;
  tmpvar_24.z = ((texture2D (u_ScreenDepthMap, (var_TexCoords + tmpvar_2)).x * 255.0) - tmpvar_22);
  vec3 tmpvar_25;
  tmpvar_25.xy = tmpvar_1;
  tmpvar_25.z = ((texture2D (u_ScreenDepthMap, (var_TexCoords + tmpvar_1)).x * 255.0) - tmpvar_22);
  vec3 tmpvar_26;
  tmpvar_26 = ((tmpvar_24.yzx * tmpvar_25.zxy) - (tmpvar_24.zxy * tmpvar_25.yzx));
  normal_23.xy = tmpvar_26.xy;
  normal_23.z = -(tmpvar_26.z);
  norm_11 = normalize(((
    normalize(normal_23)
   * 2.0) - vec3(1.0, 1.0, 1.0)));
  vec4 tmpvar_27;
  tmpvar_27 = texture2D (u_TextureMap, var_TexCoords);
  float tmpvar_28;
  tmpvar_28 = (tmpvar_21 / 2.0);
  hf_10 = tmpvar_28;
  incx_9 = (tmpvar_3 * 60.0);
  incy_8 = (tmpvar_4 * 60.0);
  incx2_7 = (tmpvar_3 * 8.0);
  incy2_6 = (tmpvar_4 * 8.0);
  for (float i_5 = -(tmpvar_28); i_5 < hf_10; i_5 += 1.0) {
    for (float j_29 = -(hf_10); j_29 < hf_10; j_29 += 1.0) {
      if (((i_5 != 0.0) || (j_29 != 0.0))) {
        float prof2_30;
        vec2 tmpvar_31;
        tmpvar_31.x = (i_5 * incx_9);
        tmpvar_31.y = (j_29 * incy_8);
        vec2 tmpvar_32;
        tmpvar_32 = (tmpvar_31 / prof_12);
        vec2 tmpvar_33;
        tmpvar_33.x = (i_5 * incx2_7);
        tmpvar_33.y = (j_29 * incy2_6);
        vec2 tmpvar_34;
        tmpvar_34 = (tmpvar_33 / prof_12);
        float tmpvar_35;
        tmpvar_35 = dot (var_TexCoords, vec2(12.9898, 78.233));
        prof2_30 = ((zFar_14 * zNear_13) / ((texture2D (u_ScreenDepthMap, 
          (var_TexCoords + (tmpvar_32 * (0.5 + (
            fract((sin(tmpvar_35) * 43758.55))
           * 0.5))))
        ).x * 
          (zFar_14 - zNear_13)
        ) - zFar_14));
        vec2 coord_36;
        coord_36 = (var_TexCoords + (tmpvar_34 * (0.5 + 
          (fract((sin(tmpvar_35) * 43758.55)) * 0.5)
        )));
        float tmpvar_37;
        tmpvar_37 = (texture2D (u_ScreenDepthMap, coord_36).x * 255.0);
        vec3 normal_38;
        vec3 tmpvar_39;
        tmpvar_39.xy = offset1;
        tmpvar_39.z = ((texture2D (u_ScreenDepthMap, (coord_36 + offset1)).x * 255.0) - tmpvar_37);
        vec3 tmpvar_40;
        tmpvar_40.xy = offset2;
        tmpvar_40.z = ((texture2D (u_ScreenDepthMap, (coord_36 + offset2)).x * 255.0) - tmpvar_37);
        vec3 tmpvar_41;
        tmpvar_41 = ((tmpvar_39.yzx * tmpvar_40.zxy) - (tmpvar_39.zxy * tmpvar_40.yzx));
        normal_38.xy = tmpvar_41.xy;
        normal_38.z = -(tmpvar_41.z);
        vec3 tmpvar_42;
        tmpvar_42 = normalize(((
          normalize(normal_38)
         * 2.0) - vec3(1.0, 1.0, 1.0)));
        vec3 tmpvar_43;
        tmpvar_43.xy = tmpvar_34;
        tmpvar_43.z = (prof_12 - ((zFar_14 * zNear_13) / (
          (texture2D (u_ScreenDepthMap, (var_TexCoords + (tmpvar_34 * (0.5 + 
            (fract((sin(tmpvar_35) * 43758.55)) * 0.5)
          )))).x * (zFar_14 - zNear_13))
         - zFar_14)));
        float tmpvar_44;
        tmpvar_44 = dot (normalize(-(tmpvar_34)), normalize(tmpvar_42.xy));
        if ((tmpvar_44 > 0.0)) {
          vec3 x_45;
          x_45 = (tmpvar_43 * 2.0);
          float tmpvar_46;
          tmpvar_46 = abs(sqrt(dot (x_45, x_45)));
          sum_16 = (sum_16 + clamp ((
            ((0.5 * (1.0 - dot (norm_11, tmpvar_42))) / ((3.1416 * (tmpvar_46 * tmpvar_46)) + 0.5))
           * 0.2), 0.0, 1.0));
        };
        vec4 tmpvar_47;
        float tmpvar_48;
        tmpvar_48 = dot (var_TexCoords, vec2(12.9898, 78.233));
        tmpvar_47 = texture2D (u_GlowMap, (var_TexCoords + (tmpvar_32 * (0.5 + 
          (fract((sin(tmpvar_48) * 43758.55)) * 0.5)
        ))));
        vec2 coord_49;
        coord_49 = (var_TexCoords + (tmpvar_32 * (0.5 + 
          (fract((sin(tmpvar_48) * 43758.55)) * 0.5)
        )));
        float tmpvar_50;
        tmpvar_50 = (texture2D (u_ScreenDepthMap, coord_49).x * 255.0);
        vec3 normal_51;
        vec3 tmpvar_52;
        tmpvar_52.xy = offset1;
        tmpvar_52.z = ((texture2D (u_ScreenDepthMap, (coord_49 + offset1)).x * 255.0) - tmpvar_50);
        vec3 tmpvar_53;
        tmpvar_53.xy = offset2;
        tmpvar_53.z = ((texture2D (u_ScreenDepthMap, (coord_49 + offset2)).x * 255.0) - tmpvar_50);
        vec3 tmpvar_54;
        tmpvar_54 = ((tmpvar_52.yzx * tmpvar_53.zxy) - (tmpvar_52.zxy * tmpvar_53.yzx));
        normal_51.xy = tmpvar_54.xy;
        normal_51.z = -(tmpvar_54.z);
        vec3 tmpvar_55;
        tmpvar_55 = normalize(((
          normalize(normal_51)
         * 2.0) - vec3(1.0, 1.0, 1.0)));
        vec3 tmpvar_56;
        tmpvar_56.xy = tmpvar_32;
        tmpvar_56.z = abs((prof_12 - prof2_30));
        float tmpvar_57;
        tmpvar_57 = dot (normalize(-(tmpvar_32)), normalize(tmpvar_55.xy));
        if ((tmpvar_57 > 0.0)) {
          vec3 x_58;
          x_58 = (tmpvar_56 * 2.0);
          float tmpvar_59;
          tmpvar_59 = abs(sqrt(dot (x_58, x_58)));
          fcolor_15 = (fcolor_15 + (tmpvar_47.xyz * clamp (
            ((1.0 - dot (norm_11, tmpvar_55)) / ((3.1416 * (tmpvar_59 * tmpvar_59)) + 0.5))
          , 0.0, 1.0)));
        };
      };
    };
  };
  float tmpvar_60;
  tmpvar_60 = (1.0 - clamp ((
    sqrt(dot (tmpvar_27.xyz, tmpvar_27.xyz))
   / 1.5), 0.0, 1.0));
  vec3 tmpvar_61;
  tmpvar_61 = ((tmpvar_27.xyz * (1.0 - 
    (((sum_16 / tmpvar_21) * 0.75) * tmpvar_60)
  )) + ((
    (fcolor_15 / tmpvar_21)
   * 0.5) * clamp (
    ((sqrt(dot (tmpvar_27.xyz, tmpvar_27.xyz)) * 0.333) * tmpvar_60)
  , 0.0, 1.0)));
  vec3 tmpvar_62;
  tmpvar_62 = clamp (texture2D (u_GlowMap, var_TexCoords).xyz, 0.0, 1.0);
  vec3 tmpvar_63;
  tmpvar_63 = clamp ((tmpvar_62 * clamp (
    ((3.0 - ((tmpvar_62.x + tmpvar_62.y) + tmpvar_62.z)) + 0.1)
  , 0.0, 3.0)), 0.0, 1.0);
  vec4 tmpvar_64;
  tmpvar_64.w = 1.0;
  tmpvar_64.xyz = clamp (((
    (tmpvar_61 * 5.0)
   + 
    max (clamp (((
      ((tmpvar_63 * tmpvar_63) * 0.5)
     + 1.0) * tmpvar_61), 0.0, 1.0), tmpvar_61)
  ) / 6.0), 0.0, 1.0);
  gl_FragColor = tmpvar_64;
}


// stats: 209 alu 15 tex 10 flow
// inputs: 4
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_ViewInfo (high float) 4x1 [-1]
//  #3: var_Local0 (high float) 4x1 [-1]
// textures: 3
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]
//  #2: u_GlowMap (high 2d) 0x0 [-1]

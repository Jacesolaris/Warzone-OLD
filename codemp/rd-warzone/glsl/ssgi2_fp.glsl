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
  float hf_8;
  vec3 norm_9;
  float prof_10;
  float zNear_11;
  float zFar_12;
  float sum_13;
  float NUM_SAMPLES_14;
  NUM_SAMPLES_14 = var_Local0.y;
  if ((var_Local0.y < 1.0)) {
    NUM_SAMPLES_14 = 1.0;
  };
  sum_13 = 0.0;
  float tmpvar_15;
  tmpvar_15 = (var_ViewInfo.y / var_Dimensions.y);
  zFar_12 = tmpvar_15;
  float tmpvar_16;
  tmpvar_16 = (var_ViewInfo.x / var_Dimensions.x);
  zNear_11 = tmpvar_16;
  vec4 tmpvar_17;
  tmpvar_17 = texture2D (u_ScreenDepthMap, var_TexCoords);
  float tmpvar_18;
  tmpvar_18 = roundEven((NUM_SAMPLES_14 / (0.5 + tmpvar_17.x)));
  prof_10 = ((tmpvar_15 * tmpvar_16) / ((tmpvar_17.x * 
    (tmpvar_15 - tmpvar_16)
  ) - tmpvar_15));
  float tmpvar_19;
  tmpvar_19 = (tmpvar_17.x * 255.0);
  vec3 normal_20;
  vec3 tmpvar_21;
  tmpvar_21.xy = tmpvar_2;
  tmpvar_21.z = ((texture2D (u_ScreenDepthMap, (var_TexCoords + tmpvar_2)).x * 255.0) - tmpvar_19);
  vec3 tmpvar_22;
  tmpvar_22.xy = tmpvar_1;
  tmpvar_22.z = ((texture2D (u_ScreenDepthMap, (var_TexCoords + tmpvar_1)).x * 255.0) - tmpvar_19);
  vec3 tmpvar_23;
  tmpvar_23 = ((tmpvar_21.yzx * tmpvar_22.zxy) - (tmpvar_21.zxy * tmpvar_22.yzx));
  normal_20.xy = tmpvar_23.xy;
  normal_20.z = -(tmpvar_23.z);
  norm_9 = normalize(((
    normalize(normal_20)
   * 2.0) - vec3(1.0, 1.0, 1.0)));
  vec4 tmpvar_24;
  tmpvar_24 = texture2D (u_TextureMap, var_TexCoords);
  float tmpvar_25;
  tmpvar_25 = (tmpvar_18 / 2.0);
  hf_8 = tmpvar_25;
  incx2_7 = (tmpvar_3 * 8.0);
  incy2_6 = (tmpvar_4 * 8.0);
  for (float i_5 = -(tmpvar_25); i_5 < hf_8; i_5 += 1.0) {
    for (float j_26 = -(hf_8); j_26 < hf_8; j_26 += 1.0) {
      if (((i_5 != 0.0) || (j_26 != 0.0))) {
        vec2 tmpvar_27;
        tmpvar_27.x = (i_5 * incx2_7);
        tmpvar_27.y = (j_26 * incy2_6);
        vec2 tmpvar_28;
        tmpvar_28 = (tmpvar_27 / prof_10);
        float tmpvar_29;
        tmpvar_29 = dot (var_TexCoords, vec2(12.9898, 78.233));
        vec2 coord_30;
        coord_30 = (var_TexCoords + (tmpvar_28 * (0.5 + 
          (fract((sin(tmpvar_29) * 43758.55)) * 0.5)
        )));
        float tmpvar_31;
        tmpvar_31 = (texture2D (u_ScreenDepthMap, coord_30).x * 255.0);
        vec3 normal_32;
        vec3 tmpvar_33;
        tmpvar_33.xy = offset1;
        tmpvar_33.z = ((texture2D (u_ScreenDepthMap, (coord_30 + offset1)).x * 255.0) - tmpvar_31);
        vec3 tmpvar_34;
        tmpvar_34.xy = offset2;
        tmpvar_34.z = ((texture2D (u_ScreenDepthMap, (coord_30 + offset2)).x * 255.0) - tmpvar_31);
        vec3 tmpvar_35;
        tmpvar_35 = ((tmpvar_33.yzx * tmpvar_34.zxy) - (tmpvar_33.zxy * tmpvar_34.yzx));
        normal_32.xy = tmpvar_35.xy;
        normal_32.z = -(tmpvar_35.z);
        vec3 tmpvar_36;
        tmpvar_36 = normalize(((
          normalize(normal_32)
         * 2.0) - vec3(1.0, 1.0, 1.0)));
        vec3 tmpvar_37;
        tmpvar_37.xy = tmpvar_28;
        tmpvar_37.z = (prof_10 - ((zFar_12 * zNear_11) / (
          (texture2D (u_ScreenDepthMap, (var_TexCoords + (tmpvar_28 * (0.5 + 
            (fract((sin(tmpvar_29) * 43758.55)) * 0.5)
          )))).x * (zFar_12 - zNear_11))
         - zFar_12)));
        float tmpvar_38;
        tmpvar_38 = dot (normalize(-(tmpvar_28)), normalize(tmpvar_36.xy));
        if ((tmpvar_38 > 0.0)) {
          vec3 x_39;
          x_39 = (tmpvar_37 * 2.0);
          float tmpvar_40;
          tmpvar_40 = abs(sqrt(dot (x_39, x_39)));
          sum_13 = (sum_13 + clamp ((
            ((0.5 * (1.0 - dot (norm_9, tmpvar_36))) / ((3.1416 * (tmpvar_40 * tmpvar_40)) + 0.5))
           * 0.2), 0.0, 1.0));
        };
      };
    };
  };
  vec3 tmpvar_41;
  tmpvar_41 = (tmpvar_24.xyz * (1.0 - (
    ((sum_13 / tmpvar_18) * 0.75)
   * 
    (1.0 - clamp ((sqrt(
      dot (tmpvar_24.xyz, tmpvar_24.xyz)
    ) / 1.5), 0.0, 1.0))
  )));
  vec3 tmpvar_42;
  tmpvar_42 = clamp (texture2D (u_GlowMap, var_TexCoords).xyz, 0.0, 1.0);
  vec3 tmpvar_43;
  tmpvar_43 = clamp ((tmpvar_42 * clamp (
    ((3.0 - ((tmpvar_42.x + tmpvar_42.y) + tmpvar_42.z)) + 0.1)
  , 0.0, 3.0)), 0.0, 1.0);
  vec4 tmpvar_44;
  tmpvar_44.w = 1.0;
  tmpvar_44.xyz = clamp (((
    (tmpvar_41 * 5.0)
   + 
    max (clamp (((
      ((tmpvar_43 * tmpvar_43) * 0.5)
     + 1.0) * tmpvar_41), 0.0, 1.0), tmpvar_41)
  ) / 6.0), 0.0, 1.0);
  gl_FragColor = tmpvar_44;
}


// stats: 131 alu 9 tex 7 flow
// inputs: 4
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_ViewInfo (high float) 4x1 [-1]
//  #3: var_Local0 (high float) 4x1 [-1]
// textures: 3
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]
//  #2: u_GlowMap (high 2d) 0x0 [-1]

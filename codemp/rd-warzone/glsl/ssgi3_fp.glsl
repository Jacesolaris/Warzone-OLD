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
  float incy_6;
  float incx_7;
  float hf_8;
  vec3 norm_9;
  float prof_10;
  float zNear_11;
  float zFar_12;
  vec3 fcolor_13;
  float NUM_SAMPLES_14;
  NUM_SAMPLES_14 = var_Local0.y;
  if ((var_Local0.y < 1.0)) {
    NUM_SAMPLES_14 = 1.0;
  };
  fcolor_13 = vec3(0.0, 0.0, 0.0);
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
  incx_7 = (tmpvar_3 * 60.0);
  incy_6 = (tmpvar_4 * 60.0);
  for (float i_5 = -(tmpvar_25); i_5 < hf_8; i_5 += 1.0) {
    for (float j_26 = -(hf_8); j_26 < hf_8; j_26 += 1.0) {
      if (((i_5 != 0.0) || (j_26 != 0.0))) {
        vec2 tmpvar_27;
        tmpvar_27.x = (i_5 * incx_7);
        tmpvar_27.y = (j_26 * incy_6);
        vec2 tmpvar_28;
        tmpvar_28 = (tmpvar_27 / prof_10);
        float tmpvar_29;
        tmpvar_29 = dot (var_TexCoords, vec2(12.9898, 78.233));
        vec4 tmpvar_30;
        tmpvar_30 = texture2D (u_GlowMap, (var_TexCoords + (tmpvar_28 * (0.5 + 
          (fract((sin(tmpvar_29) * 43758.55)) * 0.5)
        ))));
        vec2 coord_31;
        coord_31 = (var_TexCoords + (tmpvar_28 * (0.5 + 
          (fract((sin(tmpvar_29) * 43758.55)) * 0.5)
        )));
        float tmpvar_32;
        tmpvar_32 = (texture2D (u_ScreenDepthMap, coord_31).x * 255.0);
        vec3 normal_33;
        vec3 tmpvar_34;
        tmpvar_34.xy = offset1;
        tmpvar_34.z = ((texture2D (u_ScreenDepthMap, (coord_31 + offset1)).x * 255.0) - tmpvar_32);
        vec3 tmpvar_35;
        tmpvar_35.xy = offset2;
        tmpvar_35.z = ((texture2D (u_ScreenDepthMap, (coord_31 + offset2)).x * 255.0) - tmpvar_32);
        vec3 tmpvar_36;
        tmpvar_36 = ((tmpvar_34.yzx * tmpvar_35.zxy) - (tmpvar_34.zxy * tmpvar_35.yzx));
        normal_33.xy = tmpvar_36.xy;
        normal_33.z = -(tmpvar_36.z);
        vec3 tmpvar_37;
        tmpvar_37 = normalize(((
          normalize(normal_33)
         * 2.0) - vec3(1.0, 1.0, 1.0)));
        vec3 tmpvar_38;
        tmpvar_38.xy = tmpvar_28;
        tmpvar_38.z = abs((prof_10 - (
          (zFar_12 * zNear_11)
         / 
          ((texture2D (u_ScreenDepthMap, (var_TexCoords + (tmpvar_28 * 
            (0.5 + (fract((
              sin(tmpvar_29)
             * 43758.55)) * 0.5))
          ))).x * (zFar_12 - zNear_11)) - zFar_12)
        )));
        float tmpvar_39;
        tmpvar_39 = dot (normalize(-(tmpvar_28)), normalize(tmpvar_37.xy));
        if ((tmpvar_39 > 0.0)) {
          vec3 x_40;
          x_40 = (tmpvar_38 * 2.0);
          float tmpvar_41;
          tmpvar_41 = abs(sqrt(dot (x_40, x_40)));
          fcolor_13 = (fcolor_13 + (tmpvar_30.xyz * clamp (
            ((1.0 - dot (norm_9, tmpvar_37)) / ((3.1416 * (tmpvar_41 * tmpvar_41)) + 0.5))
          , 0.0, 1.0)));
        };
      };
    };
  };
  vec3 tmpvar_42;
  tmpvar_42 = (tmpvar_24.xyz + ((
    (fcolor_13 / tmpvar_18)
   * 0.5) * clamp (
    ((sqrt(dot (tmpvar_24.xyz, tmpvar_24.xyz)) * 0.333) * (1.0 - clamp ((
      sqrt(dot (tmpvar_24.xyz, tmpvar_24.xyz))
     / 1.5), 0.0, 1.0)))
  , 0.0, 1.0)));
  vec3 tmpvar_43;
  tmpvar_43 = clamp (texture2D (u_GlowMap, var_TexCoords).xyz, 0.0, 1.0);
  vec3 tmpvar_44;
  tmpvar_44 = clamp ((tmpvar_43 * clamp (
    ((3.0 - ((tmpvar_43.x + tmpvar_43.y) + tmpvar_43.z)) + 0.1)
  , 0.0, 3.0)), 0.0, 1.0);
  vec4 tmpvar_45;
  tmpvar_45.w = 1.0;
  tmpvar_45.xyz = clamp (((
    (tmpvar_42 * 5.0)
   + 
    max (clamp (((
      ((tmpvar_44 * tmpvar_44) * 0.5)
     + 1.0) * tmpvar_42), 0.0, 1.0), tmpvar_42)
  ) / 6.0), 0.0, 1.0);
  gl_FragColor = tmpvar_45;
}


// stats: 142 alu 10 tex 7 flow
// inputs: 4
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_ViewInfo (high float) 4x1 [-1]
//  #3: var_Local0 (high float) 4x1 [-1]
// textures: 3
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]
//  #2: u_GlowMap (high 2d) 0x0 [-1]

uniform sampler2D u_TextureMap;
varying float total_time;
varying vec2 vTexCoord0;
varying vec2 resolution;
float time;
float swim;
void main ()
{
  float tmpvar_1;
  tmpvar_1 = ((total_time * 0.1) + 32.2);
  time = tmpvar_1;
  swim = (((tmpvar_1 * 0.3) + (
    sin(((tmpvar_1 * 0.5) + 5.0))
   * 0.3)) * 3.0);
  vec4 screen_2;
  vec2 final_3;
  float tmpvar_4;
  tmpvar_4 = (vTexCoord0.x - 0.1);
  float tmpvar_5;
  tmpvar_5 = (vTexCoord0.y - 0.7);
  final_3 = (vTexCoord0 + ((
    sin((6.283185 * ((total_time / 11.66667) - (
      sqrt(((tmpvar_4 * tmpvar_4) + (tmpvar_5 * tmpvar_5)))
     / 0.7))))
   * 0.005) * (vTexCoord0 - vec2(0.1, 0.7))));
  float tmpvar_6;
  tmpvar_6 = (vTexCoord0.x - 0.8);
  float tmpvar_7;
  tmpvar_7 = (vTexCoord0.y - -0.1);
  final_3 = (final_3 + ((
    sin((6.283185 * ((total_time / 8.571429) - (
      sqrt(((tmpvar_6 * tmpvar_6) + (tmpvar_7 * tmpvar_7)))
     / 0.6))))
   * 0.005) * (vTexCoord0 - vec2(0.8, -0.1))));
  float tmpvar_8;
  tmpvar_8 = (vTexCoord0.x - 1.1);
  float tmpvar_9;
  tmpvar_9 = (vTexCoord0.y - 0.9);
  final_3 = (final_3 + ((
    sin((6.283185 * ((total_time / 16.0) - (
      sqrt(((tmpvar_8 * tmpvar_8) + (tmpvar_9 * tmpvar_9)))
     / 0.8))))
   * 0.005) * (vTexCoord0 - vec2(1.1, 0.9))));
  screen_2 = (texture2D (u_TextureMap, final_3) * 0.6);
  vec2 beam_11;
  vec3 pos_12;
  vec2 uv_13;
  vec3 col_14;
  vec2 tmpvar_15;
  tmpvar_15 = (final_3 - vec2(0.5, 0.5));
  uv_13.y = tmpvar_15.y;
  uv_13.x = (tmpvar_15.x * (resolution.x / resolution.y));
  vec3 tmpvar_16;
  tmpvar_16.z = -1.4;
  tmpvar_16.xy = uv_13;
  vec3 tmpvar_17;
  tmpvar_17 = normalize(tmpvar_16);
  vec3 tmpvar_18;
  tmpvar_18.x = 1.3;
  tmpvar_18.y = ((sin(
    (tmpvar_1 + 4.3)
  ) * 0.18) - 0.05);
  tmpvar_18.z = ((sin(
    (-(tmpvar_1) * 0.15)
  ) * 5.0) - 1.35);
  pos_12.yz = tmpvar_18.yz;
  float tmpvar_19;
  tmpvar_19 = clamp (((tmpvar_18.z - -1.15) / 1.15), 0.0, 1.0);
  pos_12.x = (1.3 - (tmpvar_19 * (tmpvar_19 * 
    (3.0 - (2.0 * tmpvar_19))
  )));
  vec2 x_20;
  x_20 = (vec2(-0.1, 0.6) - uv_13);
  float tmpvar_21;
  tmpvar_21 = max (0.0, (1.0 - sqrt(
    dot (x_20, x_20)
  )));
  vec3 tmpvar_22;
  tmpvar_22.x = pow (tmpvar_21, 1.9);
  tmpvar_22.y = tmpvar_21;
  tmpvar_22.z = pow (tmpvar_21, 0.8);
  col_14 = (tmpvar_22 * 1.3);
  col_14 = mix (col_14, vec3(0.0, 0.25, 0.45), (0.8099999 * (1.0 - tmpvar_15.y)));
  vec3 ro_23;
  ro_23 = pos_12;
  vec3 rd_24;
  rd_24 = tmpvar_17;
  float hit_25;
  int i_26;
  float dist_27;
  vec3 pos_28;
  vec2 ret_29;
  hit_25 = 0.0;
  ret_29 = vec2(0.0, 0.0);
  pos_28 = pos_12;
  dist_27 = 0.0;
  i_26 = 0;
  while (true) {
    if ((i_26 >= 118)) {
      break;
    };
    if ((((
      (hit_25 != 0.0)
     || 
      (pos_28.y < -0.3)
    ) || (pos_28.y > 0.46)) || (dist_27 > 7.0))) {
      i_26++;
      continue;
    };
    pos_28 = (ro_23 + (dist_27 * rd_24));
    vec3 p_30;
    vec2 tmpvar_31;
    vec3 tmpvar_32;
    tmpvar_32.xy = vec2(-0.5, 0.0);
    tmpvar_32.z = (3.0 - swim);
    p_30 = (pos_28 + tmpvar_32);
    p_30.x = (p_30.x + (sin(
      (((p_30.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_30.x = abs(p_30.x);
    float tmpvar_33;
    vec3 tmpvar_34;
    tmpvar_34 = (abs((p_30 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_35;
    tmpvar_35 = max (tmpvar_34, 0.0);
    tmpvar_33 = (min (max (tmpvar_34.x, 
      max (tmpvar_34.y, tmpvar_34.z)
    ), 0.0) + sqrt(dot (tmpvar_35, tmpvar_35)));
    if ((tmpvar_33 > 0.0)) {
      vec2 tmpvar_36;
      tmpvar_36.y = 0.0;
      tmpvar_36.x = tmpvar_33;
      tmpvar_31 = tmpvar_36;
    } else {
      vec2 tmpvar_37;
      tmpvar_37.y = 0.0;
      tmpvar_37.x = tmpvar_33;
      tmpvar_31 = tmpvar_37;
    };
    ret_29 = tmpvar_31;
    if ((tmpvar_31.x < 0.005)) {
      hit_25 = tmpvar_31.y;
    };
    if ((tmpvar_31.y >= 2.0)) {
      dist_27 = (dist_27 + (tmpvar_31.x * 0.35));
    } else {
      dist_27 = (dist_27 + (tmpvar_31.x * 0.7));
    };
    i_26++;
  };
  if ((hit_25 > 0.0)) {
    vec3 p_38;
    p_38 = (pos_28 + vec3(0.001, 0.0, 0.0));
    vec2 tmpvar_39;
    vec3 tmpvar_40;
    tmpvar_40.xy = vec2(-0.5, 0.0);
    tmpvar_40.z = (3.0 - swim);
    p_38 = (p_38 + tmpvar_40);
    p_38.x = (p_38.x + (sin(
      (((p_38.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_38.x = abs(p_38.x);
    float tmpvar_41;
    vec3 tmpvar_42;
    tmpvar_42 = (abs((p_38 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_43;
    tmpvar_43 = max (tmpvar_42, 0.0);
    tmpvar_41 = (min (max (tmpvar_42.x, 
      max (tmpvar_42.y, tmpvar_42.z)
    ), 0.0) + sqrt(dot (tmpvar_43, tmpvar_43)));
    if ((tmpvar_41 > 0.0)) {
      vec2 tmpvar_44;
      tmpvar_44.y = 0.0;
      tmpvar_44.x = tmpvar_41;
      tmpvar_39 = tmpvar_44;
    } else {
      vec2 tmpvar_45;
      tmpvar_45.y = 0.0;
      tmpvar_45.x = tmpvar_41;
      tmpvar_39 = tmpvar_45;
    };
    vec3 p_46;
    p_46 = (pos_28 - vec3(0.001, 0.0, 0.0));
    vec2 tmpvar_47;
    vec3 tmpvar_48;
    tmpvar_48.xy = vec2(-0.5, 0.0);
    tmpvar_48.z = (3.0 - swim);
    p_46 = (p_46 + tmpvar_48);
    p_46.x = (p_46.x + (sin(
      (((p_46.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_46.x = abs(p_46.x);
    float tmpvar_49;
    vec3 tmpvar_50;
    tmpvar_50 = (abs((p_46 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_51;
    tmpvar_51 = max (tmpvar_50, 0.0);
    tmpvar_49 = (min (max (tmpvar_50.x, 
      max (tmpvar_50.y, tmpvar_50.z)
    ), 0.0) + sqrt(dot (tmpvar_51, tmpvar_51)));
    if ((tmpvar_49 > 0.0)) {
      vec2 tmpvar_52;
      tmpvar_52.y = 0.0;
      tmpvar_52.x = tmpvar_49;
      tmpvar_47 = tmpvar_52;
    } else {
      vec2 tmpvar_53;
      tmpvar_53.y = 0.0;
      tmpvar_53.x = tmpvar_49;
      tmpvar_47 = tmpvar_53;
    };
    vec3 p_54;
    p_54 = (pos_28 + vec3(0.0, 0.001, 0.0));
    vec2 tmpvar_55;
    vec3 tmpvar_56;
    tmpvar_56.xy = vec2(-0.5, 0.0);
    tmpvar_56.z = (3.0 - swim);
    p_54 = (p_54 + tmpvar_56);
    p_54.x = (p_54.x + (sin(
      (((p_54.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_54.x = abs(p_54.x);
    float tmpvar_57;
    vec3 tmpvar_58;
    tmpvar_58 = (abs((p_54 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_59;
    tmpvar_59 = max (tmpvar_58, 0.0);
    tmpvar_57 = (min (max (tmpvar_58.x, 
      max (tmpvar_58.y, tmpvar_58.z)
    ), 0.0) + sqrt(dot (tmpvar_59, tmpvar_59)));
    if ((tmpvar_57 > 0.0)) {
      vec2 tmpvar_60;
      tmpvar_60.y = 0.0;
      tmpvar_60.x = tmpvar_57;
      tmpvar_55 = tmpvar_60;
    } else {
      vec2 tmpvar_61;
      tmpvar_61.y = 0.0;
      tmpvar_61.x = tmpvar_57;
      tmpvar_55 = tmpvar_61;
    };
    vec3 p_62;
    p_62 = (pos_28 - vec3(0.0, 0.001, 0.0));
    vec2 tmpvar_63;
    vec3 tmpvar_64;
    tmpvar_64.xy = vec2(-0.5, 0.0);
    tmpvar_64.z = (3.0 - swim);
    p_62 = (p_62 + tmpvar_64);
    p_62.x = (p_62.x + (sin(
      (((p_62.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_62.x = abs(p_62.x);
    float tmpvar_65;
    vec3 tmpvar_66;
    tmpvar_66 = (abs((p_62 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_67;
    tmpvar_67 = max (tmpvar_66, 0.0);
    tmpvar_65 = (min (max (tmpvar_66.x, 
      max (tmpvar_66.y, tmpvar_66.z)
    ), 0.0) + sqrt(dot (tmpvar_67, tmpvar_67)));
    if ((tmpvar_65 > 0.0)) {
      vec2 tmpvar_68;
      tmpvar_68.y = 0.0;
      tmpvar_68.x = tmpvar_65;
      tmpvar_63 = tmpvar_68;
    } else {
      vec2 tmpvar_69;
      tmpvar_69.y = 0.0;
      tmpvar_69.x = tmpvar_65;
      tmpvar_63 = tmpvar_69;
    };
    vec3 p_70;
    p_70 = (pos_28 + vec3(0.0, 0.0, 0.001));
    vec2 tmpvar_71;
    vec3 tmpvar_72;
    tmpvar_72.xy = vec2(-0.5, 0.0);
    tmpvar_72.z = (3.0 - swim);
    p_70 = (p_70 + tmpvar_72);
    p_70.x = (p_70.x + (sin(
      (((p_70.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_70.x = abs(p_70.x);
    float tmpvar_73;
    vec3 tmpvar_74;
    tmpvar_74 = (abs((p_70 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_75;
    tmpvar_75 = max (tmpvar_74, 0.0);
    tmpvar_73 = (min (max (tmpvar_74.x, 
      max (tmpvar_74.y, tmpvar_74.z)
    ), 0.0) + sqrt(dot (tmpvar_75, tmpvar_75)));
    if ((tmpvar_73 > 0.0)) {
      vec2 tmpvar_76;
      tmpvar_76.y = 0.0;
      tmpvar_76.x = tmpvar_73;
      tmpvar_71 = tmpvar_76;
    } else {
      vec2 tmpvar_77;
      tmpvar_77.y = 0.0;
      tmpvar_77.x = tmpvar_73;
      tmpvar_71 = tmpvar_77;
    };
    vec3 p_78;
    p_78 = (pos_28 - vec3(0.0, 0.0, 0.001));
    vec2 tmpvar_79;
    vec3 tmpvar_80;
    tmpvar_80.xy = vec2(-0.5, 0.0);
    tmpvar_80.z = (3.0 - swim);
    p_78 = (p_78 + tmpvar_80);
    p_78.x = (p_78.x + (sin(
      (((p_78.z * 2.0) + (swim * 5.33333)) + 2.1)
    ) * 0.07));
    p_78.x = abs(p_78.x);
    float tmpvar_81;
    vec3 tmpvar_82;
    tmpvar_82 = (abs((p_78 + vec3(0.0, -0.14, 0.0))) - vec3(0.92, 0.4, 1.3));
    vec3 tmpvar_83;
    tmpvar_83 = max (tmpvar_82, 0.0);
    tmpvar_81 = (min (max (tmpvar_82.x, 
      max (tmpvar_82.y, tmpvar_82.z)
    ), 0.0) + sqrt(dot (tmpvar_83, tmpvar_83)));
    if ((tmpvar_81 > 0.0)) {
      vec2 tmpvar_84;
      tmpvar_84.y = 0.0;
      tmpvar_84.x = tmpvar_81;
      tmpvar_79 = tmpvar_84;
    } else {
      vec2 tmpvar_85;
      tmpvar_85.y = 0.0;
      tmpvar_85.x = tmpvar_81;
      tmpvar_79 = tmpvar_85;
    };
    vec3 tmpvar_86;
    tmpvar_86.x = (tmpvar_39.x - tmpvar_47.x);
    tmpvar_86.y = (tmpvar_55.x - tmpvar_63.x);
    tmpvar_86.z = (tmpvar_71.x - tmpvar_79.x);
    vec3 tmpvar_87;
    tmpvar_87 = normalize(tmpvar_86);
    float i_88;
    vec3 colour_89;
    colour_89 = vec3(0.0, 0.0, 0.0);
    float tmpvar_90;
    tmpvar_90 = clamp (dot (tmpvar_87, vec3(0.2873479, 0.766261, 0.5746958)), 0.0, 1.0);
    if ((ret_29.y < 1.5)) {
      float v_91;
      v_91 = (clamp ((
        (0.1 - tmpvar_87.y)
       * 6.2), 0.3, 1.0) + 0.35);
      vec3 tmpvar_92;
      tmpvar_92.x = (v_91 * 0.8);
      tmpvar_92.y = (v_91 * 0.9);
      tmpvar_92.z = v_91;
      colour_89 = (tmpvar_92 * tmpvar_90);
    };
    colour_89 = (colour_89 + (vec3(0.0, 0.01, 0.13) * abs(tmpvar_87.y)));
    vec2 tmpvar_93;
    tmpvar_93 = (pos_28.xz * 5.3);
    i_88 = (min (min (
      max ((texture2D (u_TextureMap, ((tmpvar_93 + 
        ((texture2D (u_TextureMap, ((
          (tmpvar_93 * 5.0)
         + 
          (tmpvar_1 * 0.04)
        ) * 0.1), 2.0).z - texture2D (u_TextureMap, ((tmpvar_93 * 0.3) - (tmpvar_1 * 0.03)), 2.0).y) * 0.4)
      ) * 0.025), 0.0).x - 0.2), 0.0)
    , 1.0), 0.6) * 0.3);
    vec3 tmpvar_94;
    tmpvar_94.x = (i_88 * 0.5);
    tmpvar_94.y = i_88;
    tmpvar_94.z = i_88;
    colour_89 = (colour_89 + (tmpvar_94 * max (tmpvar_87.y, 0.0)));
    vec3 x_95;
    x_95 = (pos_12 - pos_28);
    col_14 = mix (colour_89, vec3(0.05, 0.31, 0.49), clamp ((
      max ((sqrt(dot (x_95, x_95)) - 0.5), 0.0)
     * 0.15), 0.0, 1.0));
  };
  beam_11.y = tmpvar_17.y;
  beam_11.x = (tmpvar_17.x * ((
    -(tmpvar_17.y)
   - 0.6) * 0.8));
  col_14 = (col_14 + (vec3(clamp (
    ((((
      (-(sin((
        ((tmpvar_17.y * 12.0) + (beam_11.x * 13.0))
       + 
        (tmpvar_1 * 0.53)
      ))) * 0.1)
     - 
      (sin(((tmpvar_17.y + 
        (beam_11.x * 17.0)
      ) + (tmpvar_1 * 0.6))) * 0.1)
    ) - (
      cos(((beam_11.x * 13.0) - (tmpvar_1 * 0.4)))
     * 0.1)) - (sin(
      ((-(beam_11.x) * 52.23) + (tmpvar_1 * 1.8))
    ) * 0.1)) * max (0.0, (1.0 - texture2D (u_TextureMap, (
      (uv_13 * 0.3)
     - 
      (swim * 0.04)
    ), 5.0).y)))
  , 0.0, 1.0)) * 0.6));
  for (float i_10 = 0.0; i_10 < 50.0; i_10 += 1.0) {
    vec2 pos_96;
    float tmpvar_97;
    tmpvar_97 = (time + 1.27);
    float tmpvar_98;
    tmpvar_98 = floor(((tmpvar_97 + 2.0) / 4.0));
    vec2 tmpvar_99;
    tmpvar_99.x = 0.0;
    tmpvar_99.y = (float(mod (((tmpvar_97 + 
      (i_10 / 50.0)
    ) + (
      fract((sin((i_10 + tmpvar_98)) * 43758.55))
     * 0.7)), 4.0)));
    vec2 tmpvar_100;
    tmpvar_100 = (vec2(0.4, -0.9) + tmpvar_99);
    pos_96.y = tmpvar_100.y;
    pos_96.x = (tmpvar_100.x + ((
      fract((sin(i_10) * 43758.55))
     * 0.7) * (uv_13.y + 0.6)));
    pos_96 = (pos_96 + (texture2D (u_TextureMap, (
      ((uv_13 * 0.3) - (time * 0.1))
     + 
      (i_10 / 80.0)
    ), 4.0).z * 0.05));
    float size_101;
    size_101 = ((0.002 * fract(
      (sin((i_10 - tmpvar_98)) * 43758.55)
    )) + 0.00015);
    float tmpvar_102;
    float d_103;
    vec2 v2_104;
    vec2 tmpvar_105;
    tmpvar_105 = (pos_96 - uv_13);
    v2_104 = tmpvar_105;
    float tmpvar_106;
    tmpvar_106 = (dot (tmpvar_105, tmpvar_105) / size_101);
    d_103 = tmpvar_106;
    if ((tmpvar_106 > 1.0)) {
      tmpvar_102 = (pow (max (0.0, 
        (1.5 - tmpvar_106)
      ), 3.0) * 5.0);
    } else {
      d_103 = (pow (tmpvar_106, 6.0) * 0.85);
      vec2 tmpvar_107;
      tmpvar_107.x = (-(size_101) * 7.0);
      tmpvar_107.y = (size_101 * 7.0);
      v2_104 = ((pos_96 - uv_13) + tmpvar_107);
      d_103 = (d_103 + (0.8 / max (
        sqrt(((dot (v2_104, v2_104) / size_101) * 8.0))
      , 0.3)));
      vec2 tmpvar_108;
      tmpvar_108.x = (size_101 * 7.0);
      tmpvar_108.y = (-(size_101) * 7.0);
      v2_104 = ((pos_96 - uv_13) + tmpvar_108);
      d_103 = (d_103 + (0.2 / max (
        ((dot (v2_104, v2_104) / size_101) * 4.0)
      , 0.3)));
      tmpvar_102 = d_103;
    };
    vec3 tmpvar_109;
    tmpvar_109.yz = vec2(1.0, 1.0);
    tmpvar_109.x = (0.6 + (fract(
      (sin(((tmpvar_98 * 323.1) + i_10)) * 43758.55)
    ) * 0.4));
    col_14 = mix (col_14, tmpvar_109, (tmpvar_102 * (
      (fract((sin(
        ((i_10 + tmpvar_98) + 399.0)
      ) * 43758.55)) * 0.3)
     + 0.08)));
  };
  uv_13 = ((final_3 * 2.0) - 1.0);
  vec3 tmpvar_110;
  tmpvar_110 = (mix (vec3(0.5, 0.5, 0.5), mix (vec3(
    dot (vec3(0.2125, 0.7154, 0.0721), (col_14 * 1.22))
  ), 
    (col_14 * 1.22)
  , 1.05), 1.1) * (1.0 - (
    abs(uv_13.x)
   * 
    abs(uv_13.y)
  )));
  col_14 = tmpvar_110;
  vec4 tmpvar_111;
  tmpvar_111.xyz = (((3.984375 * 
    clamp ((tmpvar_110 - 0.3764706), 0.0, 1.0)
  ) + screen_2.xyz) / 3.0);
  tmpvar_111.w = screen_2.w;
  gl_FragColor = tmpvar_111;
}


// stats: 479 alu 6 tex 17 flow
// inputs: 3
//  #0: total_time (high float) 1x1 [-1]
//  #1: vTexCoord0 (high float) 2x1 [-1]
//  #2: resolution (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

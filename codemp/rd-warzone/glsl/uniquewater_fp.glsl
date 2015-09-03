precision lowp float;
uniform sampler2D u_DiffuseMap;
varying vec2 var_Dimensions;
varying float var_Time;
uniform vec4 u_NormalScale;
varying vec4 var_TexCoords;
varying vec3 var_ViewDir;
varying vec3 var_Normal;
out vec4 out_Glow;
out vec4 out_Normal;
out vec4 out_DetailedNormal;
float SEA_TIME;
void main ()
{
  SEA_TIME = (1.04 * var_Time);
  vec4 specular_1;
  float scaleWater_2;
  vec3 dir_3;
  vec2 uv_4;
  vec4 diffuse_5;
  vec3 N_6;
  vec3 p_7;
  p_7 = -(var_ViewDir);
  vec3 tmpvar_8;
  tmpvar_8 = dFdx(p_7);
  vec3 tmpvar_9;
  tmpvar_9 = dFdy(p_7);
  vec2 tmpvar_10;
  tmpvar_10 = dFdx(var_TexCoords.xy);
  vec2 tmpvar_11;
  tmpvar_11 = dFdy(var_TexCoords.xy);
  vec3 tmpvar_12;
  tmpvar_12 = ((tmpvar_9.yzx * var_Normal.zxy) - (tmpvar_9.zxy * var_Normal.yzx));
  vec3 tmpvar_13;
  tmpvar_13 = ((var_Normal.yzx * tmpvar_8.zxy) - (var_Normal.zxy * tmpvar_8.yzx));
  vec3 tmpvar_14;
  tmpvar_14 = ((tmpvar_12 * tmpvar_10.x) + (tmpvar_13 * tmpvar_11.x));
  vec3 tmpvar_15;
  tmpvar_15 = ((tmpvar_12 * tmpvar_10.y) + (tmpvar_13 * tmpvar_11.y));
  float tmpvar_16;
  tmpvar_16 = inversesqrt(max (dot (tmpvar_14, tmpvar_14), dot (tmpvar_15, tmpvar_15)));
  mat3 tmpvar_17;
  tmpvar_17[0] = (tmpvar_14 * tmpvar_16);
  tmpvar_17[1] = (tmpvar_15 * tmpvar_16);
  tmpvar_17[2] = var_Normal;
  vec3 color_18;
  vec4 tmpvar_19;
  tmpvar_19 = texture2D (u_DiffuseMap, var_TexCoords.xy);
  color_18 = ((tmpvar_19.xyz - 0.4901961) * 2.217391);
  vec3 tmpvar_20;
  tmpvar_20 = clamp (((color_18 * color_18) * (color_18 * 5.0)), 0.0, 1.0);
  color_18 = tmpvar_20;
  vec3 tmpvar_21;
  tmpvar_21 = clamp (clamp ((
    (tmpvar_20 + ((tmpvar_19.xyz - 0.0627451) * 1.164384))
   * 2.5), 0.0, 1.0), 0.0, 1.0);
  float tmpvar_22;
  tmpvar_22 = clamp ((1.0 - (
    ((tmpvar_21.x + tmpvar_21.y) + tmpvar_21.z)
   / 4.0)), 0.0, 1.0);
  vec2 tmpvar_23;
  tmpvar_23.x = (tmpvar_22 - 0.5);
  tmpvar_23.y = (0.5 - tmpvar_22);
  N_6.xy = tmpvar_23;
  N_6 = ((N_6 * 0.5) + 0.5);
  N_6.xy = (N_6.xy * u_NormalScale.xy);
  N_6.z = sqrt(clamp ((
    (0.25 - (N_6.x * N_6.x))
   - 
    (N_6.y * N_6.y)
  ), 0.0, 1.0));
  N_6 = (tmpvar_17 * N_6);
  N_6 = normalize(N_6);
  diffuse_5 = tmpvar_19;
  uv_4 = ((var_TexCoords.xy * 2.0) - 1.0);
  uv_4.x = (uv_4.x * (var_Dimensions.x / var_Dimensions.y));
  vec3 tmpvar_24;
  tmpvar_24.z = -3.1415;
  tmpvar_24.xy = uv_4;
  vec3 tmpvar_25;
  tmpvar_25 = normalize(tmpvar_24);
  dir_3.xy = tmpvar_25.xy;
  dir_3.z = (tmpvar_25.z + (sqrt(
    dot (uv_4, uv_4)
  ) * 0.15));
  mat3 m_26;
  vec3 tmpvar_27;
  tmpvar_27.x = 1.0;
  tmpvar_27.y = 0.0;
  tmpvar_27.z = -0.0;
  m_26[0] = tmpvar_27;
  vec3 tmpvar_28;
  tmpvar_28.x = -0.0;
  tmpvar_28.y = 0.0707372;
  tmpvar_28.z = 0.997495;
  m_26[1] = tmpvar_28;
  vec3 tmpvar_29;
  tmpvar_29.x = 0.0;
  tmpvar_29.y = -0.997495;
  tmpvar_29.z = 0.0707372;
  m_26[2] = tmpvar_29;
  dir_3 = (normalize(dir_3) * m_26);
  vec3 dir_30;
  dir_30 = dir_3;
  vec3 p_31;
  float hm_33;
  float hx_34;
  float tx_35;
  float tm_36;
  tm_36 = 0.0;
  tx_35 = 1000.0;
  float tmpvar_37;
  vec3 p_38;
  p_38 = (vec3(0.0, 10.5, 0.0) + (dir_3 * 1000.0));
  float h_40;
  vec2 uv_41;
  float choppy_42;
  float amp_43;
  float freq_44;
  freq_44 = 0.36;
  amp_43 = 0.6;
  choppy_42 = 4.0;
  uv_41.y = p_38.z;
  uv_41.x = (p_38.x * 0.75);
  h_40 = 0.0;
  for (int i_39 = 0; i_39 < 3; i_39++) {
    vec2 uv_45;
    uv_45 = ((uv_41 + SEA_TIME) * freq_44);
    vec2 tmpvar_46;
    tmpvar_46 = floor(uv_45);
    vec2 tmpvar_47;
    tmpvar_47 = fract(uv_45);
    vec2 tmpvar_48;
    tmpvar_48 = ((tmpvar_47 * tmpvar_47) * (3.0 - (2.0 * tmpvar_47)));
    uv_45 = (uv_45 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_46, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_46 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_48.x), mix (fract((
        sin(dot ((tmpvar_46 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_46 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_48.x), tmpvar_48.y)
    )));
    vec2 tmpvar_49;
    tmpvar_49 = (1.0 - abs(sin(uv_45)));
    vec2 tmpvar_50;
    tmpvar_50 = mix (tmpvar_49, abs(cos(uv_45)), tmpvar_49);
    vec2 uv_51;
    uv_51 = ((uv_41 - SEA_TIME) * freq_44);
    vec2 tmpvar_52;
    tmpvar_52 = floor(uv_51);
    vec2 tmpvar_53;
    tmpvar_53 = fract(uv_51);
    vec2 tmpvar_54;
    tmpvar_54 = ((tmpvar_53 * tmpvar_53) * (3.0 - (2.0 * tmpvar_53)));
    uv_51 = (uv_51 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_52, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_52 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_54.x), mix (fract((
        sin(dot ((tmpvar_52 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_52 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_54.x), tmpvar_54.y)
    )));
    vec2 tmpvar_55;
    tmpvar_55 = (1.0 - abs(sin(uv_51)));
    vec2 tmpvar_56;
    tmpvar_56 = mix (tmpvar_55, abs(cos(uv_51)), tmpvar_55);
    h_40 = (h_40 + ((
      (pow ((1.0 - pow (
        (tmpvar_50.x * tmpvar_50.y)
      , 0.65)), choppy_42) * 0.3)
     + 
      (pow ((1.0 - pow (
        (tmpvar_56.x * tmpvar_56.y)
      , 0.65)), choppy_42) * 0.3)
    ) * amp_43));
    uv_41 = (uv_41 * mat2(1.6, 1.2, -1.2, 1.6));
    freq_44 = (freq_44 * 1.9);
    amp_43 = (amp_43 * 0.22);
    choppy_42 = mix (choppy_42, 1.0, 0.2);
  };
  tmpvar_37 = (p_38.y - h_40);
  hx_34 = tmpvar_37;
  if ((tmpvar_37 <= 0.0)) {
    float h_58;
    vec2 uv_59;
    float choppy_60;
    float amp_61;
    float freq_62;
    freq_62 = 0.36;
    amp_61 = 0.6;
    choppy_60 = 4.0;
    uv_59.y = 0.0;
    uv_59.x = 0.0;
    h_58 = 0.0;
    for (int i_57 = 0; i_57 < 3; i_57++) {
      vec2 uv_63;
      uv_63 = ((uv_59 + SEA_TIME) * freq_62);
      vec2 tmpvar_64;
      tmpvar_64 = floor(uv_63);
      vec2 tmpvar_65;
      tmpvar_65 = fract(uv_63);
      vec2 tmpvar_66;
      tmpvar_66 = ((tmpvar_65 * tmpvar_65) * (3.0 - (2.0 * tmpvar_65)));
      uv_63 = (uv_63 + (-1.0 + (2.0 * 
        mix (mix (fract((
          sin(dot (tmpvar_64, vec2(127.1, 311.7)))
         * 43758.55)), fract((
          sin(dot ((tmpvar_64 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
         * 43758.55)), tmpvar_66.x), mix (fract((
          sin(dot ((tmpvar_64 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
         * 43758.55)), fract((
          sin(dot ((tmpvar_64 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
         * 43758.55)), tmpvar_66.x), tmpvar_66.y)
      )));
      vec2 tmpvar_67;
      tmpvar_67 = (1.0 - abs(sin(uv_63)));
      vec2 tmpvar_68;
      tmpvar_68 = mix (tmpvar_67, abs(cos(uv_63)), tmpvar_67);
      vec2 uv_69;
      uv_69 = ((uv_59 - SEA_TIME) * freq_62);
      vec2 tmpvar_70;
      tmpvar_70 = floor(uv_69);
      vec2 tmpvar_71;
      tmpvar_71 = fract(uv_69);
      vec2 tmpvar_72;
      tmpvar_72 = ((tmpvar_71 * tmpvar_71) * (3.0 - (2.0 * tmpvar_71)));
      uv_69 = (uv_69 + (-1.0 + (2.0 * 
        mix (mix (fract((
          sin(dot (tmpvar_70, vec2(127.1, 311.7)))
         * 43758.55)), fract((
          sin(dot ((tmpvar_70 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
         * 43758.55)), tmpvar_72.x), mix (fract((
          sin(dot ((tmpvar_70 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
         * 43758.55)), fract((
          sin(dot ((tmpvar_70 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
         * 43758.55)), tmpvar_72.x), tmpvar_72.y)
      )));
      vec2 tmpvar_73;
      tmpvar_73 = (1.0 - abs(sin(uv_69)));
      vec2 tmpvar_74;
      tmpvar_74 = mix (tmpvar_73, abs(cos(uv_69)), tmpvar_73);
      h_58 = (h_58 + ((
        (pow ((1.0 - pow (
          (tmpvar_68.x * tmpvar_68.y)
        , 0.65)), choppy_60) * 0.3)
       + 
        (pow ((1.0 - pow (
          (tmpvar_74.x * tmpvar_74.y)
        , 0.65)), choppy_60) * 0.3)
      ) * amp_61));
      uv_59 = (uv_59 * mat2(1.6, 1.2, -1.2, 1.6));
      freq_62 = (freq_62 * 1.9);
      amp_61 = (amp_61 * 0.22);
      choppy_60 = mix (choppy_60, 1.0, 0.2);
    };
    hm_33 = (10.5 - h_58);
    for (int i_32 = 0; i_32 < 8; i_32++) {
      float tmpvar_75;
      tmpvar_75 = mix (tm_36, tx_35, (hm_33 / (hm_33 - hx_34)));
      p_31 = (vec3(0.0, 10.5, 0.0) + (dir_30 * tmpvar_75));
      float tmpvar_76;
      float h_78;
      vec2 uv_79;
      float choppy_80;
      float amp_81;
      float freq_82;
      freq_82 = 0.36;
      amp_81 = 0.6;
      choppy_80 = 4.0;
      uv_79.y = p_31.z;
      uv_79.x = (p_31.x * 0.75);
      h_78 = 0.0;
      for (int i_77 = 0; i_77 < 3; i_77++) {
        vec2 uv_83;
        uv_83 = ((uv_79 + SEA_TIME) * freq_82);
        vec2 tmpvar_84;
        tmpvar_84 = floor(uv_83);
        vec2 tmpvar_85;
        tmpvar_85 = fract(uv_83);
        vec2 tmpvar_86;
        tmpvar_86 = ((tmpvar_85 * tmpvar_85) * (3.0 - (2.0 * tmpvar_85)));
        uv_83 = (uv_83 + (-1.0 + (2.0 * 
          mix (mix (fract((
            sin(dot (tmpvar_84, vec2(127.1, 311.7)))
           * 43758.55)), fract((
            sin(dot ((tmpvar_84 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
           * 43758.55)), tmpvar_86.x), mix (fract((
            sin(dot ((tmpvar_84 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
           * 43758.55)), fract((
            sin(dot ((tmpvar_84 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
           * 43758.55)), tmpvar_86.x), tmpvar_86.y)
        )));
        vec2 tmpvar_87;
        tmpvar_87 = (1.0 - abs(sin(uv_83)));
        vec2 tmpvar_88;
        tmpvar_88 = mix (tmpvar_87, abs(cos(uv_83)), tmpvar_87);
        vec2 uv_89;
        uv_89 = ((uv_79 - SEA_TIME) * freq_82);
        vec2 tmpvar_90;
        tmpvar_90 = floor(uv_89);
        vec2 tmpvar_91;
        tmpvar_91 = fract(uv_89);
        vec2 tmpvar_92;
        tmpvar_92 = ((tmpvar_91 * tmpvar_91) * (3.0 - (2.0 * tmpvar_91)));
        uv_89 = (uv_89 + (-1.0 + (2.0 * 
          mix (mix (fract((
            sin(dot (tmpvar_90, vec2(127.1, 311.7)))
           * 43758.55)), fract((
            sin(dot ((tmpvar_90 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
           * 43758.55)), tmpvar_92.x), mix (fract((
            sin(dot ((tmpvar_90 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
           * 43758.55)), fract((
            sin(dot ((tmpvar_90 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
           * 43758.55)), tmpvar_92.x), tmpvar_92.y)
        )));
        vec2 tmpvar_93;
        tmpvar_93 = (1.0 - abs(sin(uv_89)));
        vec2 tmpvar_94;
        tmpvar_94 = mix (tmpvar_93, abs(cos(uv_89)), tmpvar_93);
        h_78 = (h_78 + ((
          (pow ((1.0 - pow (
            (tmpvar_88.x * tmpvar_88.y)
          , 0.65)), choppy_80) * 0.3)
         + 
          (pow ((1.0 - pow (
            (tmpvar_94.x * tmpvar_94.y)
          , 0.65)), choppy_80) * 0.3)
        ) * amp_81));
        uv_79 = (uv_79 * mat2(1.6, 1.2, -1.2, 1.6));
        freq_82 = (freq_82 * 1.9);
        amp_81 = (amp_81 * 0.22);
        choppy_80 = mix (choppy_80, 1.0, 0.2);
      };
      tmpvar_76 = (p_31.y - h_78);
      if ((tmpvar_76 < 0.0)) {
        tx_35 = tmpvar_75;
        hx_34 = tmpvar_76;
      } else {
        tm_36 = tmpvar_75;
        hm_33 = tmpvar_76;
      };
    };
  };
  vec3 tmpvar_95;
  tmpvar_95 = (p_31 - vec3(0.0, 10.5, 0.0));
  scaleWater_2 = 1.0;
  if (((N_6.z < N_6.x) && (N_6.z < N_6.y))) {
    scaleWater_2 = 10.0;
  };
  float tmpvar_96;
  tmpvar_96 = sqrt(dot (tmpvar_95, tmpvar_95));
  float eps_97;
  eps_97 = ((tmpvar_96 * tmpvar_96) * 0.0003);
  vec3 n_98;
  float h_100;
  vec2 uv_101;
  float choppy_102;
  float amp_103;
  float freq_104;
  freq_104 = (0.36 * scaleWater_2);
  amp_103 = (0.6 * scaleWater_2);
  choppy_102 = (4.0 * scaleWater_2);
  uv_101.y = p_31.z;
  uv_101.x = (p_31.x * 0.75);
  h_100 = 0.0;
  for (int i_99 = 0; i_99 < 5; i_99++) {
    vec2 uv_105;
    uv_105 = ((uv_101 + SEA_TIME) * freq_104);
    vec2 tmpvar_106;
    tmpvar_106 = floor(uv_105);
    vec2 tmpvar_107;
    tmpvar_107 = fract(uv_105);
    vec2 tmpvar_108;
    tmpvar_108 = ((tmpvar_107 * tmpvar_107) * (3.0 - (2.0 * tmpvar_107)));
    uv_105 = (uv_105 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_106, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_106 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_108.x), mix (fract((
        sin(dot ((tmpvar_106 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_106 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_108.x), tmpvar_108.y)
    )));
    vec2 tmpvar_109;
    tmpvar_109 = (1.0 - abs(sin(uv_105)));
    vec2 tmpvar_110;
    tmpvar_110 = mix (tmpvar_109, abs(cos(uv_105)), tmpvar_109);
    vec2 uv_111;
    uv_111 = ((uv_101 - SEA_TIME) * freq_104);
    vec2 tmpvar_112;
    tmpvar_112 = floor(uv_111);
    vec2 tmpvar_113;
    tmpvar_113 = fract(uv_111);
    vec2 tmpvar_114;
    tmpvar_114 = ((tmpvar_113 * tmpvar_113) * (3.0 - (2.0 * tmpvar_113)));
    uv_111 = (uv_111 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_112, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_112 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_114.x), mix (fract((
        sin(dot ((tmpvar_112 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_112 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_114.x), tmpvar_114.y)
    )));
    vec2 tmpvar_115;
    tmpvar_115 = (1.0 - abs(sin(uv_111)));
    vec2 tmpvar_116;
    tmpvar_116 = mix (tmpvar_115, abs(cos(uv_111)), tmpvar_115);
    h_100 = (h_100 + ((
      (pow ((1.0 - pow (
        (tmpvar_110.x * tmpvar_110.y)
      , 0.65)), choppy_102) * 0.3)
     + 
      (pow ((1.0 - pow (
        (tmpvar_116.x * tmpvar_116.y)
      , 0.65)), choppy_102) * 0.3)
    ) * amp_103));
    uv_101 = (uv_101 * mat2(1.6, 1.2, -1.2, 1.6));
    freq_104 = (freq_104 * 1.9);
    amp_103 = (amp_103 * 0.22);
    choppy_102 = mix (choppy_102, 1.0, 0.2);
  };
  n_98.y = (p_31.y - h_100);
  vec3 tmpvar_117;
  tmpvar_117.x = (p_31.x + eps_97);
  tmpvar_117.yz = p_31.yz;
  float h_119;
  vec2 uv_120;
  float choppy_121;
  float amp_122;
  float freq_123;
  freq_123 = (0.36 * scaleWater_2);
  amp_122 = (0.6 * scaleWater_2);
  choppy_121 = (4.0 * scaleWater_2);
  uv_120.y = tmpvar_117.z;
  uv_120.x = (tmpvar_117.x * 0.75);
  h_119 = 0.0;
  for (int i_118 = 0; i_118 < 5; i_118++) {
    vec2 uv_124;
    uv_124 = ((uv_120 + SEA_TIME) * freq_123);
    vec2 tmpvar_125;
    tmpvar_125 = floor(uv_124);
    vec2 tmpvar_126;
    tmpvar_126 = fract(uv_124);
    vec2 tmpvar_127;
    tmpvar_127 = ((tmpvar_126 * tmpvar_126) * (3.0 - (2.0 * tmpvar_126)));
    uv_124 = (uv_124 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_125, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_125 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_127.x), mix (fract((
        sin(dot ((tmpvar_125 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_125 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_127.x), tmpvar_127.y)
    )));
    vec2 tmpvar_128;
    tmpvar_128 = (1.0 - abs(sin(uv_124)));
    vec2 tmpvar_129;
    tmpvar_129 = mix (tmpvar_128, abs(cos(uv_124)), tmpvar_128);
    vec2 uv_130;
    uv_130 = ((uv_120 - SEA_TIME) * freq_123);
    vec2 tmpvar_131;
    tmpvar_131 = floor(uv_130);
    vec2 tmpvar_132;
    tmpvar_132 = fract(uv_130);
    vec2 tmpvar_133;
    tmpvar_133 = ((tmpvar_132 * tmpvar_132) * (3.0 - (2.0 * tmpvar_132)));
    uv_130 = (uv_130 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_131, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_131 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_133.x), mix (fract((
        sin(dot ((tmpvar_131 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_131 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_133.x), tmpvar_133.y)
    )));
    vec2 tmpvar_134;
    tmpvar_134 = (1.0 - abs(sin(uv_130)));
    vec2 tmpvar_135;
    tmpvar_135 = mix (tmpvar_134, abs(cos(uv_130)), tmpvar_134);
    h_119 = (h_119 + ((
      (pow ((1.0 - pow (
        (tmpvar_129.x * tmpvar_129.y)
      , 0.65)), choppy_121) * 0.3)
     + 
      (pow ((1.0 - pow (
        (tmpvar_135.x * tmpvar_135.y)
      , 0.65)), choppy_121) * 0.3)
    ) * amp_122));
    uv_120 = (uv_120 * mat2(1.6, 1.2, -1.2, 1.6));
    freq_123 = (freq_123 * 1.9);
    amp_122 = (amp_122 * 0.22);
    choppy_121 = mix (choppy_121, 1.0, 0.2);
  };
  n_98.x = ((p_31.y - h_119) - n_98.y);
  vec3 tmpvar_136;
  tmpvar_136.xy = p_31.xy;
  tmpvar_136.z = (p_31.z + eps_97);
  float h_138;
  vec2 uv_139;
  float choppy_140;
  float amp_141;
  float freq_142;
  freq_142 = (0.36 * scaleWater_2);
  amp_141 = (0.6 * scaleWater_2);
  choppy_140 = (4.0 * scaleWater_2);
  uv_139.y = tmpvar_136.z;
  uv_139.x = (p_31.x * 0.75);
  h_138 = 0.0;
  for (int i_137 = 0; i_137 < 5; i_137++) {
    vec2 uv_143;
    uv_143 = ((uv_139 + SEA_TIME) * freq_142);
    vec2 tmpvar_144;
    tmpvar_144 = floor(uv_143);
    vec2 tmpvar_145;
    tmpvar_145 = fract(uv_143);
    vec2 tmpvar_146;
    tmpvar_146 = ((tmpvar_145 * tmpvar_145) * (3.0 - (2.0 * tmpvar_145)));
    uv_143 = (uv_143 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_144, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_144 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_146.x), mix (fract((
        sin(dot ((tmpvar_144 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_144 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_146.x), tmpvar_146.y)
    )));
    vec2 tmpvar_147;
    tmpvar_147 = (1.0 - abs(sin(uv_143)));
    vec2 tmpvar_148;
    tmpvar_148 = mix (tmpvar_147, abs(cos(uv_143)), tmpvar_147);
    vec2 uv_149;
    uv_149 = ((uv_139 - SEA_TIME) * freq_142);
    vec2 tmpvar_150;
    tmpvar_150 = floor(uv_149);
    vec2 tmpvar_151;
    tmpvar_151 = fract(uv_149);
    vec2 tmpvar_152;
    tmpvar_152 = ((tmpvar_151 * tmpvar_151) * (3.0 - (2.0 * tmpvar_151)));
    uv_149 = (uv_149 + (-1.0 + (2.0 * 
      mix (mix (fract((
        sin(dot (tmpvar_150, vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_150 + vec2(1.0, 0.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_152.x), mix (fract((
        sin(dot ((tmpvar_150 + vec2(0.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), fract((
        sin(dot ((tmpvar_150 + vec2(1.0, 1.0)), vec2(127.1, 311.7)))
       * 43758.55)), tmpvar_152.x), tmpvar_152.y)
    )));
    vec2 tmpvar_153;
    tmpvar_153 = (1.0 - abs(sin(uv_149)));
    vec2 tmpvar_154;
    tmpvar_154 = mix (tmpvar_153, abs(cos(uv_149)), tmpvar_153);
    h_138 = (h_138 + ((
      (pow ((1.0 - pow (
        (tmpvar_148.x * tmpvar_148.y)
      , 0.65)), choppy_140) * 0.3)
     + 
      (pow ((1.0 - pow (
        (tmpvar_154.x * tmpvar_154.y)
      , 0.65)), choppy_140) * 0.3)
    ) * amp_141));
    uv_139 = (uv_139 * mat2(1.6, 1.2, -1.2, 1.6));
    freq_142 = (freq_142 * 1.9);
    amp_141 = (amp_141 * 0.22);
    choppy_140 = mix (choppy_140, 1.0, 0.2);
  };
  n_98.z = ((p_31.y - h_138) - n_98.y);
  n_98.y = eps_97;
  vec3 tmpvar_155;
  tmpvar_155 = normalize(n_98);
  vec3 color_156;
  color_156 = (mix ((vec3(0.1, 0.19, 0.22) + 
    (vec3(0.096, 0.108, 0.072) * pow (((
      dot (tmpvar_155, vec3(0.0, 0.7808688, 0.6246951))
     * 0.4) + 0.6), 80.0))
  ), vec3(0.2, 0.3, 0.5), (
    pow ((1.0 - max (dot (tmpvar_155, 
      -(dir_3)
    ), 0.0)), 3.0)
   * 0.65)) + ((vec3(0.144, 0.162, 0.108) * 
    (p_31.y - 0.6)
  ) * max (
    (1.0 - (dot (tmpvar_95, tmpvar_95) * 0.001))
  , 0.0)));
  color_156 = (color_156 + vec3((pow (
    max (dot ((dir_3 - (2.0 * 
      (dot (tmpvar_155, dir_3) * tmpvar_155)
    )), vec3(0.0, 0.7808688, 0.6246951)), 0.0)
  , 60.0) * 2.705714)));
  diffuse_5.xyz = pow (color_156, vec3(0.75, 0.75, 0.75));
  float tmpvar_157;
  tmpvar_157 = (((diffuse_5.x + diffuse_5.y) + diffuse_5.z) / 3.0);
  vec4 tmpvar_158;
  tmpvar_158.xyz = diffuse_5.xyz;
  tmpvar_158.w = diffuse_5.w;
  gl_FragColor = tmpvar_158;
  specular_1.xyz = (vec4((1.0 - tmpvar_22)) * diffuse_5).xyz;
  specular_1.w = ((clamp (
    (1.0 - tmpvar_22)
  , 0.0, 1.0) * 0.5) + 0.5);
  specular_1.w = clamp (((specular_1.w * 2.0) * specular_1.w), 0.2, 0.9);
  specular_1 = (specular_1 * 1.5);
  diffuse_5.xyz = (diffuse_5.xyz * (vec3(1.0, 1.0, 1.0) - specular_1.xyz));
  if ((scaleWater_2 >= 10.0)) {
    gl_FragColor.xyz = clamp (((tmpvar_158.xyz + tmpvar_19.xyz) / 2.0), 0.0, 1.0);
    gl_FragColor.w = clamp (tmpvar_157, 0.1, 1.0);
#if defined(USE_GLOW_BUFFER)
		//out_Glow = gl_FragColor;
		out_Glow = vec4(0.0);
#else
		out_Glow = vec4(0.0);
#endif
  } else {
    gl_FragColor.xyz = clamp (((
      (gl_FragColor.xyz + gl_FragColor.xyz)
     + tmpvar_19.xyz) / 3.0), 0.0, 1.0);
    gl_FragColor.w = clamp (tmpvar_157, 0.5, 1.0);
		out_Glow = vec4(0.0);
  };
}


// stats: 785 alu 1 tex 18 flow
// inputs: 5
//  #0: var_Dimensions (high float) 2x1 [-1]
//  #1: var_Time (high float) 1x1 [-1]
//  #2: var_TexCoords (high float) 4x1 [-1]
//  #3: var_ViewDir (high float) 3x1 [-1]
//  #4: var_Normal (high float) 3x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_NormalScale (high float) 4x1 [-1]
// textures: 1
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]

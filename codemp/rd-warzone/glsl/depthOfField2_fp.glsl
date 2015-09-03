precision mediump float;
precision lowp sampler2D;
uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
varying vec4 var_Local0;
vec2 sampleOffset;
void main ()
{
  sampleOffset = (1.0/(var_Dimensions));
  int axis_1;
  axis_1 = int(var_Local0.w);
  vec2 coord_2;
  coord_2 = var_TexCoords;
  float wValue_4;
  vec2 discRadius_5;
  float coordDepth_6;
  vec4 tcol_7;
  vec4 res_8;
  tcol_7 = texture2D (u_TextureMap, var_TexCoords);
  coordDepth_6 = (texture2D (u_ScreenDepthMap, var_TexCoords).x * 255.0);
  float tmpvar_9;
  if ((var_Local0.x < 4.0)) {
    tmpvar_9 = 253.0;
  } else {
    tmpvar_9 = (254.745 * texture2D (u_ScreenDepthMap, vec2(0.5, 0.5)).x);
  };
  float tmpvar_10;
  tmpvar_10 = (coordDepth_6 - tmpvar_9);
  vec2 tmpvar_11;
  tmpvar_11 = (((2.5 * sampleOffset) * tmpvar_10) / 5.0);
  discRadius_5 = tmpvar_11;
  if ((tmpvar_10 < 0.0)) {
    discRadius_5 = (tmpvar_11 * 0.06);
  };
  discRadius_5 = (discRadius_5 * 0.5);
  wValue_4 = 0.0;
  for (int i_3 = -5; i_3 < 5; i_3++) {
    vec2 taxis_12;
    vec2 tmpvar_13;
    tmpvar_13 = vec2[4](vec2(-0.306, 0.739), vec2(0.306, 0.739), vec2(-0.739, 0.306), vec2(-0.739, -0.306))[axis_1];
    taxis_12.y = tmpvar_13.y;
    taxis_12.x = ((0.9848077 * tmpvar_13.x) - (0.1736482 * tmpvar_13.y));
    taxis_12.y = ((0.1736482 * taxis_12.x) + (0.9848077 * tmpvar_13.y));
    float tmpvar_14;
    tmpvar_14 = float(i_3);
    vec2 tmpvar_15;
    tmpvar_15 = (coord_2 + ((taxis_12 * tmpvar_14) * discRadius_5));
    vec3 chroma_16;
    chroma_16.x = pow (0.5, (2.4 * discRadius_5.x));
    chroma_16.y = 1.0;
    chroma_16.z = pow (1.5, (2.4 * discRadius_5.x));
    vec3 tmpvar_17;
    tmpvar_17.x = texture2D (u_TextureMap, (((
      ((2.0 * tmpvar_15) - 1.0)
     * chroma_16.x) * 0.5) + 0.5)).x;
    tmpvar_17.y = texture2D (u_TextureMap, (((
      (2.0 * tmpvar_15)
     - 1.0) * 0.5) + 0.5)).y;
    tmpvar_17.z = texture2D (u_TextureMap, (((
      ((2.0 * tmpvar_15) - 1.0)
     * chroma_16.z) * 0.5) + 0.5)).z;
    vec4 tmpvar_18;
    tmpvar_18.w = 1.0;
    tmpvar_18.xyz = (tmpvar_17 * (1.0 - discRadius_5.x));
    float tmpvar_19;
    tmpvar_19 = (pow ((
      ((sqrt(dot (tmpvar_18.xyz, tmpvar_18.xyz)) * 0.333) + sqrt(dot (tmpvar_18.xyz, tmpvar_18.xyz)))
     + 0.1), 6.0) + abs(tmpvar_14));
    tcol_7 = (tcol_7 + (tmpvar_18 * tmpvar_19));
    wValue_4 = (wValue_4 + tmpvar_19);
  };
  tcol_7 = (tcol_7 / wValue_4);
  res_8.xyz = tcol_7.xyz;
  res_8.w = 1.0;
  gl_FragColor = res_8;
}


// stats: 64 alu 6 tex 4 flow
// inputs: 3
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_Local0 (high float) 4x1 [-1]
// textures: 2
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]

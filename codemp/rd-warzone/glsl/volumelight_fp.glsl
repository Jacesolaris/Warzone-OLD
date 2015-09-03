precision lowp float;
precision lowp sampler2D;
uniform sampler2D u_DiffuseMap;
uniform sampler2D u_ScreenDepthMap;
varying vec2 var_LightScreenPos;
varying vec2 var_TexCoords;
varying vec4 var_Local0;
varying vec4 var_Local1;
void main ()
{
  float bt_1;
  float lightOutColor_2;
  float i_3;
  float tmpShadow_4;
  float illuminationDecay_5;
  vec2 deltaTextCoord_6;
  vec2 texCoord_7;
  texCoord_7 = var_TexCoords;
  vec4 tmpvar_8;
  tmpvar_8 = texture2D (u_DiffuseMap, var_TexCoords);
  deltaTextCoord_6 = ((var_TexCoords - var_LightScreenPos) * 0.02);
  illuminationDecay_5 = 1.0;
  tmpShadow_4 = 0.0;
  i_3 = 0.0;
  for (; i_3 < 50.0; i_3 += 1.0, texCoord_7 = (texCoord_7 - deltaTextCoord_6)) {
    float depthSample_9;
    float tmpvar_10;
    tmpvar_10 = pow (texture2D (u_ScreenDepthMap, texCoord_7).x, 256.0);
    depthSample_9 = tmpvar_10;
    if ((tmpvar_10 > 0.98)) {
      depthSample_9 = 0.0;
    };
    depthSample_9 = (depthSample_9 * 256.0);
    float tmpvar_11;
    tmpvar_11 = clamp (depthSample_9, 0.0, 1.0);
    depthSample_9 = tmpvar_11;
    tmpShadow_4 = (tmpShadow_4 + (tmpvar_11 * (illuminationDecay_5 * 0.01)));
    illuminationDecay_5 = (illuminationDecay_5 * 0.995);
  };
  tmpShadow_4 = (tmpShadow_4 * 0.45);
  lightOutColor_2 = tmpShadow_4;
  bt_1 = tmpShadow_4;
  if ((tmpShadow_4 > 0.666)) {
    bt_1 = (tmpShadow_4 * 0.444);
  } else {
    if ((bt_1 < 0.333)) {
      bt_1 = (bt_1 * 2.55);
    } else {
      bt_1 = (bt_1 * 1.35);
    };
  };
  lightOutColor_2 = (clamp ((tmpShadow_4 * bt_1), 0.0, 1.0) * var_Local0.w);
  vec4 tmpvar_12;
  tmpvar_12.w = 1.0;
  tmpvar_12.xyz = (tmpvar_8.xyz + (clamp (
    ((var_Local1.xyz * lightOutColor_2) - 0.04705882)
  , 0.0, 1.0) * 2.142857));
  vec4 tmpvar_13;
  tmpvar_13 = clamp (tmpvar_12, 0.0, 1.0);
  gl_FragColor = tmpvar_13;
  float tmpvar_14;
  tmpvar_14 = (sqrt(dot (tmpvar_8.xyz, tmpvar_8.xyz)) / 3.0);
  float tmpvar_15;
  tmpvar_15 = (sqrt(dot (tmpvar_13.xyz, tmpvar_13.xyz)) / 3.0);
  if ((tmpvar_14 >= (tmpvar_15 * var_Local1.w))) {
    gl_FragColor.xyz = (tmpvar_13.xyz * var_Local1.w);
  };
}


// stats: 42 alu 2 tex 6 flow
// inputs: 4
//  #0: var_LightScreenPos (high float) 2x1 [-1]
//  #1: var_TexCoords (high float) 2x1 [-1]
//  #2: var_Local0 (high float) 4x1 [-1]
//  #3: var_Local1 (high float) 4x1 [-1]
// textures: 2
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]

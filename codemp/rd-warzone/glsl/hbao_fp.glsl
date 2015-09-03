precision mediump float;
precision lowp sampler2D;
uniform sampler2D u_NormalMap;
uniform sampler2D u_ScreenDepthMap;
uniform mat4 u_invEyeProjectionMatrix;
varying vec2 var_ScreenTex;
void main ()
{
  float total_2;
  vec3 viewNorm_3;
  vec3 viewPos_4;
  vec3 tmpvar_5;
  tmpvar_5.xy = var_ScreenTex;
  tmpvar_5.z = texture2D (u_ScreenDepthMap, var_ScreenTex).x;
  vec4 tmpvar_6;
  tmpvar_6.w = 1.0;
  tmpvar_6.xyz = ((2.0 * tmpvar_5) - 1.0);
  vec4 tmpvar_7;
  tmpvar_7 = (u_invEyeProjectionMatrix * tmpvar_6);
  viewPos_4 = (tmpvar_7.xyz / tmpvar_7.w);
  viewNorm_3 = texture2D (u_NormalMap, var_ScreenTex).xyz;
  total_2 = 0.0;
  for (int i_1 = 0; i_1 < 8; i_1++) {
    vec3 lastDiff_9;
    float horizonAngle_10;
    vec2 sampleDir_11;
    float tmpvar_12;
    tmpvar_12 = (float(i_1) * 0.7853982);
    vec2 tmpvar_13;
    tmpvar_13.x = cos(tmpvar_12);
    tmpvar_13.y = sin(tmpvar_12);
    sampleDir_11 = tmpvar_13;
    vec3 tmpvar_14;
    tmpvar_14.z = 0.0;
    tmpvar_14.xy = tmpvar_13;
    float tmpvar_15;
    tmpvar_15 = dot (tmpvar_14, viewNorm_3);
    float tmpvar_16;
    tmpvar_16 = (((1.570796 - 
      (sign(tmpvar_15) * (1.570796 - (sqrt(
        (1.0 - abs(tmpvar_15))
      ) * (1.570796 + 
        (abs(tmpvar_15) * (-0.2146018 + (abs(tmpvar_15) * (0.08656672 + 
          (abs(tmpvar_15) * -0.03102955)
        ))))
      ))))
    ) - 1.570796) + 0.2);
    horizonAngle_10 = tmpvar_16;
    lastDiff_9 = vec3(0.0, 0.0, 0.0);
    for (int j_8 = 0; j_8 < 4; j_8++) {
      vec2 tmpvar_17;
      tmpvar_17 = (var_ScreenTex + ((
        float((j_8 + 1))
       * 0.004) * sampleDir_11));
      vec3 tmpvar_18;
      tmpvar_18.xy = tmpvar_17;
      tmpvar_18.z = texture2D (u_ScreenDepthMap, tmpvar_17).x;
      vec4 tmpvar_19;
      tmpvar_19.w = 1.0;
      tmpvar_19.xyz = ((2.0 * tmpvar_18) - 1.0);
      vec4 tmpvar_20;
      tmpvar_20 = (u_invEyeProjectionMatrix * tmpvar_19);
      vec3 tmpvar_21;
      tmpvar_21 = ((tmpvar_20.xyz / tmpvar_20.w) - viewPos_4);
      float tmpvar_22;
      tmpvar_22 = sqrt(dot (tmpvar_21, tmpvar_21));
      if ((tmpvar_22 < 0.5)) {
        lastDiff_9 = tmpvar_21;
        float y_over_x_23;
        y_over_x_23 = (tmpvar_21.z / sqrt(dot (tmpvar_21.xy, tmpvar_21.xy)));
        float tmpvar_24;
        tmpvar_24 = (min (abs(y_over_x_23), 1.0) / max (abs(y_over_x_23), 1.0));
        float tmpvar_25;
        tmpvar_25 = (tmpvar_24 * tmpvar_24);
        tmpvar_25 = (((
          ((((
            ((((-0.01213232 * tmpvar_25) + 0.05368138) * tmpvar_25) - 0.1173503)
           * tmpvar_25) + 0.1938925) * tmpvar_25) - 0.3326756)
         * tmpvar_25) + 0.9999793) * tmpvar_24);
        tmpvar_25 = (tmpvar_25 + (float(
          (abs(y_over_x_23) > 1.0)
        ) * (
          (tmpvar_25 * -2.0)
         + 1.570796)));
        horizonAngle_10 = max (horizonAngle_10, (tmpvar_25 * sign(y_over_x_23)));
      };
    };
    total_2 = (total_2 + (1.0 - clamp (
      ((1.0/((1.0 + sqrt(
        dot (lastDiff_9, lastDiff_9)
      )))) * (sin(horizonAngle_10) - sin(tmpvar_16)))
    , 0.0, 1.0)));
  };
  total_2 = (total_2 / 8.0);
  vec4 tmpvar_26;
  tmpvar_26.w = 1.0;
  tmpvar_26.x = total_2;
  tmpvar_26.y = total_2;
  tmpvar_26.z = total_2;
  gl_FragColor = tmpvar_26;
}


// stats: 95 alu 3 tex 5 flow
// inputs: 1
//  #0: var_ScreenTex (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_invEyeProjectionMatrix (high float) 4x4 [-1]
// textures: 2
//  #0: u_NormalMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]

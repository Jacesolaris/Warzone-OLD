uniform sampler2D u_DiffuseMap;
varying vec3 var_Normal;
varying vec3 var_Position;
varying vec2 var_Tex1[2];
varying vec4 var_LightColor[2];
varying vec3 var_LightDir[2];
varying float var_NumLights;
void main ()
{
  vec4 out_color_2;
  vec3 eyeVec_3;
  vec3 normal_4;
  normal_4 = var_Normal;
  eyeVec_3 = var_Position;
  out_color_2 = vec4(0.0, 0.0, 0.0, 0.0);
  for (int light_1 = 0; light_1 < int(var_NumLights); light_1++) {
    vec4 final_color_5;
    vec4 tmpvar_6;
    tmpvar_6 = (texture2D (u_DiffuseMap, var_Tex1[light_1]) * 0.33333);
    final_color_5 = tmpvar_6;
    vec4 tmpvar_7;
    tmpvar_7 = var_LightColor[light_1];
    vec3 tmpvar_8;
    tmpvar_8 = normalize(normal_4);
    vec3 tmpvar_9;
    tmpvar_9 = normalize(var_LightDir[light_1]);
    float tmpvar_10;
    tmpvar_10 = dot (tmpvar_8, tmpvar_9);
    if ((tmpvar_10 > 0.0)) {
      final_color_5 = (tmpvar_6 + ((tmpvar_7 * tmpvar_6) * tmpvar_10));
      vec3 I_11;
      I_11 = -(tmpvar_9);
      float tmpvar_12;
      tmpvar_12 = max (dot ((I_11 - 
        (2.0 * (dot (tmpvar_8, I_11) * tmpvar_8))
      ), normalize(eyeVec_3)), 0.0);
      final_color_5 = (final_color_5 + ((tmpvar_7 * tmpvar_6) * (tmpvar_12 * tmpvar_12)));
    };
    out_color_2 = (out_color_2 + final_color_5);
  };
  gl_FragColor = out_color_2;
}


// stats: 26 alu 1 tex 3 flow
// inputs: 6
//  #0: var_Normal (high float) 3x1 [-1]
//  #1: var_Position (high float) 3x1 [-1]
//  #2: var_Tex1 (high float) 2x1 [2]
//  #3: var_LightColor (high float) 4x1 [2]
//  #4: var_LightDir (high float) 3x1 [2]
//  #5: var_NumLights (high float) 1x1 [-1]
// textures: 1
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]

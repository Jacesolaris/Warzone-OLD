uniform sampler2D u_TextureMap;
uniform sampler2D u_LevelsMap;
uniform vec4 u_Color;
uniform vec2 u_AutoExposureMinMax;
uniform vec3 u_ToneMinAvgMaxLinear;
varying vec2 var_TexCoords;
void main ()
{
  vec4 color_1;
  vec4 tmpvar_2;
  tmpvar_2 = (texture2D (u_TextureMap, var_TexCoords) * u_Color);
  color_1.w = tmpvar_2.w;
  color_1.xyz = (tmpvar_2.xyz * (u_ToneMinAvgMaxLinear.y / exp2(
    clamp (((texture2D (u_LevelsMap, var_TexCoords).xyz * 20.0) - 10.0), -(u_AutoExposureMinMax.y), -(u_AutoExposureMinMax.x))
  .y)));
  color_1.xyz = max (vec3(0.0, 0.0, 0.0), (color_1.xyz - u_ToneMinAvgMaxLinear.xxx));
  vec3 tmpvar_3;
  tmpvar_3 = vec3((u_ToneMinAvgMaxLinear.z - u_ToneMinAvgMaxLinear.x));
  vec3 tmpvar_4;
  tmpvar_4 = ((0.22 * tmpvar_3) * tmpvar_3);
  vec3 tmpvar_5;
  tmpvar_5 = (0.3 * tmpvar_3);
  vec3 tmpvar_6;
  tmpvar_6 = ((0.22 * color_1.xyz) * color_1.xyz);
  vec3 tmpvar_7;
  tmpvar_7 = (0.3 * color_1.xyz);
  color_1.xyz = (((
    ((tmpvar_6 + (tmpvar_7 * 0.1)) + 0.002)
   / 
    ((tmpvar_6 + tmpvar_7) + 0.06)
  ) - 0.03333333) * (1.0/((
    (((tmpvar_4 + (tmpvar_5 * 0.1)) + 0.002) / ((tmpvar_4 + tmpvar_5) + 0.06))
   - 0.03333333))));
  gl_FragColor = clamp (color_1, 0.0, 1.0);
}


// stats: 35 alu 2 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 3 (total size: 0)
//  #0: u_Color (high float) 4x1 [-1]
//  #1: u_AutoExposureMinMax (high float) 2x1 [-1]
//  #2: u_ToneMinAvgMaxLinear (high float) 3x1 [-1]
// textures: 2
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_LevelsMap (high 2d) 0x0 [-1]

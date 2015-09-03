uniform sampler2D u_DiffuseMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
void main ()
{
  vec3 result_1;
  vec2 tmpvar_2;
  tmpvar_2 = (-(var_TexCoords) + vec2(1.0, 1.0));
  vec2 tmpvar_3;
  tmpvar_3 = ((vec2(0.5, 0.5) - tmpvar_2) * 0.7);
  vec2 tmpvar_4;
  tmpvar_4 = (1.0/(var_Dimensions));
  vec3 tmpvar_5;
  tmpvar_5.y = 0.0;
  tmpvar_5.x = (-(tmpvar_4.x) * 4.0);
  tmpvar_5.z = (tmpvar_4.x * 4.0);
  vec2 tmpvar_6;
  tmpvar_6 = normalize(tmpvar_3);
  vec2 tmpvar_7;
  tmpvar_7 = fract(tmpvar_2);
  vec2 x_8;
  x_8 = (vec2(0.5, 0.5) - tmpvar_7);
  vec3 tmpvar_9;
  tmpvar_9.x = texture2D (u_DiffuseMap, (tmpvar_7 + (tmpvar_6 * tmpvar_5.x))).x;
  tmpvar_9.y = texture2D (u_DiffuseMap, tmpvar_7).y;
  tmpvar_9.z = texture2D (u_DiffuseMap, (tmpvar_7 + (tmpvar_6 * tmpvar_5.z))).z;
  result_1 = ((tmpvar_9 * 0.03) * pow ((1.0 - 
    (sqrt(dot (x_8, x_8)) / 0.7071068)
  ), 10.0));
  vec2 tmpvar_10;
  tmpvar_10 = fract((tmpvar_2 + tmpvar_3));
  vec2 x_11;
  x_11 = (vec2(0.5, 0.5) - tmpvar_10);
  vec3 tmpvar_12;
  tmpvar_12.x = texture2D (u_DiffuseMap, (tmpvar_10 + (tmpvar_6 * tmpvar_5.x))).x;
  tmpvar_12.y = texture2D (u_DiffuseMap, tmpvar_10).y;
  tmpvar_12.z = texture2D (u_DiffuseMap, (tmpvar_10 + (tmpvar_6 * tmpvar_5.z))).z;
  result_1 = (result_1 + ((tmpvar_12 * 0.03) * pow (
    (1.0 - (sqrt(dot (x_11, x_11)) / 0.7071068))
  , 10.0)));
  vec2 tmpvar_13;
  tmpvar_13 = fract((tmpvar_2 + (tmpvar_3 * 2.0)));
  vec2 x_14;
  x_14 = (vec2(0.5, 0.5) - tmpvar_13);
  vec3 tmpvar_15;
  tmpvar_15.x = texture2D (u_DiffuseMap, (tmpvar_13 + (tmpvar_6 * tmpvar_5.x))).x;
  tmpvar_15.y = texture2D (u_DiffuseMap, tmpvar_13).y;
  tmpvar_15.z = texture2D (u_DiffuseMap, (tmpvar_13 + (tmpvar_6 * tmpvar_5.z))).z;
  result_1 = (result_1 + ((tmpvar_15 * 0.03) * pow (
    (1.0 - (sqrt(dot (x_14, x_14)) / 0.7071068))
  , 10.0)));
  vec2 tmpvar_16;
  tmpvar_16 = fract((tmpvar_2 + (tmpvar_3 * 3.0)));
  vec2 x_17;
  x_17 = (vec2(0.5, 0.5) - tmpvar_16);
  vec3 tmpvar_18;
  tmpvar_18.x = texture2D (u_DiffuseMap, (tmpvar_16 + (tmpvar_6 * tmpvar_5.x))).x;
  tmpvar_18.y = texture2D (u_DiffuseMap, tmpvar_16).y;
  tmpvar_18.z = texture2D (u_DiffuseMap, (tmpvar_16 + (tmpvar_6 * tmpvar_5.z))).z;
  result_1 = (result_1 + ((tmpvar_18 * 0.03) * pow (
    (1.0 - (sqrt(dot (x_17, x_17)) / 0.7071068))
  , 10.0)));
  vec2 tmpvar_19;
  tmpvar_19 = (normalize(tmpvar_3) * 0.4);
  vec2 x_20;
  x_20 = (vec2(0.5, 0.5) - fract((tmpvar_2 + tmpvar_19)));
  vec2 texcoord_21;
  texcoord_21 = (tmpvar_2 + tmpvar_19);
  vec3 tmpvar_22;
  tmpvar_22.x = texture2D (u_DiffuseMap, (texcoord_21 + (tmpvar_6 * tmpvar_5.x))).x;
  tmpvar_22.y = texture2D (u_DiffuseMap, texcoord_21).y;
  tmpvar_22.z = texture2D (u_DiffuseMap, (texcoord_21 + (tmpvar_6 * tmpvar_5.z))).z;
  result_1 = (result_1 + ((tmpvar_22 * 0.03) * pow (
    (1.0 - (sqrt(dot (x_20, x_20)) / 0.7071068))
  , 20.0)));
  result_1.x = (result_1.x * sin(var_TexCoords.x));
  result_1.y = (result_1.y * var_TexCoords.y);
  result_1.z = (result_1.z * cos(var_TexCoords.x));
  vec4 tmpvar_23;
  tmpvar_23.w = 1.0;
  tmpvar_23.xyz = (texture2D (u_DiffuseMap, var_TexCoords).xyz + result_1);
  gl_FragColor = tmpvar_23;
}


// stats: 95 alu 16 tex 0 flow
// inputs: 2
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
// textures: 1
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]

precision mediump float;
uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
void main ()
{
  vec4 Res_1;
  float tmpvar_2;
  tmpvar_2 = (1.0/(var_Dimensions.x));
  float tmpvar_3;
  tmpvar_3 = (1.0/(var_Dimensions.y));
  Res_1 = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 tmpvar_4;
  tmpvar_4 = texture2D (u_TextureMap, var_TexCoords);
  vec2 tmpvar_5;
  tmpvar_5.x = -(tmpvar_2);
  tmpvar_5.y = -(tmpvar_3);
  vec4 tmpvar_6;
  tmpvar_6 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_5));
  vec2 tmpvar_7;
  tmpvar_7.x = 0.0;
  tmpvar_7.y = -(tmpvar_3);
  vec4 tmpvar_8;
  tmpvar_8 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_7));
  vec2 tmpvar_9;
  tmpvar_9.x = tmpvar_2;
  tmpvar_9.y = -(tmpvar_3);
  vec4 tmpvar_10;
  tmpvar_10 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_9));
  vec2 tmpvar_11;
  tmpvar_11.y = 0.0;
  tmpvar_11.x = -(tmpvar_2);
  vec4 tmpvar_12;
  tmpvar_12 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_11));
  vec2 tmpvar_13;
  tmpvar_13.y = 0.0;
  tmpvar_13.x = tmpvar_2;
  vec4 tmpvar_14;
  tmpvar_14 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_13));
  vec2 tmpvar_15;
  tmpvar_15.x = -(tmpvar_2);
  tmpvar_15.y = tmpvar_3;
  vec4 tmpvar_16;
  tmpvar_16 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_15));
  vec2 tmpvar_17;
  tmpvar_17.x = 0.0;
  tmpvar_17.y = tmpvar_3;
  vec4 tmpvar_18;
  tmpvar_18 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_17));
  vec2 tmpvar_19;
  tmpvar_19.x = tmpvar_2;
  tmpvar_19.y = tmpvar_3;
  vec4 tmpvar_20;
  tmpvar_20 = texture2D (u_TextureMap, (var_TexCoords + tmpvar_19));
  vec4 x_21;
  x_21 = (((
    abs((((
      ((tmpvar_16 + tmpvar_12) + tmpvar_6)
     - tmpvar_10) - tmpvar_14) - tmpvar_20))
   + 
    abs((((
      ((tmpvar_12 + tmpvar_6) + tmpvar_8)
     - tmpvar_14) - tmpvar_20) - tmpvar_18))
  ) + abs(
    ((((
      (tmpvar_6 + tmpvar_8)
     + tmpvar_10) - tmpvar_20) - tmpvar_18) - tmpvar_16)
  )) + abs((
    ((((tmpvar_8 + tmpvar_10) + tmpvar_14) - tmpvar_18) - tmpvar_16)
   - tmpvar_12)));
  float tmpvar_22;
  tmpvar_22 = (sqrt(dot (x_21, x_21)) / 6.0);
  if ((tmpvar_22 > 0.2)) {
    Res_1 = ((tmpvar_4 * 2.0) - ((
      ((tmpvar_6 + tmpvar_8) + (tmpvar_10 + tmpvar_12))
     + 
      ((tmpvar_14 + tmpvar_16) + (tmpvar_18 + tmpvar_20))
    ) * 0.125));
    gl_FragColor = Res_1;
  } else {
    gl_FragColor = tmpvar_4;
  };
}


// stats: 62 alu 9 tex 1 flow
// inputs: 2
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

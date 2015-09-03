uniform sampler2D u_TextureMap;
uniform sampler2D u_ScreenDepthMap;
in vec2 var_TexCoords;
in vec2 var_Dimensions;
in vec4 var_Local1;
void main ()
{
  float tmpvar_1;
  tmpvar_1 = var_Local1.w;
  if ((var_Local1.x <= 1.0)) {
    vec2 tmpvar_2;
    tmpvar_2 = (1.0/(var_Dimensions));
    vec2 tmpvar_3;
    tmpvar_3.y = 0.0;
    tmpvar_3.x = -(var_Local1.w);
    vec2 tmpvar_4;
    tmpvar_4.y = 0.0;
    tmpvar_4.x = tmpvar_1;
    vec4 tmpvar_5;
    tmpvar_5.w = 1.0;
    tmpvar_5.xyz = ((mat3(0.4155, -0.0458, -0.0545, 0.471, -0.0484, -0.0614, 0.167, -0.0258, 0.0128) * texture (u_TextureMap, (var_TexCoords + 
      (tmpvar_4 * tmpvar_2)
    )).xyz) + (mat3(-0.0109, 0.3756, -0.0651, -0.0365, 0.7333, -0.1286, -0.006, 0.0111, 1.2968) * texture (u_TextureMap, (var_TexCoords + 
      (tmpvar_3 * tmpvar_2)
    )).xyz));
    gl_FragColor = tmpvar_5;
  } else {
    float lineardepth_6;
    vec2 tmpvar_7;
    tmpvar_7 = (1.0/(var_Dimensions));
    lineardepth_6 = (pow (texture (u_ScreenDepthMap, var_TexCoords).x, 255.0) * var_Local1.w);
    vec2 tmpvar_8;
    tmpvar_8.x = (var_TexCoords.x - (tmpvar_7.x * lineardepth_6));
    tmpvar_8.y = var_TexCoords.y;
    vec2 tmpvar_9;
    tmpvar_9.x = (var_TexCoords.x + (tmpvar_7.x * lineardepth_6));
    tmpvar_9.y = var_TexCoords.y;
    vec4 tmpvar_10;
    tmpvar_10.w = 1.0;
    tmpvar_10.xyz = ((mat3(0.4155, -0.0458, -0.0545, 0.471, -0.0484, -0.0614, 0.167, -0.0258, 0.0128) * texture (u_TextureMap, tmpvar_9).xyz) + (mat3(-0.0109, 0.3756, -0.0651, -0.0365, 0.7333, -0.1286, -0.006, 0.0111, 1.2968) * texture (u_TextureMap, tmpvar_8).xyz));
    gl_FragColor = tmpvar_10;
  };
}


// stats: 24 alu 5 tex 1 flow
// inputs: 3
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_Local1 (high float) 4x1 [-1]
// textures: 2
//  #0: u_TextureMap (high 2d) 0x0 [-1]
//  #1: u_ScreenDepthMap (high 2d) 0x0 [-1]

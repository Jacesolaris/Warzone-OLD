uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
varying vec2 var_Dimensions;
varying vec4 var_Local0;
void main ()
{
  vec4 color_1;
  vec4 tmpvar_2;
  tmpvar_2 = texture2D (u_TextureMap, var_TexCoords);
  vec4 out_color_3;
  vec2 tmpvar_4;
  tmpvar_4 = (1.0/(var_Dimensions));
  float tmpvar_5;
  tmpvar_5 = dot (tmpvar_2.xyz, vec3(0.6, 0.2, 0.2));
  out_color_3.xyz = mix ((tmpvar_2.xyz - vec3((
    max (0.0, dot (((
      (texture2D (u_TextureMap, (var_TexCoords - tmpvar_4)).xyz * 2.0)
     - tmpvar_2.xyz) - texture2D (u_TextureMap, (var_TexCoords + tmpvar_4)).xyz), vec3(0.333, 0.333, 0.333)))
   * var_Local0.x))), tmpvar_2.xyz, (tmpvar_5 * tmpvar_5));
  out_color_3.w = 1.0;
  color_1.xyz = (((tmpvar_2.xyz + tmpvar_2.xyz) + out_color_3.xyz) * 0.333);
  color_1.w = 0.05;
  gl_FragColor = color_1;
}


// stats: 18 alu 3 tex 0 flow
// inputs: 3
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Dimensions (high float) 2x1 [-1]
//  #2: var_Local0 (high float) 4x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

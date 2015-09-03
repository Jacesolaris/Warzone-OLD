precision mediump float;
precision lowp sampler2D;
uniform sampler2D u_TextureMap;
uniform vec2 u_Dimensions;
varying vec2 var_TexCoords;
void main ()
{
  vec4 sum_1;
  vec2 tmpvar_2;
  tmpvar_2 = (1.0/(u_Dimensions));
  sum_1 = texture2D (u_TextureMap, (var_TexCoords - tmpvar_2));
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + (vec2(0.0, -1.0) * tmpvar_2))));
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + (vec2(1.0, -1.0) * tmpvar_2))));
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + (vec2(-1.0, 0.0) * tmpvar_2))));
  vec4 tmpvar_3;
  tmpvar_3 = texture2D (u_TextureMap, var_TexCoords);
  sum_1 = (sum_1 + tmpvar_3);
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + (vec2(1.0, 0.0) * tmpvar_2))));
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + (vec2(-1.0, 1.0) * tmpvar_2))));
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + (vec2(0.0, 1.0) * tmpvar_2))));
  sum_1 = (sum_1 + texture2D (u_TextureMap, (var_TexCoords + tmpvar_2)));
  sum_1 = (sum_1 / 9.0);
  gl_FragColor = tmpvar_3;
  if ((((
    (sum_1.x + sum_1.y)
   + sum_1.z) * 0.333) < (0.30969 * (
    (tmpvar_3.x + tmpvar_3.y)
   + tmpvar_3.z)))) {
    gl_FragColor = (tmpvar_3 - vec4(((
      ((tmpvar_3.x + tmpvar_3.y) + tmpvar_3.z)
     - 
      ((sum_1.x + sum_1.y) + sum_1.z)
    ) * 0.33333)));
  };
  gl_FragColor.w = 1.0;
}


// stats: 39 alu 9 tex 1 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_Dimensions (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

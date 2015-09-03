uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
varying vec4 var_Local0;
void main ()
{
  vec4 res_1;
  vec3 tmpvar_2;
  tmpvar_2 = texture2D (u_TextureMap, var_TexCoords).xyz;
  res_1.xyz = mix (vec3(dot (vec3(0.212656, 0.715158, 0.072186), tmpvar_2)), tmpvar_2, (1.0 + (var_Local0.x * 
    (1.0 - (sign(var_Local0.x) * (max (tmpvar_2.x, 
      max (tmpvar_2.y, tmpvar_2.z)
    ) - min (tmpvar_2.x, 
      min (tmpvar_2.y, tmpvar_2.z)
    ))))
  )));
  res_1.w = 1.0;
  gl_FragColor.xyz = res_1.xyz;
  gl_FragColor.w = 1.0;
}


// stats: 14 alu 1 tex 0 flow
// inputs: 2
//  #0: var_TexCoords (high float) 2x1 [-1]
//  #1: var_Local0 (high float) 4x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

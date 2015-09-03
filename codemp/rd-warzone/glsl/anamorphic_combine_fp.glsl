uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;
varying vec4 var_Local0;
varying vec2 var_TexCoords;
void main ()
{
  gl_FragColor.xyz = (texture2D (u_DiffuseMap, var_TexCoords).xyz + (texture2D (u_NormalMap, var_TexCoords).xyz * var_Local0.x));
  gl_FragColor.w = 1.0;
}


// stats: 3 alu 2 tex 0 flow
// inputs: 2
//  #0: var_Local0 (high float) 4x1 [-1]
//  #1: var_TexCoords (high float) 2x1 [-1]
// textures: 2
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]
//  #1: u_NormalMap (high 2d) 0x0 [-1]

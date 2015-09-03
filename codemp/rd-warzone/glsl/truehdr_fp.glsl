uniform sampler2D u_TextureMap;
varying vec2 var_TexCoords;
void main ()
{
  gl_FragColor.xyz = (clamp ((texture2D (u_TextureMap, var_TexCoords).xyz - 0.04705882), 0.0, 1.0) * 1.164384);
  gl_FragColor.w = 1.0;
}


// stats: 4 alu 1 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

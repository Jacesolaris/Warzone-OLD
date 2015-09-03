uniform sampler2D u_TextureMap;
uniform vec4 u_Color;
uniform vec2 u_InvTexRes;
varying vec2 var_TexCoords;
void main ()
{
  vec4 color_1;
  color_1 = (texture2D (u_TextureMap, ((vec2(0.0, -1.0) * u_InvTexRes) + var_TexCoords)) * 0.25);
  color_1 = (color_1 + (texture2D (u_TextureMap, var_TexCoords) * 0.5));
  color_1 = (color_1 + (texture2D (u_TextureMap, (
    (vec2(0.0, 1.0) * u_InvTexRes)
   + var_TexCoords)) * 0.25));
  gl_FragColor = (color_1 * u_Color);
}


// stats: 10 alu 3 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 2 (total size: 0)
//  #0: u_Color (high float) 4x1 [-1]
//  #1: u_InvTexRes (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

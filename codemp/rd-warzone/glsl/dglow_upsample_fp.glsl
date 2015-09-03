uniform sampler2D u_TextureMap;
uniform vec2 u_InvTexRes;
varying vec2 var_TexCoords;
void main ()
{
  vec4 color_1;
  color_1 = (0.0625 * texture2D (u_TextureMap, (var_TexCoords - u_InvTexRes)));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(0.0, -1.0))
  ))));
  color_1 = (color_1 + (0.0625 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(1.0, -1.0))
  ))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(-1.0, 0.0))
  ))));
  color_1 = (color_1 + (0.25 * texture2D (u_TextureMap, var_TexCoords)));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(1.0, 0.0))
  ))));
  color_1 = (color_1 + (0.0625 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(-1.0, 1.0))
  ))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(0.0, 1.0))
  ))));
  color_1 = (color_1 + (0.0625 * texture2D (u_TextureMap, (var_TexCoords + u_InvTexRes))));
  gl_FragColor = color_1;
}


// stats: 31 alu 9 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_InvTexRes (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

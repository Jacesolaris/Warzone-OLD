uniform sampler2D u_TextureMap;
uniform vec2 u_InvTexRes;
varying vec2 var_TexCoords;
void main ()
{
  vec4 color_1;
  color_1 = (0.03125 * texture2D (u_TextureMap, (var_TexCoords + (u_InvTexRes * vec2(-2.0, -2.0)))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(0.0, -2.0))
  ))));
  vec2 tmpvar_2;
  tmpvar_2 = (u_InvTexRes * vec2(2.0, -2.0));
  color_1 = (color_1 + (0.03125 * texture2D (u_TextureMap, (var_TexCoords + tmpvar_2))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords - u_InvTexRes))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(1.0, -1.0))
  ))));
  color_1 = (color_1 + (0.03125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(-2.0, 0.0))
  ))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, var_TexCoords)));
  color_1 = (color_1 + (0.03125 * texture2D (u_TextureMap, (var_TexCoords + tmpvar_2))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(-1.0, 1.0))
  ))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + u_InvTexRes))));
  color_1 = (color_1 + (0.03125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(-2.0, 2.0))
  ))));
  color_1 = (color_1 + (0.125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(0.0, 2.0))
  ))));
  color_1 = (color_1 + (0.03125 * texture2D (u_TextureMap, (var_TexCoords + 
    (u_InvTexRes * vec2(2.0, 2.0))
  ))));
  gl_FragColor = color_1;
}


// stats: 46 alu 13 tex 0 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_InvTexRes (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

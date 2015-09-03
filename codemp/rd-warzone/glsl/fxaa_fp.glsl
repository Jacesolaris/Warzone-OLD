precision mediump float;
varying vec2 vTexCoord0;
uniform vec2 u_Dimensions;
uniform sampler2D u_TextureMap;
void main ()
{
  vec2 tmpvar_1;
  tmpvar_1 = (vTexCoord0 * u_Dimensions);
  vec2 tmpvar_2;
  tmpvar_2 = (1.0/(u_Dimensions));
  vec2 dir_3;
  vec4 color_4;
  vec4 tmpvar_5;
  tmpvar_5 = texture2D (u_TextureMap, (tmpvar_1 * tmpvar_2));
  float tmpvar_6;
  tmpvar_6 = dot (texture2D (u_TextureMap, ((tmpvar_1 + vec2(-1.0, -1.0)) * tmpvar_2)).xyz, vec3(0.299, 0.587, 0.114));
  float tmpvar_7;
  tmpvar_7 = dot (texture2D (u_TextureMap, ((tmpvar_1 + vec2(1.0, -1.0)) * tmpvar_2)).xyz, vec3(0.299, 0.587, 0.114));
  float tmpvar_8;
  tmpvar_8 = dot (texture2D (u_TextureMap, ((tmpvar_1 + vec2(-1.0, 1.0)) * tmpvar_2)).xyz, vec3(0.299, 0.587, 0.114));
  float tmpvar_9;
  tmpvar_9 = dot (texture2D (u_TextureMap, ((tmpvar_1 + vec2(1.0, 1.0)) * tmpvar_2)).xyz, vec3(0.299, 0.587, 0.114));
  float tmpvar_10;
  tmpvar_10 = dot (tmpvar_5.xyz, vec3(0.299, 0.587, 0.114));
  float tmpvar_11;
  tmpvar_11 = min (min (tmpvar_10, tmpvar_6), min (min (tmpvar_7, tmpvar_8), tmpvar_9));
  float tmpvar_12;
  tmpvar_12 = max (max (tmpvar_10, tmpvar_6), max (max (tmpvar_7, tmpvar_8), tmpvar_9));
  dir_3.x = ((tmpvar_8 + tmpvar_9) - (tmpvar_6 + tmpvar_7));
  dir_3.y = ((tmpvar_6 + tmpvar_8) - (tmpvar_7 + tmpvar_9));
  dir_3 = (min (vec2(8.0, 8.0), max (vec2(-8.0, -8.0), 
    (dir_3 * (1.0/((min (
      abs(dir_3.x)
    , 
      abs(dir_3.y)
    ) + max (
      (((tmpvar_6 + tmpvar_7) + (tmpvar_8 + tmpvar_9)) * 0.03125)
    , 0.0078125)))))
  )) * tmpvar_2);
  vec3 tmpvar_13;
  tmpvar_13 = (0.5 * (texture2D (u_TextureMap, (
    (tmpvar_1 * tmpvar_2)
   + 
    (dir_3 * -0.1666667)
  )).xyz + texture2D (u_TextureMap, (
    (tmpvar_1 * tmpvar_2)
   + 
    (dir_3 * 0.1666667)
  )).xyz));
  vec3 tmpvar_14;
  tmpvar_14 = ((tmpvar_13 * 0.5) + (0.25 * (texture2D (u_TextureMap, 
    ((tmpvar_1 * tmpvar_2) + (dir_3 * -0.5))
  ).xyz + texture2D (u_TextureMap, 
    ((tmpvar_1 * tmpvar_2) + (dir_3 * 0.5))
  ).xyz)));
  float tmpvar_15;
  tmpvar_15 = dot (tmpvar_14, vec3(0.299, 0.587, 0.114));
  if (((tmpvar_15 < tmpvar_11) || (tmpvar_15 > tmpvar_12))) {
    vec4 tmpvar_16;
    tmpvar_16.xyz = tmpvar_13;
    tmpvar_16.w = tmpvar_5.w;
    color_4 = tmpvar_16;
  } else {
    vec4 tmpvar_17;
    tmpvar_17.xyz = tmpvar_14;
    tmpvar_17.w = tmpvar_5.w;
    color_4 = tmpvar_17;
  };
  gl_FragColor = color_4;
}


// stats: 66 alu 9 tex 1 flow
// inputs: 1
//  #0: vTexCoord0 (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_Dimensions (high float) 2x1 [-1]
// textures: 1
//  #0: u_TextureMap (high 2d) 0x0 [-1]

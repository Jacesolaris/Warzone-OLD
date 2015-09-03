precision mediump float;
attribute vec3 attr_Position;
attribute vec2 attr_TexCoord0;
uniform mat4 u_ModelViewProjectionMatrix;
varying vec2 vTexCoord0;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = attr_Position;
  gl_Position = (u_ModelViewProjectionMatrix * tmpvar_1);
  vTexCoord0 = attr_TexCoord0;
}


// stats: 2 alu 0 tex 0 flow
// inputs: 2
//  #0: attr_Position (high float) 3x1 [-1]
//  #1: attr_TexCoord0 (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_ModelViewProjectionMatrix (high float) 4x4 [-1]

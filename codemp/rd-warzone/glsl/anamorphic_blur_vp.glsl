attribute vec3 attr_Position;
uniform vec4 u_Local0;
uniform vec2 u_Dimensions;
uniform mat4 u_ModelViewProjectionMatrix;
attribute vec4 attr_TexCoord0;
varying vec4 var_Local0;
varying vec2 var_Dimensions;
varying vec2 var_TexCoords;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = attr_Position;
  gl_Position = (u_ModelViewProjectionMatrix * tmpvar_1);
  var_TexCoords = attr_TexCoord0.xy;
  var_Dimensions = u_Dimensions;
  var_Local0 = u_Local0;
}


// stats: 2 alu 0 tex 0 flow
// inputs: 2
//  #0: attr_Position (high float) 3x1 [-1]
//  #1: attr_TexCoord0 (high float) 4x1 [-1]
// uniforms: 3 (total size: 0)
//  #0: u_Local0 (high float) 4x1 [-1]
//  #1: u_Dimensions (high float) 2x1 [-1]
//  #2: u_ModelViewProjectionMatrix (high float) 4x4 [-1]

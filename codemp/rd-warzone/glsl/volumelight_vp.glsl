attribute vec3 attr_Position;
attribute vec4 attr_TexCoord0;
uniform mat4 u_ModelViewProjectionMatrix;
uniform vec4 u_ViewInfo;
uniform vec2 u_Dimensions;
uniform vec4 u_Local0;
uniform vec4 u_Local1;
uniform vec4 u_Local2;
uniform vec4 u_Local3;
varying vec2 var_TexCoords;
varying vec4 var_ViewInfo;
varying vec2 var_Dimensions;
varying vec4 var_Local0;
varying vec4 var_Local1;
varying vec4 var_Local2;
varying vec4 var_Local3;
varying vec2 var_LightScreenPos;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = attr_Position;
  gl_Position = (u_ModelViewProjectionMatrix * tmpvar_1);
  var_TexCoords = attr_TexCoord0.xy;
  var_ViewInfo = u_ViewInfo;
  var_Dimensions = u_Dimensions;
  var_Local0.w = u_Local0.w;
  var_Local1 = u_Local1;
  var_Local2 = u_Local2;
  var_Local3 = u_Local3;
  var_LightScreenPos = u_Local2.xy;
}


// stats: 2 alu 0 tex 0 flow
// inputs: 2
//  #0: attr_Position (high float) 3x1 [-1]
//  #1: attr_TexCoord0 (high float) 4x1 [-1]
// uniforms: 7 (total size: 0)
//  #0: u_ModelViewProjectionMatrix (high float) 4x4 [-1]
//  #1: u_ViewInfo (high float) 4x1 [-1]
//  #2: u_Dimensions (high float) 2x1 [-1]
//  #3: u_Local0 (high float) 4x1 [-1]
//  #4: u_Local1 (high float) 4x1 [-1]
//  #5: u_Local2 (high float) 4x1 [-1]
//  #6: u_Local3 (high float) 4x1 [-1]

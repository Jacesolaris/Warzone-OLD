uniform mat4 u_ModelViewProjectionMatrix;
attribute vec4 attr_Position;
attribute vec2 attr_TexCoord0;
uniform vec2 u_Dimensions;
uniform vec3 u_ViewOrigin;
varying vec2 texCoord1;
varying vec2 var_Dimensions;
varying vec3 viewPos;
varying vec3 pPos;
varying vec3 viewAngles;
uniform float u_Time;
varying float time;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = attr_Position.xyz;
  gl_Position = (u_ModelViewProjectionMatrix * tmpvar_1);
  pPos = gl_Position.xyz;
  vec4 tmpvar_2;
  tmpvar_2.w = 1.0;
  tmpvar_2.xyz = u_ViewOrigin;
  viewPos = (u_ModelViewProjectionMatrix * tmpvar_2).xyz;
  viewAngles = normalize((u_ViewOrigin - viewPos));
  texCoord1 = attr_TexCoord0;
  var_Dimensions = u_Dimensions;
  time = u_Time;
}


// stats: 6 alu 0 tex 0 flow
// inputs: 2
//  #0: attr_Position (high float) 4x1 [-1]
//  #1: attr_TexCoord0 (high float) 2x1 [-1]
// uniforms: 4 (total size: 0)
//  #0: u_ModelViewProjectionMatrix (high float) 4x4 [-1]
//  #1: u_Dimensions (high float) 2x1 [-1]
//  #2: u_ViewOrigin (high float) 3x1 [-1]
//  #3: u_Time (high float) 1x1 [-1]

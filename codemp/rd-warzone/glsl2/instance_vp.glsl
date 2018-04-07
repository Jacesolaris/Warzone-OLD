#extension GL_EXT_draw_instanced : enable
//#extension GL_ARB_draw_instanced : enable

//attribute vec3 attr_Position;
//attribute vec4 attr_TexCoord0;

attribute vec3		attr_InstancesPosition;
attribute mat4		attr_InstancesMVP;

//uniform mat4   u_ModelViewProjectionMatrix;
//uniform mat4   u_ModelMatrix;

varying vec2   var_Tex1;

void main()
{
	gl_Position = attr_InstancesMVP * vec4(attr_InstancesPosition, 1.0);
	//var_Tex1 = attr_TexCoord0.st;
}

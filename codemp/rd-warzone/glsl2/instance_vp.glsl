#extension GL_EXT_draw_instanced : enable

attribute vec3 attr_Position;
attribute vec4 attr_TexCoord0;

//attribute mat4 attr_InstancesMVP;
attribute vec3 attr_InstancesPos;

uniform mat4   u_ModelViewProjectionMatrix;

varying vec2   var_Tex1;


void main()
{
	//gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	//gl_Position = attr_InstancesMVP * vec4(attr_Position, 1.0);
	//gl_Position = attr_InstancesMVP * vec4(attr_InstancesPos, 1.0);
	gl_Position = u_ModelViewProjectionMatrix * vec4(attr_InstancesPos, 1.0);
	var_Tex1 = attr_TexCoord0.st;
}

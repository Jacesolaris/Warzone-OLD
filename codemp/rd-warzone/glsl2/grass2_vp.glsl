attribute vec3		attr_Position;
attribute vec3		attr_Normal;

void main()
{
	gl_Position = vec4(attr_Position.xyz, 1.0);
}

attribute vec3	attr_Position;

void main()
{
	gl_Position = vec4(attr_Position.xyz, 1.0);
}

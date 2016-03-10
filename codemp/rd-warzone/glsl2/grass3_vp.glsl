attribute vec3		attr_Position;
attribute vec3		attr_Normal;

varying vec3		var_Normal;

void main()
{
	vec3 position  = attr_Position;
	vec3 normal    = attr_Normal * 0.5 + 0.5;// * 2.0 - vec3(1.0);

	gl_Position = vec4(position.xyz, 1.0);
	var_Normal.xyz = normal;
}

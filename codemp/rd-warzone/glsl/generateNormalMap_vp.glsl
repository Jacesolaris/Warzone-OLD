//#extension GL_EXT_gpu_shader4 : enable

attribute vec3 attr_Position;
attribute vec2 attr_TexCoord0;

uniform mat4   u_ModelViewProjectionMatrix;

varying vec2	var_TexCoords;

void main(void) {
	//gl_Position = u_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	//var_TexCoords = attr_TexCoord0.xy;

	const vec2 positions[4] = vec2[](
		vec2 (-1.0, -1.0),
		vec2 (1.0, -1.0),
		vec2 (1.0, 1.0),
		vec2 (-1.0, 1.0)
	);

	const vec2 texcoords[4] = vec2[](
		vec2 (0.0, 0.0),
		vec2 (1.0, 0.0),
		vec2 (1.0, 1.0),
		vec2 (0.0, 1.0)
	);

	//gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
	//var_TexCoords = texcoords[gl_VertexID];

	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	//gl_Position = gl_ModelViewProjectionMatrix * vec4(attr_Position, 1.0);
	//gl_Position = ftransform();
	//gl_Position = vec4(attr_TexCoord0.xy * 2.0 - 1.0, 0.0, 1.0);

	//var_TexCoords = attr_TexCoord0.xy;

	var_TexCoords   = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );
	gl_Position = vec4( var_TexCoords * 2.0 - 1.0, 0.0, 1.0 );
}

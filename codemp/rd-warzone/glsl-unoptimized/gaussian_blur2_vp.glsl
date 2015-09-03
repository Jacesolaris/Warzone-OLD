varying vec2 var_TexCoords;

void main()
{
	var_TexCoords   = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );
	gl_Position = vec4( var_TexCoords * 2.0 - 1.0, 0.0, 1.0 );
}

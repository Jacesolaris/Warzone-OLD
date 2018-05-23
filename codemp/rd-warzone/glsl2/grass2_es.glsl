//layout(triangles, equal_spacing) in;
layout(triangles, fractional_odd_spacing, ccw) in; // Does rend2 spew out clockwise or counter-clockwise verts???
//layout(triangles, equal_spacing, ccw) in;

uniform vec4						u_Local1; // MAP_SIZE, sway, overlaySway, materialType
uniform vec4						u_Local2; // hasSteepMap, hasWaterEdgeMap, haveNormalMap, SHADER_WATER_LEVEL
uniform vec4						u_Local3; // hasSplatMap1, hasSplatMap2, hasSplatMap3, hasSplatMap4
uniform vec4						u_Local8; // passnum, GRASS_DISTANCE_FROM_ROADS, GRASS_HEIGHT, 0.0
uniform vec4						u_Local9; // testvalue0, 1, 2, 3
uniform vec4						u_Local10; // foliageLODdistance, GRASS_UNDERWATER_ONLY, 0.0, GRASS_TYPE_UNIFORMALITY

in vec3 esPos[];

vec3 lerp3D(vec3 v0, vec3 v1, vec3 v2)
{
	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

int iLerp3D(int i0, int i1, int i2)
{
	return int(gl_TessCoord.x * float(i0) + gl_TessCoord.y * float(i1) + gl_TessCoord.z * int(i2));
}

void main() 
{
	vec3 pos;

	if (u_Local9.g >= 3.0)
		pos = lerp3D(esPos[0], esPos[1], esPos[2]);
	else if (u_Local9.g >= 2.0)
		pos = gl_TessCoord[2] * esPos[0] + gl_TessCoord[0] * esPos[1] + gl_TessCoord[1] * esPos[2];
	else if (u_Local9.g >= 1.0)
		pos = lerp3D(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);
	else
		pos = gl_TessCoord[2] * gl_in[0].gl_Position.xyz + gl_TessCoord[0] * gl_in[1].gl_Position.xyz + gl_TessCoord[1] * gl_in[2].gl_Position.xyz;

	gl_Position = vec4(pos, 1.0);
}

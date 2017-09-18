//layout(triangles, equal_spacing, cw) in;
layout(triangles, equal_spacing, ccw) in;

uniform mat4 u_ModelViewProjectionMatrix;

in precise vec3 WorldPos_ES_in[];
in precise vec2 TexCoord_ES_in[];
in precise vec3 Normal_ES_in[];
in precise vec3 ViewDir_ES_in[];
in precise vec4 Color_ES_in[];
in precise vec4 PrimaryLightDir_ES_in[];
in precise vec2 TexCoord2_ES_in[];
in precise vec3 Blending_ES_in[];
in float Slope_ES_in[];
in float usingSteepMap_ES_in[];

out precise vec3 WorldPos_GS_in;
out precise vec2 TexCoord_GS_in;
out precise vec3 Normal_GS_in;
out precise vec3 ViewDir_GS_in;
out precise vec4 Color_GS_in;
out precise vec4 PrimaryLightDir_GS_in;
out precise vec2 TexCoord2_GS_in;
out precise vec3 Blending_GS_in;
out float Slope_GS_in;
out float usingSteepMap_GS_in;

void main()
{
    vec3 p0 = gl_TessCoord.x * WorldPos_ES_in[0];
    vec3 p1 = gl_TessCoord.y * WorldPos_ES_in[1];
    vec3 p2 = gl_TessCoord.z * WorldPos_ES_in[2];
    //WorldPos_GS_in = normalize(p0 + p1 + p2);
	WorldPos_GS_in = p0 + p1 + p2;

    //gl_Position = u_ModelViewProjectionMatrix * vec4(normalize(p0 + p1 + p2), 1);
	gl_Position = vec4(p0 + p1 + p2, 1);

	Normal_GS_in = (gl_TessCoord.x * Normal_ES_in[0] + gl_TessCoord.y * Normal_ES_in[1] + gl_TessCoord.z * Normal_ES_in[2]);
    TexCoord_GS_in = (gl_TessCoord.x * TexCoord_ES_in[0] + gl_TessCoord.y * TexCoord_ES_in[1] + gl_TessCoord.z * TexCoord_ES_in[2]);
    WorldPos_GS_in = (gl_TessCoord.x * WorldPos_ES_in[0] + gl_TessCoord.y * WorldPos_ES_in[1] + gl_TessCoord.z * WorldPos_ES_in[2]);
	ViewDir_GS_in = (gl_TessCoord.x * ViewDir_ES_in[0] + gl_TessCoord.y * ViewDir_ES_in[1] + gl_TessCoord.z * ViewDir_ES_in[2]);
	Color_GS_in = (gl_TessCoord.x * Color_ES_in[0] + gl_TessCoord.y * Color_ES_in[1] + gl_TessCoord.z * Color_ES_in[2]);
	PrimaryLightDir_GS_in = (gl_TessCoord.x * PrimaryLightDir_ES_in[0] + gl_TessCoord.y * PrimaryLightDir_ES_in[1] + gl_TessCoord.z * PrimaryLightDir_ES_in[2]);
	TexCoord2_GS_in = (gl_TessCoord.x * TexCoord2_ES_in[0] + gl_TessCoord.y * TexCoord2_ES_in[1] + gl_TessCoord.z * TexCoord2_ES_in[2]);
	Blending_GS_in = (gl_TessCoord.x * Blending_ES_in[0] + gl_TessCoord.y * Blending_ES_in[1] + gl_TessCoord.z * Blending_ES_in[2]);
	Slope_GS_in = (gl_TessCoord.x * Slope_ES_in[0] + gl_TessCoord.y * Slope_ES_in[1] + gl_TessCoord.z * Slope_ES_in[2]);
	usingSteepMap_GS_in = (gl_TessCoord.x * usingSteepMap_ES_in[0] + gl_TessCoord.y * usingSteepMap_ES_in[1] + gl_TessCoord.z * usingSteepMap_ES_in[2]);
}

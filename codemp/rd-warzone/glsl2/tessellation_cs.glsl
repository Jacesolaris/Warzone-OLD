layout(vertices = 3) out;

uniform vec4			u_Local10;

#define TessLevelInner u_Local10.g
#define TessLevelOuter u_Local10.b

#define ID gl_InvocationID

// attributes of the input CPs
in vec4 WorldPos_CS_in[];
in vec3 Normal_CS_in[];
in vec2 TexCoord_CS_in[];
in vec3 ViewDir_CS_in[];
in vec4 Color_CS_in[];
in vec4 PrimaryLightDir_CS_in[];
in vec2 TexCoord2_CS_in[];
in vec3 Blending_CS_in[];
in float Slope_CS_in[];
in float usingSteepMap_CS_in[];

// attributes of the output CPs
out precise vec3 WorldPos_ES_in[3];
out precise vec3 Normal_ES_in[3];
out precise vec2 TexCoord_ES_in[3];
out precise vec3 ViewDir_ES_in[3];
out precise vec4 Color_ES_in[3];
out precise vec4 PrimaryLightDir_ES_in[3];
out precise vec2 TexCoord2_ES_in[3];
out precise vec3 Blending_ES_in[3];
out float Slope_ES_in[3];
out float usingSteepMap_ES_in[3];
 
void main()
{
    WorldPos_ES_in[ID] = WorldPos_CS_in[ID].xyz;
	Normal_ES_in[ID] = Normal_CS_in[ID];
	TexCoord_ES_in[ID] = TexCoord_CS_in[ID];
	ViewDir_ES_in[ID] = ViewDir_CS_in[ID];
	Color_ES_in[ID] = Color_CS_in[ID];
	PrimaryLightDir_ES_in[ID] = PrimaryLightDir_CS_in[ID];
	TexCoord2_ES_in[ID] = TexCoord2_CS_in[ID];
	Slope_ES_in[ID] = Slope_CS_in[ID];
	usingSteepMap_ES_in[ID] = usingSteepMap_CS_in[ID];

    if (ID == 0) {
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter;
        gl_TessLevelOuter[1] = TessLevelOuter;
        gl_TessLevelOuter[2] = TessLevelOuter;
    }
}

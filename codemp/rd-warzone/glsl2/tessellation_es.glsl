uniform mat4	u_ModelViewProjectionMatrix;
uniform mat4	u_ModelMatrix;

uniform vec4				u_Settings0; // useTC, useDeform, useRGBA, isTextureClamped
uniform vec4				u_Settings1; // useVertexAnim, useSkeletalAnim, useFog, is2D
uniform vec4				u_Settings2; // LIGHTDEF_USE_LIGHTMAP, LIGHTDEF_USE_GLOW_BUFFER, LIGHTDEF_USE_CUBEMAP, LIGHTDEF_USE_TRIPLANAR
uniform vec4				u_Settings3; // LIGHTDEF_USE_REGIONS, LIGHTDEF_IS_DETAIL

#define USE_TC				u_Settings0.r
#define USE_DEFORM			u_Settings0.g
#define USE_RGBA			u_Settings0.b
#define USE_TEXTURECLAMP	u_Settings0.a

#define USE_VERTEX_ANIM		u_Settings1.r
#define USE_SKELETAL_ANIM	u_Settings1.g
#define USE_FOG				u_Settings1.b
#define USE_IS2D			u_Settings1.a

#define USE_LIGHTMAP		u_Settings2.r
#define USE_GLOW_BUFFER		u_Settings2.g
#define USE_CUBEMAP			u_Settings2.b
#define USE_TRIPLANAR		u_Settings2.a

#define USE_REGIONS			u_Settings3.r
#define USE_ISDETAIL		u_Settings3.g

uniform float	u_Time;

out precise vec3 WorldPos_FS_in;
out precise vec2 TexCoord_FS_in;
out precise vec3 Normal_FS_in;
out precise vec3 ViewDir_FS_in;
out precise vec4 Color_FS_in;
out precise vec4 PrimaryLightDir_FS_in;
out precise vec2 TexCoord2_FS_in;
out precise vec3 Blending_FS_in;
flat out float Slope_FS_in;

#define WorldPos_GS_in WorldPos_FS_in
#define TexCoord_GS_in TexCoord_FS_in
#define Normal_GS_in Normal_FS_in
#define ViewDir_GS_in ViewDir_FS_in
#define Color_GS_in Color_FS_in
#define PrimaryLightDir_GS_in PrimaryLightDir_FS_in
#define TexCoord2_GS_in TexCoord2_FS_in
#define Blending_GS_in Blending_FS_in
#define Slope_GS_in Slope_FS_in

// PN patch data
struct PnPatch
{
	float b210;
	float b120;
	float b021;
	float b012;
	float b102;
	float b201;
	float b111;
	float n110;
	float n011;
	float n101;
};

uniform vec4 u_TesselationInfo;

#define uTessAlpha u_TesselationInfo.r

//layout(quads, fractional_odd_spacing, ccw) in;
layout(triangles, fractional_odd_spacing, ccw) in; // Does rend2 spew out clockwise or counter-clockwise verts???
//layout(triangles, equal_spacing, ccw) in;

uniform vec3			u_ViewOrigin;

uniform int    u_DeformGen;
uniform float  u_DeformParams[7];

in precise vec4 WorldPos_ES_in[];
in precise vec3 iNormal[];
in precise vec2 iTexCoord[];
in precise PnPatch iPnPatch[];
in precise vec4 Color_ES_in[];
in precise vec4 PrimaryLightDir_ES_in[];
in precise vec2 TexCoord2_ES_in[];
in precise vec3 Blending_ES_in[];
in float Slope_ES_in[];

#define b300    gl_in[0].gl_Position.xyz
#define b030    gl_in[1].gl_Position.xyz
#define b003    gl_in[2].gl_Position.xyz
#define n200    iNormal[0]
#define n020    iNormal[1]
#define n002    iNormal[2]
#define uvw     gl_TessCoord

vec3 DeformPosition(const vec3 pos, const vec3 normal, const vec2 st)
{
	float base = u_DeformParams[0];
	float amplitude = u_DeformParams[1];
	float phase = u_DeformParams[2];
	float frequency = u_DeformParams[3];
	float spread = u_DeformParams[4];

	if (u_DeformGen == DGEN_PROJECTION_SHADOW)
	{
		vec3 ground = vec3(
			u_DeformParams[0],
			u_DeformParams[1],
			u_DeformParams[2]);
		float groundDist = u_DeformParams[3];
		vec3 lightDir = vec3(
			u_DeformParams[4],
			u_DeformParams[5],
			u_DeformParams[6]);

		float d = dot(lightDir, ground);

		lightDir = lightDir * max(0.5 - d, 0.0) + ground;
		d = 1.0 / dot(lightDir, ground);

		vec3 lightPos = lightDir * d;

		return pos - lightPos * dot(pos, ground) + groundDist;
	}
	else if (u_DeformGen == DGEN_BULGE)
	{
		phase *= st.x;
	}
	else // if (u_DeformGen <= DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		phase += dot(pos.xyz, vec3(spread));
	}

	float value = phase + (u_Time * frequency);
	float func;

	if (u_DeformGen == DGEN_WAVE_SIN)
	{
		func = sin(value * 2.0 * M_PI);
	}
	else if (u_DeformGen == DGEN_WAVE_SQUARE)
	{
		func = sign(fract(0.5 - value));
	}
	else if (u_DeformGen == DGEN_WAVE_TRIANGLE)
	{
		func = abs(fract(value + 0.75) - 0.5) * 4.0 - 1.0;
	}
	else if (u_DeformGen == DGEN_WAVE_SAWTOOTH)
	{
		func = fract(value);
	}
	else if (u_DeformGen == DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		func = (1.0 - fract(value));
	}
	else // if (u_DeformGen == DGEN_BULGE)
	{
		func = sin(value);
	}

	return pos + normal * (base + func * amplitude);
}

void main()
{
	vec3 uvwSquared = uvw*uvw;
	vec3 uvwCubed = uvwSquared*uvw;

	// extract control points
	vec3 b210 = vec3(iPnPatch[0].b210, iPnPatch[1].b210, iPnPatch[2].b210);
	vec3 b120 = vec3(iPnPatch[0].b120, iPnPatch[1].b120, iPnPatch[2].b120);
	vec3 b021 = vec3(iPnPatch[0].b021, iPnPatch[1].b021, iPnPatch[2].b021);
	vec3 b012 = vec3(iPnPatch[0].b012, iPnPatch[1].b012, iPnPatch[2].b012);
	vec3 b102 = vec3(iPnPatch[0].b102, iPnPatch[1].b102, iPnPatch[2].b102);
	vec3 b201 = vec3(iPnPatch[0].b201, iPnPatch[1].b201, iPnPatch[2].b201);
	vec3 b111 = vec3(iPnPatch[0].b111, iPnPatch[1].b111, iPnPatch[2].b111);

	// extract control normals
	vec3 n110 = normalize(vec3(iPnPatch[0].n110,
		iPnPatch[1].n110,
		iPnPatch[2].n110));
	vec3 n011 = normalize(vec3(iPnPatch[0].n011,
		iPnPatch[1].n011,
		iPnPatch[2].n011));
	vec3 n101 = normalize(vec3(iPnPatch[0].n101,
		iPnPatch[1].n101,
		iPnPatch[2].n101));

	// compute texcoords
	WorldPos_GS_in = gl_TessCoord[2] * WorldPos_ES_in[0].xyz
		+ gl_TessCoord[0] * WorldPos_ES_in[1].xyz
		+ gl_TessCoord[1] * WorldPos_ES_in[2].xyz;
	TexCoord_GS_in = gl_TessCoord[2] * iTexCoord[0]
		+ gl_TessCoord[0] * iTexCoord[1]
		+ gl_TessCoord[1] * iTexCoord[2];
	Color_GS_in = gl_TessCoord[2] * Color_ES_in[0]
		+ gl_TessCoord[0] * Color_ES_in[1]
		+ gl_TessCoord[1] * Color_ES_in[2];
	PrimaryLightDir_GS_in = gl_TessCoord[2] * PrimaryLightDir_ES_in[0]
		+ gl_TessCoord[0] * PrimaryLightDir_ES_in[1]
		+ gl_TessCoord[1] * PrimaryLightDir_ES_in[2];
	TexCoord2_GS_in = gl_TessCoord[2] * TexCoord2_ES_in[0]
		+ gl_TessCoord[0] * TexCoord2_ES_in[1]
		+ gl_TessCoord[1] * TexCoord2_ES_in[2];
	Blending_GS_in = gl_TessCoord[2] * Blending_ES_in[0]
		+ gl_TessCoord[0] * Blending_ES_in[1]
		+ gl_TessCoord[1] * Blending_ES_in[2];
	Slope_GS_in = gl_TessCoord[2] * Slope_ES_in[0]
		+ gl_TessCoord[0] * Slope_ES_in[1]
		+ gl_TessCoord[1] * Slope_ES_in[2];

	// normal
	vec3 barNormal = gl_TessCoord[2] * iNormal[0]
		+ gl_TessCoord[0] * iNormal[1]
		+ gl_TessCoord[1] * iNormal[2];
	vec3 pnNormal = n200*uvwSquared[2]
		+ n020*uvwSquared[0]
		+ n002*uvwSquared[1]
		+ n110*uvw[2] * uvw[0]
		+ n011*uvw[0] * uvw[1]
		+ n101*uvw[2] * uvw[1];
	Normal_GS_in = uTessAlpha*pnNormal + (1.0 - uTessAlpha)*barNormal;
	//Normal_GS_in = gl_TessCoord[2] * iNormal[0]
	//	+ gl_TessCoord[0] * iNormal[1]
	//	+ gl_TessCoord[1] * iNormal[2];

	// compute interpolated pos
	vec3 barPos = gl_TessCoord[2] * b300
		+ gl_TessCoord[0] * b030
		+ gl_TessCoord[1] * b003;

	// save some computations
	uvwSquared *= 3.0;

	// compute PN position
	vec3 pnPos = b300*uvwCubed[2]
		+ b030*uvwCubed[0]
		+ b003*uvwCubed[1]
		+ b210*uvwSquared[2] * uvw[0]
		+ b120*uvwSquared[0] * uvw[2]
		+ b201*uvwSquared[2] * uvw[1]
		+ b021*uvwSquared[0] * uvw[1]
		+ b102*uvwSquared[1] * uvw[2]
		+ b012*uvwSquared[1] * uvw[0]
		+ b111*6.0*uvw[0] * uvw[1] * uvw[2];

	// final position and normal
	vec3 finalPos = (1.0 - uTessAlpha)*barPos + uTessAlpha*pnPos;

#ifndef __USE_GEOM_SHADER__
	gl_Position = u_ModelViewProjectionMatrix * vec4(finalPos, 1.0);

	if (USE_DEFORM == 1.0)
	{
		finalPos = DeformPosition(finalPos, Normal_GS_in, TexCoord_GS_in.st);
	}

	finalPos = (u_ModelMatrix * vec4(finalPos, 1.0)).xyz;
	Normal_GS_in = (u_ModelMatrix * vec4(Normal_GS_in, 0.0)).xyz;
	//Normal_GS_in = uTessAlpha*(u_ModelMatrix * vec4(pnNormal, 0.0) + (1.0 - uTessAlpha)*(u_ModelMatrix * vec4(barNormal, 0.0);

#else //__USE_GEOM_SHADER__
	gl_Position = vec4(finalPos, 1.0);
#endif //__USE_GEOM_SHADER__
	WorldPos_GS_in = finalPos.xyz;
	ViewDir_GS_in = u_ViewOrigin - finalPos;
}

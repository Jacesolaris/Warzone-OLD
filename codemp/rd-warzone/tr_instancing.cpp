#include "tr_local.h"

#ifdef __INSTANCED_MODELS__

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "VectorUtils3.h"

void setupInstancedVertexAttributes(mdvModel_t *m)
{
	qglUseProgram(tr.instanceShader.program);
	//qglGenBuffers(1, &tr.instanceShader.instances_mvp);
	qglGenBuffers(1, &tr.instanceShader.instances_buffer);
}

void drawModelInstanced(mdvModel_t *m, GLuint count, vec3_t *positions, vec3_t *angles, matrix_t MVP) 
{
	qglUseProgram(tr.instanceShader.program);

	//GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, MVP);
	GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformVec4(&tr.instanceShader, UNIFORM_COLOR, colorWhite);

	GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);

	//shader_t *shader = tr.shaders[m->surfaces->shaderIndexes[0]];
	//image_t *image = shader->stages[0]->bundle[0].image[0];
	//GL_BindToTMU(shader->stages[0]->bundle[0].image[0], TB_DIFFUSEMAP);
	GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);

	if (m && m->vao) 
	{
		qglBindVertexArray(m->vao);	// Select VAO
	} 
	else 
	{
		ri->Printf(PRINT_WARNING, "Warning warning, fuckup in drawmodelinstanced - Model has no VAO!\n");
		R_BindNullVBO();
		R_BindNullIBO();
		qglBindVertexArray(0);
		GLSL_BindProgram(NULL);
		return;
	}

	//tess.vbo = m->vboSurfaces->vbo;
	//tess.ibo = m->vboSurfaces->ibo;

	R_BindVBO(m->vboSurfaces->vbo);
	R_BindIBO(m->vboSurfaces->ibo);

	GLSL_VertexAttribPointers(ATTR_INDEX_POSITION | ATTR_INDEX_NORMAL | ATTR_INDEX_TEXCOORD0 /*| ATTR_INDEX_INSTANCES_MVP*/ | ATTR_INDEX_INSTANCES_POS);
	GLSL_VertexAttribsState(ATTR_INDEX_POSITION | ATTR_INDEX_NORMAL | ATTR_INDEX_TEXCOORD0 /*| ATTR_INDEX_INSTANCES_MVP*/ | ATTR_INDEX_INSTANCES_POS);

	//RB_UpdateVBOs(ATTR_INDEX_POSITION | ATTR_INDEX_NORMAL | ATTR_INDEX_TEXCOORD0 /*| ATTR_INDEX_INSTANCES_MVP*/ | ATTR_INDEX_INSTANCES_POS);

#if 0
	qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * count, model_matrixes, GL_STATIC_DRAW);
#endif

	qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * count, positions, GL_STATIC_DRAW);

	//qglBufferSubData(GL_ARRAY_BUFFER, m->vboSurfaces->vbo->ofs_instances, m->vboSurfaces->numVerts * sizeof(vec3_t), positions);


#if 0
	mat4 transEverything;
	//memcpy(transEverything.m, MVP, sizeof(MVP));
	transEverything.m[0] = glState.modelviewProjection[0];
	transEverything.m[1] = glState.modelviewProjection[1];
	transEverything.m[2] = glState.modelviewProjection[2];
	transEverything.m[3] = glState.modelviewProjection[3];
	transEverything.m[4] = glState.modelviewProjection[4];
	transEverything.m[5] = glState.modelviewProjection[5];
	transEverything.m[6] = glState.modelviewProjection[6];
	transEverything.m[7] = glState.modelviewProjection[7];
	transEverything.m[8] = glState.modelviewProjection[8];
	transEverything.m[9] = glState.modelviewProjection[9];
	transEverything.m[10] = glState.modelviewProjection[10];
	transEverything.m[11] = glState.modelviewProjection[11];
	transEverything.m[12] = glState.modelviewProjection[12];
	transEverything.m[13] = glState.modelviewProjection[13];
	transEverything.m[14] = glState.modelviewProjection[14];
	transEverything.m[15] = glState.modelviewProjection[15];
	
	mat4 model_matrixes[MAX_INSTANCED_MODEL_INSTANCES];// = { 0 };

	for (int pos = 0; pos < count; pos++) 
	{
		
		model_matrixes[pos] = 
			Mult(
				Mult(
					Mult(transEverything, Ry(angles[pos][0]))	, T(positions[pos][0], positions[pos][1], positions[pos][2])) 	, Rz(angles[pos][1]));
		
		model_matrixes[pos] = Transpose(model_matrixes[pos]);
		
		/*Matrix16Multiply(transEverything, Ry(time + (float)pos / randoms[pos]), model_matrixes[pos]);
		Matrix16Multiply(model_matrixes[pos], T((float)pos / 300 + randoms[pos] * (float)pos / 3000, (float)pos / 75, 1), model_matrixes[pos]);
		Matrix16Multiply(model_matrixes[pos], Rz(time * (pos % 12) + randoms[pos]), model_matrixes[pos]);*/
	}
#endif

	qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_UNSIGNED_INT, 0, count);
	//qglFlushMappedBufferRange();

	R_BindNullVBO();
	R_BindNullIBO();
	qglBindVertexArray(0);
	GLSL_BindProgram(NULL);
}

#endif //__INSTANCED_MODELS__

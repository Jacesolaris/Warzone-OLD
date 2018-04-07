#include "tr_local.h"

#ifdef __INSTANCED_MODELS__

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "VectorUtils3.h"

void setupInstancedVertexAttributes(mdvModel_t *m)
{
	GLSL_BindProgram(&tr.instanceShader);
	qglGenBuffers(1, &tr.instanceShader.instances_buffer);
	qglGenBuffers(1, &tr.instanceShader.instances_mvp);
}

void drawModelInstanced(mdvModel_t *m, GLuint count, vec3_t *positions, matrix_t *model_matrixes, vec3_t *angles, matrix_t MVP)
{
	RB_EndSurface();

	//qglUseProgram(tr.instanceShader.program);
	GLSL_BindProgram(&tr.instanceShader);

	//GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, MVP);
	GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);
	GLSL_SetUniformVec4(&tr.instanceShader, UNIFORM_COLOR, colorWhite);

	GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);

	//shader_t *shader = tr.shaders[m->surfaces->shaderIndexes[0]];
	//image_t *image = shader->stages[0]->bundle[0].image[0];
	//GL_BindToTMU(shader->stages[0]->bundle[0].image[0], TB_DIFFUSEMAP);
	GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);

	if (m->vao == NULL)
	{
		ri->Printf(PRINT_WARNING, "Warning warning, fuckup in drawmodelinstanced - Model has no VAO!\n");
		R_BindNullVBO();
		R_BindNullIBO();
		qglBindVertexArray(0);
		GLSL_BindProgram(NULL);
		return;
	}

	qglBindVertexArray(m->vao);	// Select VAO
	qglEnableVertexAttribArray(m->vao);

	qglBindBuffer(GL_ARRAY_BUFFER, m->vboSurfaces->vbo->vertexesVBO);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->vboSurfaces->ibo->indexesVBO);

	//tess.useInternalVBO = qtrue;

	R_BindVBO(m->vboSurfaces->vbo);
	R_BindIBO(m->vboSurfaces->ibo);
	//tess.ibo = m->vboSurfaces->ibo;

	GLSL_VertexAttribsState(/*ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0 |*/ ATTR_INSTANCES_MVP | ATTR_INSTANCES_POSITION);
	GLSL_VertexAttribPointers(/*ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0 |*/ ATTR_INSTANCES_MVP | ATTR_INSTANCES_POSITION);

	//qglEnableVertexAttribArray(ATTR_INDEX_INSTANCES_MVP);
	//qglVertexAttribPointer(ATTR_INDEX_INSTANCES_MVP, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//qglEnableVertexAttribArray(ATTR_INDEX_INSTANCES_POSITION);
	//qglVertexAttribPointer(ATTR_INDEX_INSTANCES_POSITION, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(NR_VERTICES * sizeof(vertices[0])));

	//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
	//qglBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * count, positions, /*GL_STATIC_DRAW*/GL_DYNAMIC_DRAW);
	qglBufferSubData(GL_ARRAY_BUFFER, tess.vbo->ofs_instancesPosition, count * sizeof(vec3_t), (const GLvoid *)positions);

	//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
	//qglBufferData(GL_ARRAY_BUFFER, sizeof(matrix_t) * count, model_matrixes, /*GL_STATIC_DRAW*/GL_DYNAMIC_DRAW);
	qglBufferSubData(GL_ARRAY_BUFFER, tess.vbo->ofs_instancesMVP, count * sizeof(matrix_t), (const GLvoid *)model_matrixes);

	//ForceCrash();
	
#if 1
	//qglDrawElements(GL_TRIANGLES, count, GL_INDEX_TYPE, 0);
	//qglDrawArraysInstanced(GL_TRIANGLES, 0, m->vboSurfaces->numIndexes, count);
	qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_INDEX_TYPE, 0, count);
#else
	tess.numIndexes += count * m->vboSurfaces->numIndexes * m->vboSurfaces->numIndexes;
	tess.numVertexes += count * m->vboSurfaces->numIndexes;
	tess.minIndex = 0;
	tess.maxIndex = count * m->vboSurfaces->numIndexes * m->vboSurfaces->numIndexes;

	RB_EndSurface();
#endif

	R_BindNullVBO();
	R_BindNullIBO();
	qglBindVertexArray(0);
	GLSL_BindProgram(NULL);
}

#endif //__INSTANCED_MODELS__

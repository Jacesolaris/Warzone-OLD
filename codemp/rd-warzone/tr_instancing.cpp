#include "tr_local.h"

#ifdef __INSTANCED_MODELS__

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "VectorUtils3.h"

void drawModelInstanced(mdvModel_t *m, GLuint count, vec3_t *positions, matrix_t *model_matrixes, vec3_t *angles, matrix_t MVP)
{
	RB_EndSurface();

	if (m->vao == NULL)
	{
		ri->Printf(PRINT_WARNING, "Warning warning, fuckup in drawmodelinstanced - Model has no VAO!\n");
		R_BindNullVBO();
		R_BindNullIBO();
		qglBindVertexArray(0);
		GLSL_BindProgram(NULL);
		return;
	}

	R_BindVBO(m->vboSurfaces->vbo);
	R_BindIBO(m->vboSurfaces->ibo);

	qglBindVertexArray(m->vao);	// Select VAO
	qglEnableVertexAttribArray(m->vao);

	GLSL_VertexAttribsState(/*ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0 |*/ ATTR_INSTANCES_MVP | ATTR_INSTANCES_POSITION);
	GLSL_VertexAttribPointers(/*ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0 |*/ ATTR_INSTANCES_MVP | ATTR_INSTANCES_POSITION);

	qglBufferSubData(GL_ARRAY_BUFFER, m->ofs_instancesPosition, count * sizeof(positions[0]), positions);
	qglBufferSubData(GL_ARRAY_BUFFER, m->ofs_instancesMVP, count * sizeof(model_matrixes[0]), model_matrixes);

	//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
	//qglBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * count, positions, GL_STREAM_DRAW);

	//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
	//qglBufferData(GL_ARRAY_BUFFER, sizeof(model_matrixes[0]) * count, model_matrixes, GL_STREAM_DRAW);


	//ForceCrash();

	GLSL_BindProgram(&tr.instanceShader);

	GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);

	GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);

	//qglBindVertexArray(m->vao);	// Select VAO

	//qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->vboSurfaces->ibo->indexesVBO);
	//qglBindBuffer(GL_ARRAY_BUFFER, m->vboSurfaces->vbo->vertexesVBO);
	//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
	//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);

	//qglBufferSubData(GL_ARRAY_BUFFER, glState.currentVBO->ofs_instancesPosition, count * sizeof(origins[0]), origins);
	//qglBufferSubData(GL_ARRAY_BUFFER, glState.currentVBO->ofs_instancesMVP, count * sizeof(mvps[0]), mvps);

	qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_INDEX_TYPE, 0, count);
	//qglDrawArraysInstanced(GL_TRIANGLES, 0, m->vboSurfaces->numIndexes, count);
	//qglDrawElements(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_UNSIGNED_INT, 0);

	qglBindVertexArray(0);
	R_BindNullVBO();
	R_BindNullIBO();
	
	GLSL_BindProgram(NULL);
}

#endif //__INSTANCED_MODELS__

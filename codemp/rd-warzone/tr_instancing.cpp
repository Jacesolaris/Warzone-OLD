#include "tr_local.h"

#ifdef __INSTANCED_MODELS__

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "VectorUtils3.h"

int			INSTANCED_MODEL_TYPES = 0;

int			INSTANCED_MODEL_COUNT[MAX_INSTANCED_MODEL_TYPES] = { 0 };
mdvModel_t	*INSTANCED_MODEL_MODEL[MAX_INSTANCED_MODEL_TYPES] = { NULL };
vec3_t		INSTANCED_MODEL_ORIGINS[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 0 };
matrix_t	INSTANCED_MODEL_MATRIXES[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 0 };
vec3_t		INSTANCED_MODEL_ANGLES[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 0 };

void R_AddInstancedModelToList(mdvModel_t *model, vec3_t origin, vec3_t angles, matrix_t model_matrix, trRefEntity_t *ent)
{
	qboolean	FOUND = qfalse;
	int			modelID = 0;

	for (modelID = 0; modelID < INSTANCED_MODEL_TYPES && modelID < MAX_INSTANCED_MODEL_TYPES; modelID++)
	{
		if (INSTANCED_MODEL_MODEL[modelID] == NULL)
		{
			break;
		}

		if (INSTANCED_MODEL_MODEL[modelID] == model)
		{
			FOUND = qtrue;
			break;
		}
	}

	if (!FOUND)
	{
		if (INSTANCED_MODEL_TYPES + 1 >= MAX_INSTANCED_MODEL_TYPES) return; // Uh oh...

		INSTANCED_MODEL_TYPES++;
		INSTANCED_MODEL_MODEL[modelID] = model;
	}

	if (INSTANCED_MODEL_COUNT[modelID] + 1 < MAX_INSTANCED_MODEL_INSTANCES)
	{
		VectorCopy(origin, INSTANCED_MODEL_ORIGINS[modelID][INSTANCED_MODEL_COUNT[modelID]]);
		VectorCopy(angles, INSTANCED_MODEL_ANGLES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		//Matrix16Copy(model_matrix, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);
		//Matrix16Multiply(glState.modelviewProjection, glState.modelview/*model_matrix*/, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		// set up the transformation matrix

		R_RotateForEntity(ent, &tr.viewParms, &tr.ori);
		//GL_SetModelviewMatrix(backEnd.ori.modelViewMatrix);
		Matrix16Multiply(tr.viewParms.projectionMatrix/*glState.projection*/, tr.ori.modelViewMatrix, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		//ForceCrash();

		//GLSL_SetUniformMatrix16(sp, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);
		//GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
		//Matrix16Multiply(glState.modelviewProjection, backEnd.ori.modelMatrix, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		INSTANCED_MODEL_COUNT[modelID]++;
	}
}

void R_AddInstancedModelsToScene(void)
{
	if (INSTANCED_MODEL_TYPES <= 0)
	{
		return;
	}

	//ForceCrash();

	// First finish any drawing...
	RB_EndSurface();

	GLSL_BindProgram(&tr.instanceShader);

	// Draw them for this scene...
	for (int modelID = 0; modelID < INSTANCED_MODEL_TYPES && modelID < MAX_INSTANCED_MODEL_TYPES; modelID++)
	{
		if (INSTANCED_MODEL_COUNT[modelID] > 0)
		{
			mdvModel_t *m = INSTANCED_MODEL_MODEL[modelID];
			GLuint count = INSTANCED_MODEL_COUNT[modelID];

			if (m->vao == NULL)
			{
				ri->Printf(PRINT_WARNING, "Warning warning, fuckup in drawmodelinstanced - Model has no VAO!\n");
				continue;
			}

			GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
			GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);

			R_BindVBO(m->vboSurfaces->vbo);
			R_BindIBO(m->vboSurfaces->ibo);

			qglBindVertexArray(m->vao);	// Select VAO
			qglEnableVertexAttribArray(m->vao);

#define __INSTANCING_USE_REND2_ATTRIB_CODE__

#ifndef __INSTANCING_USE_REND2_ATTRIB_CODE__
			GLSL_VertexAttribsState(/*ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0 |*/ ATTR_INSTANCES_POSITION | ATTR_INSTANCES_MVP);
			GLSL_VertexAttribPointers(/*ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0 |*/ ATTR_INSTANCES_POSITION | ATTR_INSTANCES_MVP);

			qglBufferSubData(GL_ARRAY_BUFFER, m->ofs_instancesPosition, count * sizeof(vec3_t), INSTANCED_MODEL_ORIGINS[modelID);
			qglBufferSubData(GL_ARRAY_BUFFER, m->ofs_instancesMVP, count * sizeof(matrix_t), INSTANCED_MODEL_MATRIXES[modelID]);

			//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
			//qglBufferData(GL_ARRAY_BUFFER, count * sizeof(vec3_t), INSTANCED_MODEL_ORIGINS[modelID], GL_STREAM_DRAW);

			//qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
			//qglBufferData(GL_ARRAY_BUFFER, count * sizeof(matrix_t), INSTANCED_MODEL_MATRIXES[modelID], GL_STREAM_DRAW);
#else //!__INSTANCING_USE_REND2_ATTRIB_CODE__
			qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
			qglBufferData(GL_ARRAY_BUFFER, count * sizeof(vec3_t), INSTANCED_MODEL_ORIGINS[modelID], GL_STREAM_DRAW);
			qglEnableVertexAttribArray(ATTR_INDEX_INSTANCES_POSITION);
			qglVertexAttribPointer(ATTR_INDEX_INSTANCES_POSITION, 3 * MAX_INSTANCED_MODEL_INSTANCES, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
			qglVertexAttribDivisor(ATTR_INDEX_INSTANCES_POSITION, 1);

			qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
			qglBufferData(GL_ARRAY_BUFFER, count * sizeof(matrix_t), INSTANCED_MODEL_MATRIXES[modelID], GL_STREAM_DRAW);
			qglEnableVertexAttribArray(ATTR_INDEX_INSTANCES_MVP);
			qglVertexAttribPointer(ATTR_INDEX_INSTANCES_MVP, 16 * MAX_INSTANCED_MODEL_INSTANCES, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)/*BUFFER_OFFSET(m->ofs_instancesMVP)*/);
			qglVertexAttribDivisor(ATTR_INDEX_INSTANCES_MVP, 1);
#endif //__INSTANCING_USE_REND2_ATTRIB_CODE__

			//ForceCrash();

			qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_INDEX_TYPE, 0, count);
			//qglDrawArraysInstanced(GL_TRIANGLES, 0, m->vboSurfaces->numIndexes, count);
			//qglDrawElements(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_UNSIGNED_INT, 0);

#ifdef __INSTANCING_USE_REND2_ATTRIB_CODE__
			qglDisableVertexAttribArray(ATTR_INDEX_INSTANCES_POSITION);
			qglDisableVertexAttribArray(ATTR_INDEX_INSTANCES_MVP);
#endif //__INSTANCING_USE_REND2_ATTRIB_CODE__
			qglDisableVertexAttribArray(m->vao);
		}
	}

	// Clear the buffer ready for next scene...
	for (int modelID = 0; modelID < INSTANCED_MODEL_TYPES && modelID < MAX_INSTANCED_MODEL_TYPES; modelID++)
	{
		INSTANCED_MODEL_COUNT[modelID] = 0;
		INSTANCED_MODEL_MODEL[modelID] = NULL;
	}

	INSTANCED_MODEL_TYPES = 0;

	R_BindNullVBO();
}
#endif //__INSTANCED_MODELS__

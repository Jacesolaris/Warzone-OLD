#include "tr_local.h"

#ifdef __INSTANCED_MODELS__

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "VectorUtils3.h"

int			INSTANCED_MODEL_TYPES = 0;

int				INSTANCED_MODEL_COUNT[MAX_INSTANCED_MODEL_TYPES] = { 0 };
mdvModel_t		*INSTANCED_MODEL_MODEL[MAX_INSTANCED_MODEL_TYPES] = { NULL };
vec3_t			INSTANCED_MODEL_ORIGINS[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 0 };
//matrix_t		INSTANCED_MODEL_MATRIXES[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 0 };
vec3_t			INSTANCED_MODEL_ANGLES[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 0 };
vec3_t			INSTANCED_MODEL_SCALES[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { 1 };
//trRefEntity_t	*INSTANCED_MODEL_ENTITIES[MAX_INSTANCED_MODEL_TYPES][MAX_INSTANCED_MODEL_INSTANCES] = { NULL };

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
			if (INSTANCED_MODEL_COUNT[modelID] + 1 < MAX_INSTANCED_MODEL_INSTANCES)
			{
				FOUND = qtrue;
				break;
			}
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
		VectorCopy(ent->e.modelScale, INSTANCED_MODEL_SCALES[modelID][INSTANCED_MODEL_COUNT[modelID]]);
		//INSTANCED_MODEL_ENTITIES[modelID][INSTANCED_MODEL_COUNT[modelID]] = ent;
#if 0
		VectorCopy(angles, INSTANCED_MODEL_ANGLES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		//Matrix16Copy(model_matrix, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);
		//Matrix16Copy(glState.modelviewProjection, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);
		//Matrix16Multiply(glState.modelviewProjection, glState.modelview/*model_matrix*/, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		// set up the transformation matrix

		//R_RotateForEntity(ent, &tr.viewParms, &tr.ori);
		//Matrix16Multiply(tr.viewParms.projectionMatrix/*glState.projection*/, tr.ori.modelViewMatrix, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);
		//Matrix16Copy(glState.modelviewProjection, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		backEnd.currentEntity = ent;
		backEnd.refdef.floatTime = backEnd.currentEntity->e.shaderTime;

		// set up the transformation matrix
		R_RotateForEntity(backEnd.currentEntity, &backEnd.viewParms, &backEnd.ori);
		Matrix16Copy(backEnd.ori.modelViewMatrix, glState.modelview);
		Matrix16Multiply(glState.projection, glState.modelview, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);

		//ForceCrash();

		//GLSL_SetUniformMatrix16(sp, UNIFORM_MODELMATRIX, backEnd.ori.modelMatrix);
		//GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
		//Matrix16Multiply(glState.modelviewProjection, backEnd.ori.modelMatrix, INSTANCED_MODEL_MATRIXES[modelID][INSTANCED_MODEL_COUNT[modelID]]);
#endif

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

	GLSL_BindProgram(&tr.instanceShader);

	FBO_Bind(tr.renderFbo);

	SetViewportAndScissor();
	//GL_SetProjectionMatrix(backEnd.viewParms.projectionMatrix);
	//GL_SetModelviewMatrix(backEnd.viewParms.world.modelViewMatrix);
	GL_SetModelviewMatrix(backEnd.viewParms.world.modelViewMatrix);

	GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformVec3(&tr.instanceShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg);
	GLSL_SetUniformFloat(&tr.instanceShader, UNIFORM_TIME, backEnd.refdef.floatTime);

	//GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
	//GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);

	//GL_Cull(CT_TWO_SIDED);
	//GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_LESS | GLS_ATEST_GT_0);

#define __INSTANCING_USE_UNIFORMS__

	// Draw them for this scene...
	for (int modelID = 0; modelID < INSTANCED_MODEL_TYPES && modelID < MAX_INSTANCED_MODEL_TYPES; modelID++)
	{
		if (INSTANCED_MODEL_COUNT[modelID] > 0)
		{
			mdvModel_t *m = INSTANCED_MODEL_MODEL[modelID];
			GLuint count = INSTANCED_MODEL_COUNT[modelID];

			if (r_instancing->integer >= 2)
			{
				ri->Printf(PRINT_WARNING, "ModelID %i. Count %i.\n", modelID, count);
			}

#ifndef __INSTANCING_USE_UNIFORMS__
			if (m->vao == NULL)
			{
				ri->Printf(PRINT_WARNING, "Warning warning, fuckup in drawmodelinstanced - Model has no VAO!\n");
				continue;
			}

			qglBindVertexArray(m->vao);	// Select VAO
			qglEnableVertexAttribArray(m->vao);

			R_BindVBO(m->vboSurfaces->vbo);
			R_BindIBO(m->vboSurfaces->ibo);

			qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
			qglBufferData(GL_ARRAY_BUFFER, count * sizeof(vec3_t), INSTANCED_MODEL_ORIGINS[modelID], GL_STREAM_DRAW);
			qglEnableVertexAttribArray(ATTR_INDEX_INSTANCES_POSITION);
			qglVertexAttribPointer(ATTR_INDEX_INSTANCES_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
			qglVertexAttribDivisor(ATTR_INDEX_INSTANCES_POSITION, 1);

			qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
			qglBufferData(GL_ARRAY_BUFFER, count * sizeof(matrix_t), INSTANCED_MODEL_MATRIXES[modelID], GL_STREAM_DRAW);
			qglEnableVertexAttribArray(ATTR_INDEX_INSTANCES_MVP);
			qglVertexAttribPointer(ATTR_INDEX_INSTANCES_MVP, 16, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)/*BUFFER_OFFSET(m->ofs_instancesMVP)*/);
			qglVertexAttribDivisor(ATTR_INDEX_INSTANCES_MVP, 1);

			//ForceCrash();

			qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_INDEX_TYPE, 0, count);

			qglDisableVertexAttribArray(ATTR_INDEX_INSTANCES_POSITION);
			qglDisableVertexAttribArray(ATTR_INDEX_INSTANCES_MVP);

			qglDisableVertexAttribArray(m->vao);
#else //__INSTANCING_USE_UNIFORMS__
			for (int j = 0; j < m->numVBOSurfaces; j++)
			{
				R_BindVBO(m->vboSurfaces[j].vbo);
				R_BindIBO(m->vboSurfaces[j].ibo);

				shader_t *shader = tr.shaders[m->vboSurfaces[j].mdvSurface->shaderIndexes[0]];
				
				for (int stage = 0; stage <= shader->maxStage && stage < MAX_SHADER_STAGES; stage++)
				{
					shaderStage_t *pStage = shader->stages[stage];

					if (!pStage)
					{// How does this happen???
						break;
					}

					if (!pStage->active)
					{// Shouldn't this be here, just in case???
						continue;
					}

					uint32_t stateBits = pStage->stateBits;

					if (pStage->isFoliage)
					{
						stateBits = GLS_DEPTHMASK_TRUE | GLS_DEPTHFUNC_LESS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_ATEST_GE_128;
						pStage->stateBits = stateBits;

						if (!(backEnd.depthFill || (backEnd.viewParms.flags & VPF_SHADOWPASS)) && (stateBits & GLS_ATEST_BITS))
						{
							GL_Cull(CT_TWO_SIDED);
						}
					}

					GLSL_VertexAttribsState(ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0);
					GLSL_VertexAttribPointers(ATTR_POSITION | ATTR_NORMAL | ATTR_TEXCOORD0);

					if ((backEnd.depthFill || (backEnd.viewParms.flags & VPF_SHADOWPASS)) && !(stateBits & GLS_ATEST_BITS))
					{
						GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
						GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);
					}
					else
					{
						GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);
						GL_BindToTMU(pStage->bundle[TB_DIFFUSEMAP].image[0], TB_DIFFUSEMAP);
					}

					GLSL_SetUniformVec3xX(&tr.instanceShader, UNIFORM_INSTANCE_POSITIONS, INSTANCED_MODEL_ORIGINS[modelID], count);
					GLSL_SetUniformVec3xX(&tr.instanceShader, UNIFORM_INSTANCE_SCALES, INSTANCED_MODEL_SCALES[modelID], count);
					//GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_INSTANCE_MATRIXES, (const float *)INSTANCED_MODEL_MATRIXES[modelID], count);

					vec4_t l0;
					VectorSet4(l0, shader->materialType, 0.0, 0.0, 0.0);
					GLSL_SetUniformVec4(&tr.instanceShader, UNIFORM_SETTINGS0, l0);
					//ForceCrash();

					//UpdateTexCoords(pStage);

					GL_State(stateBits);

					qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces[j].numIndexes, GL_INDEX_TYPE, 0, count);
				}
			}
#endif //__INSTANCING_USE_UNIFORMS__
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

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
	if (m && m->vao)
		qglBindVertexArray(m->vao);	// Select VAO
	else {
		ri->Printf(PRINT_WARNING, "Warning warning, fuckup in drawmodelinstanced - Model has no VAO!\n");
		return;
	}

	qglUseProgram(tr.instanceShader.program);

	GLSL_SetUniformMatrix16(&tr.instanceShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, MVP);
	GLSL_SetUniformVec4(&tr.instanceShader, UNIFORM_COLOR, colorWhite);

	GLSL_SetUniformInt(&tr.instanceShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP);

	//shader_t *shader = tr.shaders[m->surfaces->shaderIndexes[0]];
	//image_t *image = shader->stages[0]->bundle[0].image[0];
	//GL_BindToTMU(shader->stages[0]->bundle[0].image[0], TB_DIFFUSEMAP);
	GL_BindToTMU(tr.whiteImage, TB_DIFFUSEMAP);

	R_BindVBO(m->vboSurfaces->vbo);
	R_BindIBO(m->vboSurfaces->ibo);

	mat4 transEverything;
	//memcpy(transEverything.m, MVP, sizeof(MVP));
	transEverything.m[0] = MVP[0];
	transEverything.m[1] = MVP[1];
	transEverything.m[2] = MVP[2];
	transEverything.m[3] = MVP[3];
	transEverything.m[4] = MVP[4];
	transEverything.m[5] = MVP[5];
	transEverything.m[6] = MVP[6];
	transEverything.m[7] = MVP[7];
	transEverything.m[8] = MVP[8];
	transEverything.m[9] = MVP[9];
	transEverything.m[10] = MVP[10];
	transEverything.m[11] = MVP[11];
	transEverything.m[12] = MVP[12];
	transEverything.m[13] = MVP[13];
	transEverything.m[14] = MVP[14];
	transEverything.m[15] = MVP[15];
	
#if 0
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

	GLSL_VertexAttribPointers(ATTR_INDEX_POSITION | ATTR_INDEX_NORMAL | ATTR_INDEX_TEXCOORD0 /*| ATTR_INDEX_INSTANCES_MVP*/ | ATTR_INDEX_INSTANCES_POS);
	GLSL_VertexAttribsState(ATTR_INDEX_POSITION | ATTR_INDEX_NORMAL | ATTR_INDEX_TEXCOORD0 /*| ATTR_INDEX_INSTANCES_MVP*/ | ATTR_INDEX_INSTANCES_POS);

#if 0
	qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_mvp);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * count, model_matrixes, GL_STATIC_DRAW);
#endif

	qglBindBuffer(GL_ARRAY_BUFFER, tr.instanceShader.instances_buffer);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * count, positions, GL_STATIC_DRAW);

	qglDrawElementsInstanced(GL_TRIANGLES, m->vboSurfaces->numIndexes, GL_UNSIGNED_INT, 0, count);

	R_BindNullVBO();
	R_BindNullIBO();
	qglBindVertexArray(0);
	GLSL_BindNullProgram();
}

#endif //__INSTANCED_MODELS__

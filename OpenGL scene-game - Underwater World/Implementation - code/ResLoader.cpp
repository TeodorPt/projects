#include "ResLoader.h"
#include <stdlib.h>
#include <stdio.h>

/*
Load an .obj model from the file specified into the GLMmodel specified. Also sets it
with an original given scale.
*/
void loadModel(GLMmodel **pmodel, char*filename, GLfloat scale) {
	*pmodel=glmReadOBJ(filename);
	if (!*pmodel) exit(1);
	
	glmUnitize(*pmodel);
	glmScale(*pmodel,scale);
	glmFacetNormals(*pmodel);
	glmVertexNormals(*pmodel,90.0);
}

void drawModel(GLMmodel *pmodel,GLuint mode) {		
	glmDraw(pmodel,mode);
}

/*
Load a .tga texture from the file specified.
*/
void loadTexture(char *path, GLuint *tex_name)	{
	glGenTextures(1,tex_name);
	loadTGA(path,*tex_name);
}
#ifndef _RESLOADER_H_
#define _RESLOADER_H_
#include "glm.h"
#include "tga.h"

void loadModel(GLMmodel **pmodel, char*filename, GLfloat scale);
void drawModel(GLMmodel *pmodel, GLuint mode);
void loadTexture(char *path, GLuint *tex_name);

#endif
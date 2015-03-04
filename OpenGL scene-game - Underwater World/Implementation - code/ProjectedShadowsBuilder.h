#ifndef _PROJSHADOWBUILD_H_
#define _PROJSHADOWBUILD_H_
#include "Object.h"

class ProjectedShadowsBuilder {
	private:
		GLfloat projectionMat[16];
	public:
		ProjectedShadowsBuilder(GLfloat planeEq[4], GLfloat lightPos[4]);
		void placeShadow(Object *object);
};

#endif
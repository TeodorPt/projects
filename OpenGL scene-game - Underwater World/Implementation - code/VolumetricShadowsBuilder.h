#ifndef _VOLSHADOWBUILD_H_
#define _VOLSHADOWBUILD_H_
#include "Object.h"
#include <stdlib.h>

class VolumetricShadowsBuilder {
	public:
		void makeShadowVolume(Object* object, GLfloat lp[4]);
		void drawShadowVolume();
		void drawSceneWithShadow(void (*drawFunction)(int));
};

#endif
#ifndef _OBJECT_H_
#define _OBJECT_H_
#include "ResLoader.h"

enum {
	PASS_NORMAL, PASS_CAUSTIC
};

/*
Class which represents and object in the scene. The object
is defined by a material, by a model (GLMmodel), by position,
rotation on all axes and by a texture.
The class provides methods for setting the position and rotation
of the object, and for drawing the object.
*/
class Object {
	private:
		GLfloat material_ambient[4], material_diffuse[4], material_specular[4];
		GLMmodel *object_model;
		GLuint tex_name; 
		GLfloat posX, posY, posZ;
		GLfloat rotAngle;
		GLfloat rotX, rotY, rotZ;
	public:
		Object(char* object_url, char* texture_url, float size);
		void draw(int pass);
		void setPosition(float x, float y, float z) {
			posX = x; posY = y; posZ = z;
		};
		void setRotation(float x, float y, float z) {
			rotX = x; rotY = y; rotZ = z;
		};
		GLfloat getPosX() { return posX; };
		GLfloat getPosY() { return posY; };
		GLfloat getPosZ() { return posZ; };
		GLMmodel* getModel() {return object_model;};
};

#endif
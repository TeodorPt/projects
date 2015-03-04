#include "Object.h"
#include <stdio.h>

/*
Constructor for the Object class. Receives the model and texture URLs
and the size of the object. It loads the model and the associated texture,
and scales the object.
The scale size means that the object will be scaled to fit in the cube with 
"size" dimension.
*/
Object::Object(char* object_url, char* texture_url, float size){

	if (texture_url!=nullptr) 
		loadTexture(texture_url,&tex_name);

	loadModel(&object_model,object_url,size);
	
	for (int i=0;i<4;i++) {
		material_ambient[i] = 1;
		material_diffuse[i] = 1;
		material_specular[i] = 1;
	}

	posX=posY=posZ = 0;
	rotX=rotY=rotZ = 0;
}

/*
Draw the object. Actually places the object in the scene
at its position and with its rotation values. It will
leave the modelview matrix unmodified afterwards (it will
revert it to the previous state).
*/
void Object::draw(int pass){
	if (pass == PASS_NORMAL) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D,tex_name);
	}

	
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,material_ambient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,material_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);

	glPushMatrix();
		glTranslatef(posX,posY,posZ);
		glRotatef(rotX,1,0,0);
		glRotatef(rotY,0,1,0);
		glRotatef(rotZ,0,0,1);
		drawModel(object_model,GLM_SMOOTH|GLM_TEXTURE);
	glPopMatrix();
	
}

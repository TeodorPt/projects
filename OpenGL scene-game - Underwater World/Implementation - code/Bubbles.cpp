#include "Bubbles.h"
#include <random>
#include "ResLoader.h"


GLfloat bubble_ambient[] = {0.01, 0.01, 0.2, 0.1};
GLfloat bubble_diffuse[] = {1,1,1,0.1};
GLfloat bubble_specular[] = {1,1,1,1};

GLMmodel *bubble;

void generateBubble(float *bubbleX, float *bubbleZ, float *bubbleSpeed) {
	*bubbleX = rand()%200 - 100;
	*bubbleZ = rand()%200 - 100;
	*bubbleSpeed = rand()%10+1;
}


/*
Generate all the initial bubbles positions in the scene. This needs to be called only once, at program
start.
*/
void Bubbles::genAllBubbles() {
	for (int i=0; i<NUM_BUBBLES;i++) {
		bubbleY[i]=0;
		generateBubble(&bubbleX[i],&bubbleZ[i],&bubbleSpeed[i]);
	}
}


Bubbles::Bubbles(){
	loadModel(&bubble,"res/obj/bubble.obj",0.5);

	genAllBubbles();
}

/*
Draw all the bubbles in the scene.
*/
void Bubbles::drawBubbles() {
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,bubble_ambient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,bubble_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,bubble_specular);
	
	
	glDepthMask(GL_FALSE);
	for (int i=0;i<NUM_BUBBLES;i++) {		
		glPushMatrix();
		glTranslatef(bubbleX[i],bubbleY[i],bubbleZ[i]);
		drawModel(bubble,GLM_SMOOTH|GLM_NONE);
		glPopMatrix();
	}

	glDepthMask(GL_TRUE);
	
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}

void Bubbles::moveBubbles() {
	for (int i=0;i<NUM_BUBBLES;i++) {
		bubbleY[i]+=bubbleSpeed[i]/100;
		if (bubbleY[i]>50) {
			generateBubble(&bubbleX[i],&bubbleZ[i],&bubbleSpeed[i]);
			bubbleY[i]=0;
		}
	}
}
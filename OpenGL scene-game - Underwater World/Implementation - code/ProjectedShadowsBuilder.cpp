#include "ProjectedShadowsBuilder.h"


void matrixInverse(GLfloat OpenGLmatIn[16], GLfloat invMatrix[16])
{
    GLfloat matIn[4][4];
    GLfloat matOut[4][4];

	// OpenGL matrix is column major matrix in 1x16 array. Convert it to row major 4x4 matrix
    for(int m=0, k=0; m<=3, k<16; m++)
      for(int n=0;n<=3;n++)
      {
        matIn[m][n] = OpenGLmatIn[k];
        k++;
      }

    // 3x3 rotation Matrix Transpose ( it is equal to invering rotations) . Since rotation matrix is anti-symmetric matrix, transpose is equal to Inverse.
    for(int i=0 ; i<3; i++){
    for(int j=0; j<3; j++){
      matOut[j][i] = matIn[i][j];
     }
    }

    // Negate the translations ( equal to inversing translations)
    GLfloat vTmp[3];
 
    vTmp[0] = -matIn[3][0];
    vTmp[1] = -matIn[3][1];
    vTmp[2] = -matIn[3][2];

    // Roatate this vector using the above newly constructed rotation matrix
    matOut[3][0] = vTmp[0]*matOut[0][0] + vTmp[1]*matOut[1][0] + vTmp[2]*matOut[2][0];
    matOut[3][1] = vTmp[0]*matOut[0][1] + vTmp[1]*matOut[1][1] + vTmp[2]*matOut[2][1];
    matOut[3][2] = vTmp[0]*matOut[0][2] + vTmp[1]*matOut[1][2] + vTmp[2]*matOut[2][2];
 
    // Take care of the unused part of the OpenGL 4x4 matrix
    matOut[0][3] = matOut[1][3] = matOut[2][3] = 0.0f;
    matOut[3][3] = 1.0f;

	int k=0;
		for (int i=0;i<4;i++) 
			for (int j=0;j<4;j++) {
				invMatrix[k] = matOut[i][j];
				k++;
			}
}


void computeShadowMatrix(GLfloat shadowMat[16], GLfloat plane[4], GLfloat lightPos[4])
{
	GLfloat dotProduct;

	dotProduct = plane[0] * lightPos[0] +
				 plane[1] * lightPos[1] +
				 plane[2] * lightPos[2] +
				 plane[3] * lightPos[3];

	shadowMat[0] = dotProduct - lightPos[0] * plane[0];
	shadowMat[1] = 0.0f - lightPos[1] * plane[0];
	shadowMat[2] = 0.0f - lightPos[2] * plane[0];
	shadowMat[3] = 0.0f - lightPos[3] * plane[0];

	shadowMat[4] = 0.0f - lightPos[0] * plane[1];
	shadowMat[5] = dotProduct - lightPos[1] * plane[1];
	shadowMat[6] = 0.0f - lightPos[2] * plane[1];
	shadowMat[7] = 0.0f - lightPos[3] * plane[1];

	shadowMat[8] = 0.0f - lightPos[0] * plane[2];		
	shadowMat[9] = 0.0f - lightPos[1] * plane[2];
	shadowMat[10] = dotProduct - lightPos[2] * plane[2];
	shadowMat[11] = 0.0f - lightPos[3] * plane[2];

	shadowMat[12] = 0.0f - lightPos[0] * plane[3];
	shadowMat[13] = 0.0f - lightPos[1] * plane[3];		
	shadowMat[14] = 0.0f - lightPos[2] * plane[3];		
	shadowMat[15] = dotProduct - lightPos[3] * plane[3];
}


ProjectedShadowsBuilder::ProjectedShadowsBuilder(GLfloat planeEq[4], GLfloat lightPos[4]) {
	computeShadowMatrix(projectionMat,planeEq,lightPos);
}

void ProjectedShadowsBuilder::placeShadow(Object *object) {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 1, 1);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		glColor4f(0.05f, 0.05f, 0.1f, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		
		//this is done in order to move the multMatrix on the projection matrix, rather than on the modelview matrix
		//so that it does not break up the fog calculations
		GLfloat modelView[16];
		GLfloat invModelView[16];
		glGetFloatv(GL_MODELVIEW_MATRIX,modelView);

		matrixInverse(modelView,invModelView); //get the modelview inverse matrix
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glMultMatrixf(modelView);		//P'=P*M
			glMultMatrixf(projectionMat);   //P'=P*M*S
			glMultMatrixf(invModelView);	//P'=P*M*S*M^-1

			glMatrixMode(GL_MODELVIEW);

			object->draw(PASS_NORMAL);		//P'*M = P*M*S*M^-1*M = P*M*S
			
			glMatrixMode(GL_PROJECTION);
		glPopMatrix();
			
		glMatrixMode(GL_MODELVIEW);
		
		
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		
		glEnable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
}
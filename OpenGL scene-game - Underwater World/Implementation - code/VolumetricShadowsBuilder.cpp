#include "VolumetricShadowsBuilder.h"

struct Edge
{
	GLuint i0; //index of the first vertex of the contour edge
	GLuint i1; //index of the second vertex of the contour edge
	Edge *next; //pointer to the next element in the contour edges list
};

struct Edge *first, *last;

const GLint SHADOWVOL = 1000;



GLfloat dotProduct(GLfloat v0[3], GLfloat v1[3])
{
	return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}

//////////////////////////////////////////////////////////////////////////
//		Standard functions for using dynamic single linked lists		//
//////////////////////////////////////////////////////////////////////////
unsigned int listList()
{
	Edge *e = first;
	unsigned int count = 0;
	
	while (e != NULL)
	{
		count++;
		e = e->next;
	}

	return count;
}

void deleteList()
{
	Edge *e = first;
	Edge *temp;

	while (e != NULL)
	{
		temp = e;
		e = e->next;
		free(temp);
	}
	first = last = NULL;
}

bool isInList(GLuint i0, GLuint i1)
{
	Edge *e = first;

	while (e != NULL)
	{
		if (((e->i0 == i0) && (e->i1 == i1)) ||			
			((e->i0 == i1) && (e->i1 == i0)))
			return true;
		e = e->next;
	}
return false;
}

void addToList(GLuint i0, GLuint i1)
{
	Edge *e;
	
	e = (Edge*) malloc(sizeof(Edge));
	e->i0 = i0;
	e->i1 = i1;
	e->next = NULL;

	if (first == NULL)
	{
		first = e;
		last = e;
	}
	else
	{
		last->next = e;
		last = e;
	}	
}

void removeFromList(GLuint i0, GLuint i1)
{
	Edge *e = first;
	Edge *temp;
	
	if (((e->i0 == i0) && (e->i1 == i1)) ||			
			((e->i0 == i1) && (e->i1 == i0)))
	{
		if (first == last)
		{
			first = last = NULL;
		}
		else
		{
			first = e->next;
		}
		free(e);
	}
	else
	{
		while (e->next != NULL)
		{
			if (((e->next->i0 == i0) && (e->next->i1 == i1)) ||			
				((e->next->i0 == i1) && (e->next->i1 == i0)))
			{
				temp = e->next;
				e->next = e->next->next;
				if (temp == last)
				{
					last = e;
				}
				free(temp);
			}
			else
				e = e->next;
		}
	}
}

void findContourEdges(Object *object, GLfloat lp[4])
{
	GLfloat fvIncidentLightDir[3];
	GLfloat fvFacetNormal[3];

	GLMmodel* model = object->getModel();

	//for each face of the object (triangles in our case)
	for (unsigned int i = 0; i < model->numtriangles; i++)
	{
		//compute the light vector (between the center of the current 
		//triangle and the position of the light (converted to object space)
		for (unsigned int j = 0; j < 3; j++)
		{
			fvIncidentLightDir[j] = (model->vertices[3*model->triangles[i].vindices[0]+j] +
									model->vertices[3*model->triangles[i].vindices[1]+j] +
									model->vertices[3*model->triangles[i].vindices[2]+j]) / 3.0 - lp[j];			
		}
		//get the X, Y and Z coordinates for the current triangle's facet normal
		fvFacetNormal[0] = model->facetnorms[3*model->triangles[i].findex];
		fvFacetNormal[1] = model->facetnorms[3*model->triangles[i].findex+1];
		fvFacetNormal[2] = model->facetnorms[3*model->triangles[i].findex+2];
		//check if the current triangle is front-facing (towards the light)
		//or back-facing (away from the light), by computing the dot product
		//between the light vector and the normal

		//if the polygon is facing away from the light
		if (dotProduct(fvIncidentLightDir,fvFacetNormal) >= 0.0)
		{
			//check all eges of the triangle

			//if the first edge of the triangle is already in the list
			if (isInList(model->vertices[model->triangles[i].vindices[0]], model->vertices[model->triangles[i].vindices[1]]))
				//then it can't be a contour edge since it is
				// referenced by two triangles that are facing
				// away from the light
				removeFromList(model->triangles[i].vindices[0], model->triangles[i].vindices[1]);
			else
				addToList(model->triangles[i].vindices[0], model->triangles[i].vindices[1]);

			//if the second edge of the triangle is already in the list
			if (isInList(model->vertices[model->triangles[i].vindices[1]], model->vertices[model->triangles[i].vindices[2]]))
				//then it can't be a contour edge since it is
				// referenced by two triangles that are facing
				// away from the light
				removeFromList(model->triangles[i].vindices[1], model->triangles[i].vindices[2]);
			else
				addToList(model->triangles[i].vindices[1], model->triangles[i].vindices[2]);

			//if the third edge of the triangle is already in the list
			if (isInList(model->vertices[model->triangles[i].vindices[2]], model->vertices[model->triangles[i].vindices[0]]))
				//then it can't be a contour edge since it is
				// referenced by two triangles that are facing
				// away from the light
				removeFromList(model->triangles[i].vindices[2], model->triangles[i].vindices[0]);
			else
				addToList(model->triangles[i].vindices[2], model->triangles[i].vindices[0]);
		}		
	}
}

void ExtendVertex(GLfloat extended[3], GLfloat light[3], GLfloat vertex[3], GLfloat magnitude)
{
	GLfloat delta[3];

	//compute the light vector (the direction of the extension)
	delta[0] = vertex[0] - light[0];
	delta[1] = vertex[1] - light[1];
	delta[2] = vertex[2] - light[2];

	//compute the extended point
	extended[0] = light[0] + delta[0] * magnitude;
	extended[1] = light[1] + delta[1] * magnitude;
	extended[2] = light[2] + delta[2] * magnitude;
}

//multiply a matrix with a column vector
void VMatMult(GLfloat M[16], GLfloat v[4])

{

    GLfloat res[4];                         // Hold Calculated Results

    res[0]=M[ 0]*v[0]+M[ 4]*v[1]+M[ 8]*v[2]+M[12]*v[3];
    res[1]=M[ 1]*v[0]+M[ 5]*v[1]+M[ 9]*v[2]+M[13]*v[3];
    res[2]=M[ 2]*v[0]+M[ 6]*v[1]+M[10]*v[2]+M[14]*v[3];
    res[3]=M[ 3]*v[0]+M[ 7]*v[1]+M[11]*v[2]+M[15]*v[3];

    v[0]=res[0];                            // Results Are Stored Back In v[]
    v[1]=res[1];
    v[2]=res[2];
    v[3]=res[3];                            // Homogenous Coordinate

}

void VolumetricShadowsBuilder::makeShadowVolume(Object* object, GLfloat lp[4]) 
{
		GLfloat extVert[3];
		Edge *e;
		GLfloat Minv[16];
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		

		glPushMatrix();
		glTranslatef(0,50,0);
		
		glGetFloatv(GL_MODELVIEW_MATRIX,Minv);
		glPopMatrix();

		
		//apply the inverse matrix to the light position
		VMatMult(Minv, lp);     
		
		//find contour edges
		findContourEdges(object, lp);
		
		//create a display list for the shadow volume
		glNewList(SHADOWVOL, GL_COMPILE);
		glDisable(GL_LIGHTING);
		
		e = first;
		glPushMatrix();
		glTranslatef(0,50,0);
		glBegin(GL_QUADS);  
			//set shadow colume color
			glColor3f(.2f, .8f, .4f);
			//traverse the list of edges
			while (e != NULL)
			{
				//create a quad between the contour edge's points and two extended points
				GLMmodel* model = object->getModel();
				glVertex3fv(&model->vertices[3*e->i1]);
				ExtendVertex(extVert, lp, &model->vertices[3*e->i1], 100);
				glVertex3fv(extVert);
				ExtendVertex(extVert, lp, &model->vertices[3*e->i0], 100);
				glVertex3fv(extVert);
				glVertex3fv(&model->vertices[3*e->i0]);
		
				e = e->next;
			}
		glEnd();
		glPopMatrix();
		
		glEnable(GL_LIGHTING);
		glEndList();
		
		deleteList();
}

void VolumetricShadowsBuilder::drawShadowVolume() 
{
	glCallList(SHADOWVOL);
}

void VolumetricShadowsBuilder::drawSceneWithShadow(void (*drawFunction)(int)) 
{
			glClear(GL_STENCIL_BUFFER_BIT);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			(*drawFunction)(PASS_NORMAL);  // render scene in depth buffer only

			//enable stencil testing
			glEnable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST);
			//make the depth buffer read-only
			glDepthMask(GL_FALSE);
			//set the stencil function to always pass
			glStencilFunc(GL_ALWAYS, 0, 0);
			glEnable(GL_CULL_FACE);
			
			//set the stencil buffer to keep all values except when depth test fails
			//when the depth test fails we increment the stencil buffer
		
			glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			glCullFace(GL_BACK);  // increment when we encounter a front face of the shadow volume
			glCallList(SHADOWVOL); //draw the shadow volume in the stencil buffer

			//set the stencil buffer to keep all values except when depth test fails
			//when the depth test fails we decrement the stencil buffer
			glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
			glCullFace(GL_FRONT);  // decrement using front face of shadow volume
			glCallList(SHADOWVOL); //draw the shadow volume in the stencil buffer

			//make the depth buffer write-enabled
			glDepthMask(GL_TRUE);
			//make the color buffer write-enabled
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			//cull back-facing polygons for speedup
			glCullFace(GL_BACK);
			//set depth testing function
			glDepthFunc(GL_LEQUAL);
			//set the stencil buffer to keep all data
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glDisable(GL_CULL_FACE);
			
			glStencilFunc(GL_NOTEQUAL, 0, 1);  //draw shadowed part
			glDisable(GL_LIGHT0);				
			(*drawFunction)(PASS_NORMAL); 

			glStencilFunc(GL_EQUAL, 0, 1);  // draw lit part
			glEnable(GL_LIGHT0);
			(*drawFunction)(PASS_NORMAL);

			glDepthFunc(GL_LESS);
			glDisable(GL_STENCIL_TEST);
}

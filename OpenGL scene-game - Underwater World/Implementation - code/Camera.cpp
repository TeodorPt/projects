#include "Camera.h"
#include <stdio.h>
#include <math.h>
#include "glm.h"


Camera::Camera(float x, float y, float z) {
	mvX = -x;
	mvY = -y;
	mvZ = -z;
}

void Camera::move(float x, float y, float z) {
	double rX = -rotX*0.01745329252;
	double rY = -rotY*0.01745329252;

	double dirX = x*cos(rX) + y*sin(rX)*sin(rY) + z*sin(rX)*cos(rY);
	double dirY = y*cos(rY) - z*sin(rY);
	double dirZ = -x*sin(rX) + y*sin(rY)*cos(rX) + z*cos(rX)*cos(rY);

	mvX+=dirX;
	if (mvX>200 || mvX<-200) mvX-=dirX;
	mvY+=dirY;
	if (mvY>-5) mvY-=dirY;
	mvZ+=dirZ;
	if (mvZ>200 || mvZ<-200) mvZ-=dirZ;
}

void Camera::place() {
	glRotatef(rotY,1,0,0);
	glRotatef(rotX,0,1,0);
	glTranslatef(mvX,mvY,mvZ);
}

void Camera::rotate(float dispX, float dispY) {
	rotX+=dispX;
	if (rotX>360) rotX=0; //reset values in the -360 - 360 range
	if (rotX<-360) rotX=0;
		
	if((rotY<80&&dispY>0) || (rotY>-80 && dispY<0)) rotY += dispY; //do not allow rotation greater than 80, or smaller than -80 on Y axis (camera can not turn upside down)
        
}
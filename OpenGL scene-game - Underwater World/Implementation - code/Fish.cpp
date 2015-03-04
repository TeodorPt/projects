#include "Fish.h"
#include <random>

#define RAD_TO_DEG 57.295777

float dispX,dispY,dispZ, len;

/*
Constructor for the Fish class. As an extra to the Object constructor, the
Fish also receives a speed parameter, which will dictate the speed of movement
of the fish.
*/
Fish::Fish(char* object_url, char* texture_url, float size, float speed):Object(object_url, texture_url, size) {
	targetX = rand()%200-100;
	targetY = rand()%40+5;
	targetZ = rand()%200-100;
	fishAngle = 0;
	fishAngleDir = 1;
	fishSpeed = speed;
	fishStep = 0;
}

/*
Moves the fish in random directions. This is obtained by the following technique:
choose a random target position, make a move towards that position. When the
target position is reached, choose another random target and repeat the process.
All of the time, rotate the fish to face the respective movement direction.
*/
void Fish::moveFish() {
	float posX = Object::getPosX();
	float posY = Object::getPosY();
	float posZ = Object::getPosZ();
	
	dispX = targetX-posX;
	dispY = targetY-posY;
	dispZ = targetZ-posZ;
	
	len = sqrt(dispX*dispX+dispY*dispY+dispZ*dispZ);


	if (abs(posX-targetX)<2 && abs(posY-targetY)<2 && abs(posZ-targetZ)){
		targetX = rand()%200-100;
		targetY = rand()%40+5;
		targetZ = rand()%200-100;
	}
	
	Object::setPosition(posX+dispX/(len*5)*fishSpeed,posY+dispY/(len*5)*fishSpeed,posZ+dispZ/(len*5)*fishSpeed);
	
	Object::setRotation(0,
		-dispZ/abs(dispZ)* //to have correct rotation sign
			acos(dispX/sqrt(dispX*dispX+dispZ*dispZ))*RAD_TO_DEG + fishAngle, //calculate the angle between the original fish position and the present direction, on the y=0 plane
		dispY/abs(dispY)*acos(abs(dispX)/sqrt(dispX*dispX+dispY*dispY))*RAD_TO_DEG/3
	); //calculate the angle between the original fish position and the present direction, on the z=0 plane

	fishAngle+=fishAngleDir*fishSpeed;
	if (abs(fishAngle)>10) {
		fishAngleDir = -fishAngleDir;
	}
}

/*
Move the fish in a circle path, with the given radius.
*/
void Fish::moveCircular(GLfloat radius) {
	Object::setPosition(-radius*cos(fishStep),25,-radius*sin(fishStep));
	fishStep+=fishSpeed*M_PI/2000.;
	Object::setRotation(0,90-fishStep*RAD_TO_DEG + fishAngle,0);

	fishAngle+=fishAngleDir*fishSpeed;
	if (abs(fishAngle)>10) {
		fishAngleDir = -fishAngleDir;
	}
}

void Fish::draw(int pass) {
	Object::draw(pass);
}

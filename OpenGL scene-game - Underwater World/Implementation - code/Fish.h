#ifndef _FISH_H_
#define _FISH_H_

#include "Object.h"

/*
Fish class extends the Object class and adds the animation
functions (movement), so that the fish can be animated
automatically.
*/
class Fish : public Object {
	private:
		GLfloat targetX, targetY, targetZ;
		GLfloat fishSpeed;
		GLfloat fishAngle, fishAngleDir;
		GLfloat fishStep;
	public:
		Fish(char* object_url, char* texture_url, float size, float speed);
		void moveFish();
		void moveCircular(GLfloat radius);
		void draw(int pass);
};

#endif

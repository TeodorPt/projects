#ifndef _CAMERA_H_
#define _CAMERA_H_

/*
Camera class, which provides functions for the camera movement.
The rotate() function will rotate the camera with the given values
along Ox (on y=0 plane) and alon Oy (on x=0 plane). 
The move() function will move the camera in the viewing direction, 
therefore it takes into consideration the rotation values.
The place() function will actually apply the translations and rotations
on the modelview matrix to achieve the effect.
*/
class Camera {
	private:
		float mvX,mvY,mvZ;
		float rotX,rotY;
	public:
		Camera(float,float,float);
		void move(float, float, float);
		void place();
		void rotate(float, float);
		
		float getX() {return -mvX;};
		float getY() {return -mvY;};
		float getZ() {return -mvZ;};
};

#endif
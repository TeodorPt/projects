#ifndef _BUBBLES_H_
#define _BUBBLES_H_
#define NUM_BUBBLES 200

class Bubbles {
	private:		
		float bubbleX[NUM_BUBBLES];
		float bubbleZ[NUM_BUBBLES];
		float bubbleY[NUM_BUBBLES];
		float bubbleSpeed[NUM_BUBBLES];

	private:
		void genAllBubbles();

	public:
		Bubbles();
		void drawBubbles();
		void moveBubbles();
};

#endif
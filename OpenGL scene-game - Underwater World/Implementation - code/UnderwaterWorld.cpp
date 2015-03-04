/*******************************************************************
* UnderwaterWorld.cpp : Defines the entry point for the application.
* All classes are used here to create the scene. The GLUT specific
* functions, OpenGL initiation and scene initiation are done here.
* Scene rendering is also done here, although each object has a
* function by which it renders itself.
********************************************************************/

#include "stdafx.h"
#include "glut.h"
#include "glui.h"
#include <gl/gl.h>
#include <ctime>
#include "tga.h"
#include "texload.h"
#include "Camera.h"
#include "Bubbles.h"
#include "ResLoader.h"
#include "Object.h"
#include "Fish.h"
#include "ProjectedShadowsBuilder.h"
#include "VolumetricShadowsBuilder.h"

#define NUM_CAUSTIC_PATTERNS 32		//number of caustic patterns used for caustics generation
#define NUM_BUBBLES 200				//number of bubbles rendered in the scene
#define NUM_SOIL_OBJECTS 10			//number of objects on the bottom of the scene
#define NUM_FISHES 15				//number of fishes in the scene
 
//Control variables for application menu
int enableCaustics = 1;
int enableBubbles = 1;
int enableProjected = 1;
int enableVolumetric = 0;
int enableWireframe = 0;
int menuAccessing = 0;
char wireframe = 0;
//End of control variables

static GLuint ceilTex;

//Define the variables of control for caustics generation
static GLint currentCaustic = 0;
static GLint causticIncrement = 1;
static GLfloat causticScale = 0.25;
static GLfloat causticSpeed = 1;

static int botPosition = 0;

//Global light, directional
GLfloat mylight_ambient[] = {0.1, 0.1, 0.2, 0.1};
GLfloat mylight_diffuse[] = {0.5, 0.7, 0.7, 1};
GLfloat mylight_specular[] = {1, 1, 1, 1};
GLfloat mylight_position[] = {20, 100, 0, 0};

//Red spotlight
GLfloat mylight2_ambient[] = {1, 0, 0, 0.1};
GLfloat mylight2_diffuse[] = {1, 0, 0, 1};
GLfloat mylight2_specular[] = {1, 0, 0, 1};
GLfloat mylight2_position[] = {55, 1, 0, 1};
GLfloat spot_direction[] = {-100,200,-30};
//Light amount of the spotlight
GLfloat light_amount = 1;

GLfloat ceil_ambient[] = {1, 1, 1, 1};
GLfloat ceil_diffuse[] = {1, 1, 1, 1};
GLfloat ceil_specular[] = {1,1,1,1};

GLint screen_width=1380;
GLint screen_height=764;

//Variables for mouse position, used for camera movement
GLint mouseX = screen_width / 2;
GLint mouseY = screen_height / 2;
bool isWarpingPointer = false;

//Collision variables
bool collision = false;
bool collisionLeft = false;
bool collisionRight = false;
GLfloat depth_data[2000000];

//Color of fog
GLfloat fogColor[] = {0.0f, 0.775f, 0.775f, 1.0f};

Camera camera(0,10,20);
Bubbles bubbles;
Object *grass;
Object *ship;
Object *lantern;
Object *rock;
Object *starfish;
Object *chest;
Object *soil;
Object *skydome;
GLfloat positionX[20],positionY[20];
Fish *fish[20];
Fish *whale;
ProjectedShadowsBuilder *proj_shadows;
VolumetricShadowsBuilder *vol_shadows;
Object *boat;
 

//place the global directional light
void placeGlobalLight() {
	glLightfv(GL_LIGHT0, GL_AMBIENT, mylight_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, mylight_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, mylight_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, mylight_position);
}

//place the red spotlight
void placeSpotLight() {
	glLightfv(GL_LIGHT1, GL_AMBIENT, mylight2_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, mylight2_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, mylight2_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, mylight2_position);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.1);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
}

//place the spotlight bulb, at the tip of the lantern
void placeSpotLightBulb() {
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mylight2_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mylight2_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mylight2_diffuse);
	glTranslatef(54.5,0.9,0);
	glutSolidSphere(0.4,6,6);
	glPopMatrix();
}

//place the floor; enable the stencil buffer if we are
//drawing projected shadows
void placeFloor(int pass) {
	
	if (pass==PASS_NORMAL && enableProjected) {
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0);
		glClear(GL_STENCIL_BUFFER_BIT);
	} 
	
	soil->draw(pass);
}

//place the ceil (top of the water)
void placeCeil(int pass) {
	if (pass==PASS_NORMAL) {
		//glDisable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, ceilTex);
	}
	glEnable(GL_FOG);

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,ceil_ambient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,ceil_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,ceil_specular);

	for (int i=-5;i<5;i++) {
		for (int j=-5;j<5;j++) {
		    glBegin(GL_QUADS);
				glTexCoord2f(0,0); glVertex3f(i*100-50,50,j*100-50);
				glTexCoord2f(3,0); glVertex3f(i*100+50,50,j*100-50);
				glTexCoord2f(3,3); glVertex3f(i*100+50,50,j*100+50);
				glTexCoord2f(0,3); glVertex3f(i*100-50,50,j*100+50);
			glEnd();
		}
	}

	if (pass==PASS_NORMAL) {
		//glDisable(GL_BLEND);
		//glEnable(GL_TEXTURE_2D);
	}
}

/*
Load caustic textures and the texture for the ceil.
All other textures are directly loaded by each object
upon construction
*/
void loadTextures() {
	glGenTextures(1,&ceilTex);
	loadTexture("res/textures/ceil.tga",&ceilTex);
	
	GLubyte *imageData;
	int width, height;

	for (int i=0;i<NUM_CAUSTIC_PATTERNS;i+=causticIncrement) {
		char filename[80];

		sprintf(filename,"res/caustics/caust%02d.bw",i);

		imageData = read_alpha_texture(filename,&width,&height);
		if (imageData == NULL) {
			fprintf(stderr, "\n%s: could not load image file\n", filename);
			exit(1);
		}
		glBindTexture(GL_TEXTURE_2D,i+100);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, imageData);

		free(imageData);
	}
}

/*
Load all models in the scene.
*/
void loadModels() {
	skydome = new Object("res/obj/skydome.obj","res/textures/skydome.tga",300);

	ship = new Object("res/obj/old_ship.obj","res/textures/old_ship.tga",50);
	ship->setPosition(0,13,0);
	ship->setRotation(15,0,-15);

	lantern = new Object("res/obj/lantern.obj","res/textures/lantern.tga",1);
	lantern->setPosition(55,0,0);
	lantern->setRotation(0,180,60);

	boat = new Object("res/obj/boat.obj","res/textures/old_ship.tga",10);
	boat->setPosition(0,50,0);

	chest = new Object("res/obj/chest.obj","res/textures/chest.tga",2);
	chest->setPosition(15,1.5,45);
	chest->setRotation(0,60,0);

	grass = new Object("res/obj/grass.obj","res/textures/grass.tga",10);
	rock = new Object("res/obj/rock.obj","res/textures/rock.tga",2);
	starfish = new Object("res/obj/starfish.obj","res/textures/starfish.tga",2);
	starfish->setPosition(0,-1,0);

	soil = new Object("res/obj/soil.obj","res/textures/soil.tga",300);

	for (int i=0;i<NUM_SOIL_OBJECTS;i++) {
		 positionX[i] = (rand()%200-100);
		//if (rand()%2==1) positionX[i]=-positionX[i];
		positionY[i] = (rand()%100-50);
		if (abs(positionX[i])<25) positionY[i]+=(positionY[i]<0?-10:10);
		//if (rand()%2==1) positionY[i]=-positionY[i];
	}

	fish[0] = new Fish("res/obj/redcrown.obj","res/textures/redcrown.tga",1,1);
	fish[0]->setPosition(10,15,10);
	fish[1] = new Fish("res/obj/ryatu.obj","res/textures/ryatu.tga",1,1);
	fish[1]->setPosition(60,30,40);
	fish[2] = new Fish("res/obj/shark.obj","res/textures/shark.tga",10,0.5);
	fish[2]->setPosition(80,15,40);
	fish[3] = new Fish("res/obj/neon.obj","res/textures/neon.tga",1,1.5);
	fish[3]->setPosition(100,15,30);
	fish[4] = new Fish("res/obj/tuna.obj","res/textures/tuna.tga",4,0.7);
	fish[4]->setPosition(20,15,50);
	fish[5] = new Fish("res/obj/redcrown.obj","res/textures/redcrown.tga",1,1);
	fish[5]->setPosition(10,15,10);
	fish[6] = new Fish("res/obj/ryatu.obj","res/textures/ryatu.tga",1,1);
	fish[6]->setPosition(60,30,40);
	fish[7] = new Fish("res/obj/shark.obj","res/textures/shark.tga",10,0.5);
	fish[7]->setPosition(-50,20,40);
	fish[8] = new Fish("res/obj/neon.obj","res/textures/neon.tga",1,1.5);
	fish[8]->setPosition(100,15,30);
	fish[9] = new Fish("res/obj/tuna.obj","res/textures/tuna.tga",4,0.7);
	fish[9]->setPosition(20,15,50);
	fish[10] = new Fish("res/obj/spade.obj","res/textures/spade.tga",4,0.7);
	fish[10]->setPosition(40,20,10);
	fish[11] = new Fish("res/obj/spade.obj","res/textures/spade.tga",4,0.7);
	fish[11]->setPosition(40,20,20);
	for (int i=12;i<15;i++) {
		fish[i] = new Fish("res/obj/redcrown.obj","res/textures/redcrown.tga",1,1);
		fish[i]->setPosition(10+i*2,20,10-i*2);
	}
	whale = new Fish("res/obj/whale.obj","res/textures/whale.tga",35,0.5);
}

/*
Function called everytime the mouse is moved.
It will calculate the mouse displacement, take it into
the consideration for rotation, then it will
warp the mouse back to the center of the screen.
*/
void mouseMoved(int x, int y) {
	if (!menuAccessing) {
		if(!isWarpingPointer) //do not record the cursor movement if the cursor is "warping" to the center
		{
			float dispX = (x - screen_width/2)/50.; //rotate relative to Oy by this value
			float dispY = (y - screen_height/2)/100.; //rotate relativ to Ox by this value
 
			camera.rotate(dispX,dispY); //call the camera rotation function
		
			isWarpingPointer = true; //warp the cursor to the center of the screen
			glutWarpPointer(screen_width/2, screen_height/2);
		}
		else
			isWarpingPointer = false;

		glutPostRedisplay();
	}
}

/*
Draws the scene of objects. Takes an integer argument,
which is the pass of the rendering. If there are no
caustics, then PASS_NORMAL should be given as argument.
If there are also caustics, the scene should be rendered
twice, once with PASS_NORMAL and once with PASS_CAUSTIC.
*/
void drawScene(int pass)
{
	if (pass == PASS_CAUSTIC) {
		GLfloat sPlane[4] = { 0.05, 0.03, 0.0, 0.0 };
		GLfloat tPlane[4] = { 0.0, 0.03, 0.08, 0.0 };
	
		sPlane[0] = 0.05 * causticScale;
		sPlane[1] = 0.03 * causticScale;

		tPlane[1] = 0.03 * causticScale;
		tPlane[2] = 0.05 * causticScale;

		glColor3f(1.0, 1.0, 1.0);
		glDisable(GL_LIGHTING);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
		glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		
		glBindTexture(GL_TEXTURE_2D,currentCaustic+100);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	placeFloor(pass);
	
	if (pass == PASS_NORMAL && enableProjected) {
		
		proj_shadows->placeShadow(ship);
		for (int i=0;i<NUM_FISHES;i++) {
			proj_shadows->placeShadow(fish[i]);
		}
		for (int i=0;i<NUM_SOIL_OBJECTS;i++) {
			grass->setPosition(positionX[i],1.2,positionY[i]);
			proj_shadows->placeShadow(grass);
			rock->setPosition(-positionX[i],1,-positionY[i]);
			proj_shadows->placeShadow(rock);
			starfish->setPosition(positionX[i],0.3,-positionY[i]);
			proj_shadows->placeShadow(starfish);
			proj_shadows->placeShadow(whale);
		}
		proj_shadows->placeShadow(chest);
	}
	
	//THIS IS GOOOD
	for (int i=0;i<NUM_SOIL_OBJECTS;i++) {
		grass->setPosition(positionX[i],1.2,positionY[i]);
		grass->draw(pass);
		rock->setPosition(-positionX[i],1,-positionY[i]);
		rock->draw(pass);
		starfish->setPosition(positionX[i],0.3,-positionY[i]);
		starfish->draw(pass);
	}
	
	chest->draw(pass);

	//Place the fishes
	for (int i=0;i<NUM_FISHES;i++) fish[i]->draw(pass);
	
	whale->draw(pass);
	ship->draw(pass);
	boat->draw(pass);
	lantern->draw(pass);
	
	placeSpotLightBulb();

	glPushMatrix();
	glTranslatef(camera.getX(),camera.getY(),camera.getZ());
	glutSolidCube(1000);


	glPopMatrix();

	//Collision checking using the depth buffer
	if (pass == PASS_NORMAL) {
		glReadPixels((screen_width-800)/2,(screen_height-600)/2,800,600,GL_DEPTH_COMPONENT,GL_FLOAT,depth_data);

		collision = false;
		collisionLeft = false;
		collisionRight = false;
		for (int i=0;i<800*600;i++) {
			if (depth_data[i]<0.5) collision=true;
			if (i%screen_width<screen_width/3 && depth_data[i]<0.5) collisionLeft=true;
			if (i%screen_width>screen_width*2/3 && depth_data[i]<0.5) collisionRight=true;
		}
	}

	//Place the ceil
	placeCeil(pass);

	if (pass == PASS_CAUSTIC) {
		glEnable(GL_LIGHTING);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

	}
}

void renderScene(void)
{
	if (enableWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	gluLookAt(0, 0, 0, 0, 0, -1, 0.0, 1.0, 0.0);
	
	//"Place" the camera. Means that the rotations and translations are
	//actually added to the modelview matrix.
	camera.place();

	placeGlobalLight();
	placeSpotLight();
	
	
	//Draw the sky dome
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	skydome->setPosition(camera.getX(),200,camera.getZ());
	skydome->draw(PASS_NORMAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_FOG);

	if (!enableWireframe) {
		//If projected shadows are enabled, or volumetric shadows are disabled
		//We normally draw the scene
		if (enableProjected || !enableVolumetric) drawScene(PASS_NORMAL);
		else {
			if (enableVolumetric==2) {							//If shadow volume is enabled (enableVolumetric==2), we normally draw the scene + shadow volume
				drawScene(PASS_NORMAL);
				vol_shadows->drawShadowVolume();
			}
			else vol_shadows->drawSceneWithShadow(drawScene);	//If volumetric shadows are enabled, we let VolumetricShadowBuilder to draw the scene
		}
	
		//Draw the scene with caustics, if they are enabled
		if (enableCaustics == 1) {
			glDepthMask(GL_FALSE);
			glDepthFunc(GL_EQUAL);
			glBlendFunc(GL_ZERO,GL_SRC_COLOR);
			glEnable(GL_BLEND);
	
			drawScene(PASS_CAUSTIC);

			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glDisable(GL_BLEND);
	
		}
	} else drawScene(PASS_NORMAL);
	
	//Draw any transparent object here
	if (enableBubbles == true) {
		bubbles.drawBubbles();
	}
	
	glutSwapBuffers();
}

void changeSize(int w, int h)
{
	screen_width=w;
	screen_height=h;

	if(h == 0)
		h = 1;

	float ratio = 1.0*w/h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 50.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
	
	GLUI_Master.reshape();
}

/*
Listen for key pressings and execute actions accordingly
(e.g. mouse escape, camera movement).
*/
void processNormalKeys(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 's': //"move back"
			camera.move(0,0,-0.5);
			glutPostRedisplay();
			break;
		case 'w': //"move fwd"
			if(collision==false) {
				camera.move(0,0,0.5);
				glutPostRedisplay();
			}
			break;
		case 'a'://move left
			if (collisionLeft==false)
			camera.move(0.5,0,0);
			glutPostRedisplay();
			break;
		case 'd'://move right
			if (collisionRight==false)
			camera.move(-0.5,0,0);
			glutPostRedisplay();
			break;
		case 'q'://exit
			exit(0);
			break;
		case 'm':
			menuAccessing = 1-menuAccessing;
			if (menuAccessing)
				glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			else
				glutSetCursor(GLUT_CURSOR_NONE);
			break;
	}
	

}

/*
Idle processings are done here, such as caustics animation,
bubbles movement upwards and fishes animation.
*/
void idleProcessing() {
	static int index;
	index++;
	if (index>1/causticSpeed) {
		currentCaustic = (currentCaustic + causticIncrement)%NUM_CAUSTIC_PATTERNS;
		index = 0;
		botPosition++;
	}

	bubbles.moveBubbles();
	
	//animateFish();

	for (int i=0;i<NUM_FISHES;i++) fish[i]->moveFish();
	whale->moveCircular(150);

	light_amount-=0.1;
	if (light_amount<-2) light_amount = 2;

	GLfloat newLightAmount = abs(light_amount);
	if (newLightAmount>1) newLightAmount = 1;
	mylight2_ambient[0] = newLightAmount;
	mylight2_diffuse[0] = newLightAmount;
	mylight2_specular[0] = newLightAmount;

	glutPostRedisplay();
}

void initOpenGL()
{

	glClearColor(0.0f, 0.6f, 0.6f, 1.0f);
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, screen_width, screen_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 1.0f, 1000.0f);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_NORMALIZE);
	//CODE FOR ENABLING LIGHTS
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	//END OF LIGHTS CODE

	//CODE FOR FOG
	glFogi(GL_FOG_MODE, GL_EXP2);        // Fog Mode
	glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
	glFogf(GL_FOG_DENSITY, 0.01f);              // How Dense Will The Fog Be
	glHint(GL_FOG_HINT, GL_NONE);          // Fog Hint Value
	glEnable(GL_FOG);     
	
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	//END OF CODE FOR FOG

	loadTextures();
	glEnable(GL_TEXTURE_2D);

	loadModels();

	GLfloat floorEq[] = {0,1,0,0};
	proj_shadows = new ProjectedShadowsBuilder(floorEq,mylight_position);
	vol_shadows = new VolumetricShadowsBuilder();

	vol_shadows->makeShadowVolume(boat,mylight_position);
}

int main(int argc, char* argv[])
{ 
	srand(time(NULL));
	int main_window;

	//Initialize the GLUT library
	glutInit(&argc, argv);
	//Set the display mode
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	//Set the initial position and dimensions of the window
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(screen_width, screen_height);
	//creates the window
	main_window = glutCreateWindow("Underwater World");
	//Specifies the function to call when the window needs to be redisplayed
	glutDisplayFunc(renderScene);
	//Sets the idle callback function
	glutIdleFunc(idleProcessing);
	//Sets the reshape callback function
	glutReshapeFunc(changeSize);
	//Keyboard callback function
	glutKeyboardFunc(processNormalKeys);
	//Initialize some OpenGL parameters
	initOpenGL();
	
	glutPassiveMotionFunc(&mouseMoved);
	glutSetCursor(GLUT_CURSOR_NONE);
	

	//UI CODE HERE
	GLUI *glui =
	GLUI_Master.create_glui_subwindow(
		main_window,
		GLUI_SUBWINDOW_RIGHT ); 

	GLUI_RadioGroup *radio;
	GLUI_Panel *type_panel = glui->add_panel("Caustics");
	
	radio = glui->add_radiogroup_to_panel(type_panel, &enableCaustics, 200);
	glui->add_radiobutton_to_group(radio, "No caustics");
	glui->add_radiobutton_to_group(radio, "With caustics");

	GLUI_Panel *type_panel2 = glui->add_panel("Bubbles");
	
	radio = glui->add_radiogroup_to_panel(type_panel2, &enableBubbles, 201);
	glui->add_radiobutton_to_group(radio, "No bubbles");
	glui->add_radiobutton_to_group(radio, "With bubles");


	GLUI_Panel *type_panel3 = glui->add_panel("Projected Shadows");

	radio = glui->add_radiogroup_to_panel(type_panel3, &enableProjected, 202);
	glui->add_radiobutton_to_group(radio, "No projected shadows");
	glui->add_radiobutton_to_group(radio, "With projected shadows");

	
	GLUI_Panel *type_panel4 = glui->add_panel("Volumetric Shadows");

	radio = glui->add_radiogroup_to_panel(type_panel4, &enableVolumetric, 203);
	glui->add_radiobutton_to_group(radio, "No volumetric shadows");
	glui->add_radiobutton_to_group(radio, "With volumetric shadows");
	glui->add_radiobutton_to_group(radio, "Show shadow volume");

	GLUI_Panel *type_panel5 = glui->add_panel("Volumetric Shadows");

	radio = glui->add_radiogroup_to_panel(type_panel5, &enableWireframe, 204);
	glui->add_radiobutton_to_group(radio, "Normal rendering");
	glui->add_radiobutton_to_group(radio, "Wireframe rendering");
	
	glui->add_separator_to_panel(type_panel);
	glui->add_separator_to_panel(type_panel2);
	glui->add_separator_to_panel(type_panel3);
	glui->add_separator_to_panel(type_panel4);
	glui->add_separator_to_panel(type_panel5);
	glui->set_main_gfx_window(main_window);

	//END OF UI CODE

	//Starts the GLUT infinite loop
	glutMainLoop();
	return 0;
}


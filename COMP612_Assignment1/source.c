/******************************************************************************
 *
 * Animation v1.0 (23/02/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene.
 *
 * Monitosh Thaker | 17000777
 * COMP612 2023 SEM 1
 * 
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

  // Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60				

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Define all character keys used for input (add any new key definitions here).
 // Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
 // characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_EXIT			27 // Escape key.
#define KEY_Q				113 // Q key.
#define KEY_S				115 // S key.

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void idle(void);
void mouseButton(int button, int state, int x, int y);
void mousePosition(int x, int y);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);

void createParticle(void);
void spawnParticles(void);
void testParticles(int i);
void drawParticles(void);
void updateSnow(int i);

void drawScene(void);
void drawBackground(void);
void initialiseGround(void);
void drawGround(void);
void drawParticles(void);

void updateSnow(int i);

void drawSnowmanHead(GLfloat _xPos, GLfloat _yPos, GLfloat radius);
void drawSnowmanBody(GLfloat xPos, GLfloat yPos, GLfloat radius);

void drawButton(void);
int checkCircleArea(float x, float y, float radius);

void drawText(void);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

#define PI 3.1459
#define MAX_PARTICLES 1000

typedef struct {
	float x;
	float y;
}Position2D;

typedef struct {
	Position2D position;
	float size;		//GL point size
	float alpha;	//alpha value
	float dx;		//x velocity
	float dy;		//y velocity
	int active;		//is inactive
}Particle_t;

Particle_t particle;
Particle_t particle_system[MAX_PARTICLES];
int numParticles;

GLfloat snowmanX;
GLfloat snowmanY;

GLint windowWidth = 800;
GLint windowHeight = 800;

typedef struct
{
	float x;
	float y;
} Point2D;

Point2D mouseLocation;

int mouseHold;
int mousePressed;
int startKeyPressed;

int headInRange;

int SimulationStarted;

float groundVertices[4][2];

 /******************************************************************************
  * Entry Point (don't put anything except the main function here)
  ******************************************************************************/

void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Animation");

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutMouseFunc(mouseButton);
	glutPassiveMotionFunc(mousePosition);
	glutIdleFunc(idle);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

 /*
	 Called when GLUT wants us to (re)draw the current animation frame.

	 Note: This function must not do anything to update the state of our simulated
	 world. Animation (moving or rotating things, responding to keyboard input,
	 etc.) should only be performed within the think() function provided below.
 */
void display(void)
{
	/*
		TEMPLATE: REPLACE THIS COMMENT WITH YOUR DRAWING CODE

		Separate reusable pieces of drawing code into functions, which you can add
		to the "Animation-Specific Functions" section below.

		Remember to add prototypes for any new functions to the "Animation-Specific
		Function Prototypes" section near the top of this template.
	*/
	glClear(GL_COLOR_BUFFER_BIT);

	drawScene();
	drawParticles();

	//drawButton();
	drawText();

	glutSwapBuffers();
}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
		/*
			TEMPLATE: Add any new character key controls here.

			Rather than using literals (e.g. "d" for diagnostics), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
		*/
	case KEY_S:
		SimulationStarted = (SimulationStarted == 0);
		break;
	case KEY_Q:
		initialiseGround();
		break;
	case KEY_EXIT:
		exit(0);
		break;
	}
}

/*
	Called by GLUT when it's not rendering a frame.

	Note: We use this to handle animation and timing. You shouldn't need to modify
	this callback at all. Instead, place your animation logic (e.g. moving or rotating
	things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

 /*
	 Initialise OpenGL and set up our scene before we begin the render loop.
 */
void init(void)
{
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluOrtho2D(-1.0, 1.0, -1, 1.0);

	srand((unsigned)time(0));

	initialiseGround();

	snowmanX = -0.4f;
	snowmanY = -0.2f;

	SimulationStarted = 0;
	particle.active = 0;
	numParticles = 0;
	startKeyPressed = 0;
	mouseHold = 0;
}

/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
*/
void think(void)
{
	/*
		TEMPLATE: REPLACE THIS COMMENT WITH YOUR ANIMATION/SIMULATION CODE

		In this function, we update all the variables that control the animated
		parts of our simulated world. For example: if you have a moving box, this is
		where you update its coordinates to make it move. If you have something that
		spins around, here's where you update its angle.

		NOTHING CAN BE DRAWN IN HERE: you can only update the variables that control
		how everything will be drawn later in display().

		How much do we move or rotate things? Because we use a fixed frame rate, we
		assume there's always FRAME_TIME milliseconds between drawing each frame. So,
		every time think() is called, we need to work out how far things should have
		moved, rotated, or otherwise changed in that period of time.

		Movement example:
		* Let's assume a distance of 1.0 GL units is 1 metre.
		* Let's assume we want something to move 2 metres per second on the x axis
		* Each frame, we'd need to update its position like this:
			x += 2 * (FRAME_TIME / 1000.0f)
		* Note that we have to convert FRAME_TIME to seconds. We can skip this by
		  using a constant defined earlier in this template:
			x += 2 * FRAME_TIME_SEC;

		Rotation example:
		* Let's assume we want something to do one complete 360-degree rotation every
		  second (i.e. 60 Revolutions Per Minute, or RPM).
		* Each frame, we'd need to update our object's angle like this (we'll use the
		  FRAME_TIME_SEC constant as per the example above):
			a += 360 * FRAME_TIME_SEC;

		This works for any type of "per second" change: just multiply the amount you'd
		want to move in a full second by FRAME_TIME_SEC, and add or subtract that
		from whatever variable you're updating.

		You can use this same approach to animate other things like color, opacity,
		brightness of lights, etc.
	*/

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		updateSnow(i);		
	}

	if (SimulationStarted == 1)
	{
		if (numParticles < MAX_PARTICLES)
		{
			spawnParticles();
		}
	}
}

float buttonArea;

GLfloat buttonPosX = 0.85f;
GLfloat buttonPosY = 0.85f;
GLfloat buttonRadius = 0.1f;

int buttonPressed = 0;

void drawButton()
{
	if (buttonPressed)
	{
		glColor3f(0.8f, 0.8f, 0.3f);
	}
	else
	{
		glColor3f(0.7f, 0.5f, 0.3f);
	}

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(buttonPosX, buttonPosY);

	for (int i = 0; i <= 360; i += 5)
	{
		double angle = i * PI / 180.0f;
		double x = cos(angle) * buttonRadius;
		double y = sin(angle) * buttonRadius;

		glVertex2f(buttonPosX + (GLfloat)x, buttonPosY + (GLfloat)y);
	}

	glEnd();

	buttonArea = PI * (buttonRadius * buttonRadius);
}

void mouseButton(int button, int state, int x, int y)
{
	/*	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mousePressed = 1;
		mouseY = (GLfloat)windowHeight - (GLfloat)y;
		mouseY = (mouseY / (GLfloat)windowHeight) * 2.0f - 1.0f;
		glutPostRedisplay();
	}
	*/

	if (button == GLUT_LEFT_BUTTON)
	{
		mouseHold = (state == GLUT_DOWN);
		glutMotionFunc(mousePosition);
	}
}

void mousePosition(int x, int y)
{
	mouseLocation.x = (GLfloat)windowWidth - (GLfloat)x;
	mouseLocation.x = (mouseLocation.x / (GLfloat)windowWidth) * -2.0f + 1.0f;

	mouseLocation.y = (GLfloat)windowHeight - (GLfloat)y;
	mouseLocation.y = (mouseLocation.y / (GLfloat)windowHeight) * 2.0f - 1.0f;
}

int checkCircleArea(float x, float y, float radius)
{
	float distance = sqrt(pow((mouseLocation.x - x), 2) + pow((mouseLocation.y - y), 2));

	return (distance < radius);
}

void createParticle()
{
	particle.size = (((float)rand() / (float)RAND_MAX) * 3.0f) + 3.0f; //random size between 5 and 10

	particle.alpha = (((float)rand() / (float)RAND_MAX) * 0.6f) + 0.4f; //random alpha value between 0.2 and 1.0

	particle.position.x = (((float)rand() / (float)RAND_MAX) * 3.0f) - 1.5f; //random x position between -1.5 and 1.5
	particle.position.y = (((float)rand() / (float)RAND_MAX) * 1.5f) + 1.0f; //random y position between 1.0 and 2.5

	particle.dy = 0.001f * particle.size;
	particle.dx = 0.0001f * particle.size;
}

void spawnParticles()
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		if (particle_system[i].active == 0)
		{
			createParticle();
			particle_system[i] = particle;
			particle_system[i].active = 1;
			numParticles++;
			break;
		}
	}	
}

void testParticles(int i)
{
	createParticle();
	particle_system[i] = particle;
	particle_system[i].active = 1;
	numParticles++;
}


void updateSnow(int i)
{
	if (particle_system[i].active == 1)
	{
		if (particle_system[i].position.y >= -1.0f)
		{
			particle_system[i].position.y -= particle_system[i].dy;
			particle_system[i].position.x -= particle_system[i].dx;
		}
		else
		{
			particle_system[i].active = 0;
			numParticles--;

			if (SimulationStarted)
			{
				spawnParticles();
			}
		}
	}
	
}

void drawScene()
{
	drawBackground();
	drawGround();

	//body
	drawSnowmanBody(snowmanX, snowmanY - 0.350f, 0.20f);
	drawSnowmanBody(snowmanX, snowmanY - 0.175f, 0.15f);

	//shirt buttons
	glPointSize(12.0f);
	glBegin(GL_POINTS);
	glColor3f(0.4f, 0.2f, 0.1f);
	glVertex2f(snowmanX, snowmanY - 0.14f);
	glVertex2f(snowmanX, snowmanY - 0.200f);
	glVertex2f(snowmanX, snowmanY - 0.260f);
	glEnd();

	//stick arms
	glLineWidth(10.0f);
	glBegin(GL_LINES);
	glColor3f(0.2f, 0.1f, 0.0f);
	glVertex2f(snowmanX - 0.12f, snowmanY - 0.125f);
	glVertex2f(snowmanX - 0.30f, snowmanY - 0.05f);
	glVertex2f(snowmanX + 0.12f, snowmanY - 0.125f);
	glVertex2f(snowmanX + 0.30f, snowmanY - 0.05f);
	glEnd();

	//head
	drawSnowmanHead(snowmanX, snowmanY, 0.10f);
}

void drawBackground()
{
	glBegin(GL_QUADS);

	glColor3f(0.0f, 0.0f, 0.2f);
	glVertex2f(-1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glColor3f(0.1f, 0.35f, 0.7f);
	glVertex2f(1.0f, -1.0f);
	glVertex2f(-1.0f, -1.0f);

	glEnd();
}

float groundVertX, groundVertY;
float groundVertX2, groundVertY2;

void drawGround()
{
	float groundVertices[4][2] =
	{
		{-1.0f, -1.0f},             {1.0f, -1.0f},
		{groundVertX, groundVertY}, {groundVertX2, groundVertY2}
	};

	glBegin(GL_POLYGON);
	glColor3f(0.4f, 0.5f, 0.8f);
	glVertex2f(groundVertices[0][0], groundVertices[0][1]);
	glVertex2f(groundVertices[1][0], groundVertices[1][1]);
	glColor3f(0.80f, 0.85f, 0.95f);
	glVertex2f(groundVertices[2][0], groundVertices[2][1]);
	glVertex2f(groundVertices[3][0], groundVertices[3][1]);
	glEnd();
}

void initialiseGround()
{
	groundVertX = (((float)rand() / (float)RAND_MAX) * 0.3f) + 0.6f;
	groundVertY = (((float)rand() / (float)RAND_MAX) * 0.3f) - 0.6f;

	groundVertX2 = (((float)rand() / (float)RAND_MAX) * 0.3f) - 0.9f;
	groundVertY2 = (((float)rand() / (float)RAND_MAX) * 0.3f) - 0.6f;
}

void drawParticles()
{
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		glPointSize(particle_system[i].size);

		glColor4f(1.0f, 1.0f, 1.0f, particle_system[i].alpha);

		glBegin(GL_POINTS);
		glVertex2f(particle_system[i].position.x, particle_system[i].position.y);
		glEnd();
	}
}

void drawText()
{
	char mouseXText[50];
	sprintf(mouseXText, "Mouse X: %.4f", mouseLocation.x);

	char mouseYText[50];
	sprintf(mouseYText, "Mouse Y: %.4f", mouseLocation.y);

	char particleText[50];
	sprintf(particleText, "Particles: %d", numParticles);

	char snowText[50];
	sprintf(snowText, "Press [S] to toggle snow.");

	char terrainText[50];
	sprintf(terrainText, "Press [Q] to change terrain.");

	char headText[50];
	sprintf(headText, "Click and drag the snowman's head to move it.");

	glColor3f(1.0f, 0.6f, 0.0f);
	glRasterPos2f(-0.98f, 0.95f);
	for (int i = 0; i < strlen(mouseXText); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, mouseXText[i]);
	}

	glColor3f(1.0f, 0.6f, 0.0f);
	glRasterPos2f(-0.98f, 0.90f);
	for (int i = 0; i < strlen(mouseYText); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, mouseYText[i]);
	}

	glColor3f(1.0f, 0.6f, 0.0f);
	glRasterPos2f(-0.98f, 0.85f);
	for (int i = 0; i < strlen(particleText); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, particleText[i]);
	}

	glColor3f(1.0f, 0.6f, 0.0f);
	glRasterPos2f(-0.5f, 0.95f);
	for (int i = 0; i < strlen(snowText); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, snowText[i]);
	}

	glColor3f(1.0f, 0.6f, 0.0f);
	glRasterPos2f(-0.5f, 0.90f);
	for (int i = 0; i < strlen(terrainText); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, terrainText[i]);
	}

	glColor3f(1.0f, 0.6f, 0.0f);
	glRasterPos2f(-0.5f, 0.85f);
	for (int i = 0; i < strlen(headText); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, headText[i]);
	}
}

void drawSnowmanHead(GLfloat _xPos, GLfloat _yPos, GLfloat radius)
{
	float xPos;
	float yPos;

	if(mouseHold && headInRange)
	{
		xPos = mouseLocation.x;
		yPos = mouseLocation.y;
	}
	else
	{
		xPos = _xPos;
		yPos = _yPos;
	}

	headInRange = (mouseHold && checkCircleArea(xPos, yPos, radius));

	//main snowball
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(xPos, yPos);

	for (int i = 0; i <= 360; i += 5)
	{
		double angle = i * PI / 180.0f;
		double x = cos(angle) * radius;
		double y = sin(angle) * radius;

		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex2f(xPos + (GLfloat)x, yPos + (GLfloat)y);
	}
	glEnd();

	//eyes
	glPointSize(9.0f);
	glBegin(GL_POINTS);
	glColor3f(0, 0, 0);
	glVertex2f(xPos + 0.04f, yPos + 0.02f);
	glVertex2f(xPos - 0.04f, yPos + 0.02f);
	glEnd();

	//carrot nose base
	glPointSize(20.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0, 0.5, 0.0);
	glVertex2f(xPos + 0.01f, yPos - 0.02f);
	glEnd();

	//carrot nose
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.5, 0.0);
	glVertex2f(xPos + 0.01f, yPos);
	glVertex2f(xPos + 0.085f, yPos - 0.02f);
	glVertex2f(xPos + 0.01f, yPos - 0.04f);
	glEnd();
}

void drawSnowmanBody(GLfloat xPos, GLfloat yPos, GLfloat radius)
{
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(xPos, yPos);

	for(int i = 0; i <= 360; i+= 5)
	{
		double angle = i * PI / 180.0f;
		double x = cos(angle) * radius;
		double y = sin(angle) * radius;

		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex2f(xPos + (GLfloat)x, yPos + (GLfloat)y);
	}

	glEnd();
}

/******************************************************************************/
/******************************************************************************
*
* Official Name:  Farhana Mustafa
*
* E-mail:  fmustafa@syr.edu
*
* Assignment:  Homework 4
*
* Environment/Compiler:  Visual Studio 2015
*
* Date:  April 3rd, 2018
*
* References:  none
*
* Interactions:

******************************************************************************/

#include <iostream>
#include <cmath>
#include <fstream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159
using namespace std;

static unsigned int texture[2]; // Array of texture indices.
static unsigned char chessboard[64][64][3]; // Storage for chessboard image.

static GLUquadricObj *qobj; // Create a pointer to a quadric object.

static bool overhead = false;

static float meX = 0, meY = 0, meZ = 0;
static float angle = 0;
static float stepsize = 5.0;  //step size
static float turnsize = 10.0; //degrees to turn
static float pointOfView = 0.0;
static float spotExponent = 2.0;

static int isSelecting = 0; // In selection mode?
static int hits; // Number of entries in hit buffer.
static unsigned int buffer[1024]; // Hit buffer.
static unsigned int closestName = 0; // Name of closest hit.

static long font = (long)GLUT_BITMAP_8_BY_13; //Fonts

// Struct of bitmap file.
struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char *data;
};

// Routine to read a bitmap file.
// Works only for uncompressed bmp files of 24-bit color.
BitMapFile *getBMPData(string filename)
{
	BitMapFile *bmp = new BitMapFile;
	unsigned int size, offset, headerSize;

	// Read input file name.
	ifstream infile(filename.c_str(), ios::binary);

	// Get the starting point of the image data.
	infile.seekg(10);
	infile.read((char *)&offset, 4);

	// Get the header size of the bitmap.
	infile.read((char *)&headerSize, 4);

	// Get width and height values in the bitmap header.
	infile.seekg(18);
	infile.read((char *)&bmp->sizeX, 4);
	infile.read((char *)&bmp->sizeY, 4);

	// Allocate buffer for the image.
	size = bmp->sizeX * bmp->sizeY * 24;
	bmp->data = new unsigned char[size];

	// Read bitmap data.
	infile.seekg(offset);
	infile.read((char *)bmp->data, size);

	// Reverse color from bgr to rgb.
	int temp;
	for (int i = 0; i < size; i += 3)
	{
		temp = bmp->data[i];
		bmp->data[i] = bmp->data[i + 2];
		bmp->data[i + 2] = temp;
	}

	return bmp;
}

// Load external textures.
void loadExternalTextures()
{
	// Local storage for bmp image data.
	BitMapFile *image[2];

	// Load the texture.
	image[0] = getBMPData("Textures/Road.bmp");
	image[1] = getBMPData("Textures/bmo.bmp");

	// Bind road image to texture index[0]
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);

	// Bind floor image to texture index[1]
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[1]->sizeX, image[1]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[1]->data);
}

// Routine to load a program-generated image as a texture. 
void loadProceduralTextures()
{
	// Activate texture index texture[1]. 
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	// Set texture parameters for wrapping.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture parameters for filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Specify an image as the texture to be bound with the currently active texture index.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, chessboard);
}

// Create 64 x 64 RGB image of a chessboard.
void createChessboard(void)
{
	int i, j;
	for (i = 0; i < 64; i++)
		for (j = 0; j < 64; j++)
			if ((((i / 8) % 2) && ((j / 8) % 2)) || (!((i / 8) % 2) && !((j / 8) % 2)))
			{
				chessboard[i][j][0] = 0x00;
				chessboard[i][j][1] = 0x00;
				chessboard[i][j][2] = 0x00;
			}
			else
			{
				chessboard[i][j][0] = 0xFF;
				chessboard[i][j][1] = 0xFF;
				chessboard[i][j][2] = 0xFF;
			}
}

// Routine to draw a bitmap character string.
void writeBitmapString(void *font, char *string)
{
	char *c;

	for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

// Initialization routine.
void setup(void)
{
	//glClearColor(0.196078, 0.6, 0.8, 0.0);
	glClearColor(0.22, 0.69, 0.87, 0.0);
	glEnable(GL_DEPTH_TEST); // Enable depth testing.

	glEnable(GL_LIGHTING);

	float lightAmb[] = { 0.0, 0.0, 0.0, 1.0 };
	float lightDifAndSpec0[] = { 1.0, 1.0, 1.0, 1.0 };
	//float lightPos[] = { 0.0, 1.0, 0.0, 0.0 };
	float globAmb[] = { 0.3, 0.3, 0.3, 1.0 };

	//Light0 properties
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec0);
	//glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_LIGHT0); // Enable particular light source.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb); // Global ambient light.

	// Create texture index array.
	glGenTextures(2, texture);

	// Load external texture and generate and load procedural texture.
	loadExternalTextures();
	createChessboard();
	loadProceduralTextures();

	// Specify how texture values combine with current surface color values.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Create the new quadric object.
	qobj = gluNewQuadric();
}

// Process hit buffer to find record with smallest min-z value.
void findClosestHit(int hits, unsigned int buffer[])
{
	unsigned int *ptr, minZ;

	minZ = 0xffffffff; // 2^32 - 1
	ptr = buffer;
	closestName = 0;
	for (int i = 0; i < hits; i++)
	{
		ptr++;
		if (*ptr < minZ)
		{
			minZ = *ptr;
			ptr += 2;
			closestName = *ptr;
			ptr++;
		}
		else ptr += 3;
	}
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120, 1, 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void addAllText() {
	glColor3f(1.0, 0.3, 0.7);

}

void drawRoof() {
	float matAmbAndDif1[] = { 1.0, 1.0, 1.0, 1.0 };

	glPushMatrix();
	glTranslatef(0.0, 20.0, 80.0);
	glScalef(45.0, 0.5, 29.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif1);
	glutSolidCube(2.0);
	glPushMatrix();
}

void drawSun() {
	float lightPos0[] = { 0.0, 0.0, 0.0, 1.0 };
	float matAmbAndDif1[] = { 1.0, 1.0, 0.0, 1.0 };

	glPushMatrix();
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	glTranslatef(70.0, 30.0, 80.0);
	//glTranslatef(20.0, 5.0, 80.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif1);
	glutSolidSphere(4.0, 20.0, 20.0);
	glPopMatrix();
}

void drawGround() {
	int i;
	float z;

	// Draw floor as a stack of triangle strips.
	i = 0;
	for (z = 200.0; z > 0.0; z -= 5.0)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (float x = -150.0; x < 150.0; x += 5.0)
		{
			if (i % 2) glColor4f(0.0, 0.5, 0.5, 1.0);
			else glColor4f(1.0, 1.0, 1.0, 1.0);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(x, -23.5, z - 5.0);
			glVertex3f(x, -23.5, z);
			i++;
		}
		glEnd();
		i++;
	}

	glFlush();
}

void drawFLoor() {
	// Turn on OpenGL texturing.
	glEnable(GL_TEXTURE_2D);

	// Activate a texture.
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	// Map the texture onto a square polygon.
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-50.0, -22.5, 50.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(50.0, -22.5, 50.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(50.0, -22.5, 110.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-50.0, -22.5, 110.0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawRoad() {
	// Turn on OpenGL texturing.
	glEnable(GL_TEXTURE_2D);

	// Activate a texture.
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Map the texture onto a square polygon.
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(-60.0, -22.5, 5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(60.0, -22.5, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(60.0, -22.5, 25.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-60.0, -22.5, 25.0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawWalls() {
	float matAmbAndDif1[] = { 0.20, 0.50, 1.00, 1.0 }; // blue
	float matAmbAndDif4[] = { 1.00, 0.30, 0.70, 1.0 }; // pink
	float matAmbAndDif5[] = { 1.00, 0.39, 0.39, 1.0 }; // orange
	float matAmbAndDif6[] = { 0.87, 0.58, 0.98, 1.0 }; // purple


	//wall; left of door
	glPushMatrix();
	glTranslatef(26.0, -3.0, 50.0);
	glScalef(21.0, 20.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif4);
	glutSolidCube(2.0);
	glPopMatrix();

	//wall; right of door
	glPushMatrix();
	glTranslatef(-26.0, -3.0, 50.0);
	glScalef(21.0, 20.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif4);
	glutSolidCube(2.0);
	glPopMatrix();

	//wall; above door
	glPushMatrix();
	glTranslatef(0.0, 13.0, 50.0);
	glScalef(5.0, 4.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif4);
	glutSolidCube(2.0);
	glPopMatrix();

	//back wall
	glPushMatrix();
	glTranslatef(0.0, -3.0, 110.0);
	glScalef(47.0, 20.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif1);
	glutSolidCube(2.0);
	glPopMatrix();

	//left wall; left of window
	glPushMatrix();
	glTranslatef(46.0, -3.0, 61.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(12.0, 20.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif5);
	glutSolidCube(2.0);
	glPopMatrix();

	//window
	glPushMatrix();
	glTranslatef(46.0, 2.5, 73.0);
	//glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(0.3, 0.3, 0.8);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif5);
	gluCylinder(qobj, 1, 1, 25, 15, 5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(46.0, 12.9, 80.0);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glScalef(0.3, 0.3, 0.8);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif5);
	gluCylinder(qobj, 1, 1, 25, 15, 5);
	glPopMatrix();

	glPushMatrix(); //wall above window
	glTranslatef(46.0, 15, 79.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(8.0, 2.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif5);
	glutSolidCube(2.0);
	glPopMatrix();

	glPushMatrix(); //wall below window
	glTranslatef(46.0, -14, 79.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(8.0, 9.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif5);
	glutSolidCube(2.0);
	glPopMatrix();

	//end window

	//left wall; right of window
	glPushMatrix();
	glTranslatef(46.0, -3.0, 99.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(12.0, 20.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif5);
	//glColor3f(0.65, 0.50, 0.39);
	glutSolidCube(2.0);
	glPopMatrix();


	//right wall
	glPushMatrix();
	glTranslatef(-46.0, -3.0, 80.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glScalef(30.0, 20.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif6);
	//glColor3f(0.65, 0.50, 0.39);
	glutSolidCube(2.0);
	glPopMatrix();
}

void drawAll() {

	float matShine[] = { 50.0 };
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);

	if (overhead == true)
		gluLookAt(0.0, 60.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0);

	gluLookAt(meX, meY, meZ,
		sin(angle * PI / 180) + meX, pointOfView, cos(angle * PI / 180) + meZ,
		0.0, 1.0, 0.0);

	glEnable(GL_LIGHTING);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);

	drawWalls();
	drawGround();
	drawFLoor();
	//drawRoof();
	drawRoad();
	drawSun();
}

// The mouse callback routine.
void mousePickFunction(int button, int state, int x, int y)
{
	int viewport[4]; // Viewport data.

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) //mouse clicked
	{
		glGetIntegerv(GL_VIEWPORT, viewport); // Get viewport data.

		glSelectBuffer(1024, buffer); // Specify buffer to write hit records in selection mode
		(void)glRenderMode(GL_SELECT); // Enter selection mode.

									   // Save the viewing volume defined in the resize routine.
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();

		// Define a viewing volume corresponding to selecting in 3 x 3 region around the cursor.
		glLoadIdentity();
		gluPickMatrix((float)x, (float)(viewport[3] - y), 3.0, 3.0, viewport);
		gluPerspective(120, 1, 1, 100);

		glMatrixMode(GL_MODELVIEW); // Return to modelview mode before drawing.
		glLoadIdentity();

		glInitNames(); // Initializes the name stack to empty.
		glPushName(0); // Puts name 0 on top of stack.

					   // Determine hits by calling drawBallAndTorus() so that names are assigned.
		isSelecting = 1;
		drawAll();

		hits = glRenderMode(GL_RENDER); // Return to rendering mode, returning number of hits.

										// Determine closest of the hit objects (if any).
		findClosestHit(hits, buffer);
		if (closestName == 1) { 

		}



		cout << "closest hit = " << closestName << endl;

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		glutPostRedisplay();
	}
}

void drawScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	isSelecting = 0;

	drawAll();
	glutSwapBuffers();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	cout << "Interaction:\n" << endl;
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
		case '0':
			overhead = !overhead;
			glutPostRedisplay();
		break;
		case 27:
			exit(0);
			break;
		default:
			break;
	}
}

void specialKeyInput(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP: //forward
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			pointOfView += stepsize;
		else
		{
			meZ += stepsize*cos(angle*PI / 180);
			meX += stepsize*sin(angle*PI / 180);
		}
		break;
	case GLUT_KEY_DOWN: //back
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			pointOfView -= stepsize;
		else
		{
			meZ -= stepsize*cos(angle*PI / 180);
			meX -= stepsize*sin(angle*PI / 180);
		}
		break;
	case GLUT_KEY_RIGHT: //turn right
		angle -= turnsize;
		break;
	case GLUT_KEY_LEFT: //turn left
		angle += turnsize;
		break;
	}
	glutPostRedisplay();
}

// Main routine.
int main(int argc, char **argv)
{
	printInteraction();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(850, 600);
	glutInitWindowPosition(400, 50);
	glutCreateWindow("Art Gallery");
	setup();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	glutMouseFunc(mousePickFunction); // Mouse control.
	//glutTimerFunc(5, animate, 1);
	glutMainLoop();

	return 0;
}

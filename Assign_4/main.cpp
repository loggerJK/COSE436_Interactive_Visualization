/*
 * Skeleton code for COSE436 Fall 2023
 *
 * Won-Ki Jeong, wkjeong@korea.ac.kr
 *
 */

#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <assert.h>
#include "textfile.h"

#define FILE_NAME "../CThead_512_512_452.raw" // tooth_100_90_160.raw"
// #define FILE_NAME "../tooth_100_90_160.raw" // tooth_100_90_160.raw
// #define FILE_NAME "../lung_256_256_128.raw" // tooth_100_90_160.raw
#define W 512
#define H 512
#define D 452

int volumeRenderingWindow;
int transferFunctionWindow;

// Shader programs
GLuint p;

GLuint objectTex;
GLint transferFunctionTex;

float histogram[256];

GLint locEyePosition, locObjectTex, locObjectMin, locObjectMax, locUp, locTransferFunctionTex;

float eyePosition[3] = {0, 0, 5};
float scale = 1;
float up[3] = {0, 1, 0};

GLuint transferFunctiontex;
float transferFunction[256 * 4];
bool transferFunctionChanged = false;

void load3Dfile(char *filename, int w, int h, int d)
{
	FILE *f = fopen(filename, "rb");
	unsigned char *data = new unsigned char[w * h * d];
	fread(data, 1, w * h * d, f);
	fclose(f);

	glGenTextures(1, &objectTex);
	glBindTexture(GL_TEXTURE_3D, objectTex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, w, h, d, 0, GL_RED, GL_UNSIGNED_BYTE, data);

	for (int i = 0; i < 256; i++)
	{
		histogram[i] = 0;
	}
	for (int i = 0; i < w * h * d; i++)
	{
		histogram[data[i]]++;
	}
	for (int i = 0; i < 256; i++)
	{
		histogram[i] /= w * h * d;
	}

	delete[] data;
}
void changeSize(int w, int h)
{

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = 1.0f * (float)w / (float)h;

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'p')
	{
	}

	glutPostRedisplay();
}

void renderScene(void)
{
	glClearColor(0, 0, 0, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(p);

	glUniform3fv(locEyePosition, 1, eyePosition);
	float objectMin[3] = {-W / sqrt(W * W + H * H + D * D) * scale, -H / sqrt(W * W + H * H + D * D) * scale, -D / sqrt(W * W + H * H + D * D) * scale};
	float objectMax[3] = {W / sqrt(W * W + H * H + D * D) * scale, H / sqrt(W * W + H * H + D * D) * scale, D / sqrt(W * W + H * H + D * D) * scale};
	printf("Object Min : %f %f %f\n", objectMin[0], objectMin[1], objectMin[2]);
	printf("Object Max : %f %f %f\n", objectMax[0], objectMax[1], objectMax[2]);
	glUniform3fv(locObjectMin, 1, objectMin);
	glUniform3fv(locObjectMax, 1, objectMax);
	glUniform3fv(locUp, 1, up);

	glUniform4fv(locTransferFunctionTex, 256, transferFunction);

	glBegin(GL_QUADS);
	glVertex3f(-1, -1, -1);
	glVertex3f(-1, 1, -1);
	glVertex3f(1, 1, -1);
	glVertex3f(1, -1, -1);
	// glVertex3f(-0.5, -0.5, -1);
	// glVertex3f(-0.5, 0.5, -1);
	// glVertex3f(0.5, 0.5, -1);
	// glVertex3f(0.5, -0.5, -1);
	glEnd();
	glUseProgram(0);

	glutSwapBuffers();
}

void idle()
{
	if (transferFunctionChanged)
	{
		glutSetWindow(volumeRenderingWindow);
		transferFunctionChanged = false;
		glutPostRedisplay();
	}
}
void init()
{
	load3Dfile(FILE_NAME, W, H, D);
	glUseProgram(p);

	locEyePosition = glGetUniformLocation(p, "eyePosition");
	locObjectTex = glGetUniformLocation(p, "tex");
	locObjectMin = glGetUniformLocation(p, "objectMin");
	locObjectMax = glGetUniformLocation(p, "objectMax");
	locUp = glGetUniformLocation(p, "up");
	locTransferFunctionTex = glGetUniformLocation(p, "transferFunction");

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(locObjectTex, 0);
	glBindTexture(GL_TEXTURE_3D, objectTex);

	for (int i = 0; i < 256; i++)
	{
		transferFunction[i * 4 + 0] = float(i) / 255.0;
		transferFunction[i * 4 + 1] = float(i) / 255.0;
		transferFunction[i * 4 + 2] = float(i) / 255.0;
		transferFunction[i * 4 + 3] = float(i) / 255.0;
	}

	glUniform4fv(locTransferFunctionTex, 256, transferFunction);
}

int mouseButton = -1;
int mousePos[2];
void mouseClick(int button, int state, int x, int y)
{
	mouseButton = button;
	mousePos[0] = x;
	mousePos[1] = y;
}
float *normalize(float *a)
{
	float *t = new float[3];
	t[0] = a[0] / sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	t[1] = a[1] / sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	t[2] = a[2] / sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	return t;
}
float *cross_product(float *a, float *b)
{
	float *t = new float[3];
	t[0] = a[1] * b[2] - b[1] * a[2];
	t[1] = a[2] * b[0] - b[2] * a[0];
	t[2] = a[0] * b[1] - b[0] * a[1];
	return t;
}
void mouseMove(int x, int y)
{
	if (mouseButton == GLUT_LEFT_BUTTON)
	{
		float *t1 = new float[3]{-eyePosition[0], -eyePosition[1], -eyePosition[2]};
		float *t2 = normalize(cross_product(up, t1));
		float t3[3];
		t3[0] = eyePosition[0] + t2[0] * (x - mousePos[0]) / 1000;
		t3[1] = eyePosition[1] + t2[1] * (x - mousePos[0]) / 1000;
		t3[2] = eyePosition[2] + t2[2] * (x - mousePos[0]) / 1000;
		t2 = normalize(t3);
		eyePosition[0] = t2[0] * 5;
		eyePosition[1] = t2[1] * 5;
		eyePosition[2] = t2[2] * 5;

		t1 = new float[3]{-eyePosition[0], -eyePosition[1], -eyePosition[2]};
		t2 = normalize(cross_product(up, t1));
		t3[0] = eyePosition[0] + up[0] * (y - mousePos[1]) / 1000;
		t3[1] = eyePosition[1] + up[1] * (y - mousePos[1]) / 1000;
		t3[2] = eyePosition[2] + up[2] * (y - mousePos[1]) / 1000;
		t2 = normalize(cross_product(t1, t2));
		up[0] = t2[0];
		up[1] = t2[1];
		up[2] = t2[2];

		t2 = normalize(t3);
		eyePosition[0] = t2[0] * 5;
		eyePosition[1] = t2[1] * 5;
		eyePosition[2] = t2[2] * 5;
	}
	if (mouseButton == GLUT_RIGHT_BUTTON)
	{
		scale += 1.0 * (mousePos[1] - y) / 20000;
	}
	glutPostRedisplay();
}

// Transfer Function Part
float points[1000][2];
float colors[1000][3];
int nodeNum = 0;
int tfWidth = 600;
int tfHeight = 300;

float nodeInSize = 0.025;
float nodeOutSize = 0.03;
float LineWidth = 0.01;

void renderScene_transferFunction(void)
{

	transferFunctionChanged = true;
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(0);
	glLoadIdentity();
	glScalef(1.6, 1.6, 1);
	glTranslatef(-0.5, -0.5, 0);
	glBegin(GL_QUADS);
	int start_index = 0, stop_index = 1;
	for (int i = 0; i < 256; i++)
	{

		if (float(i) / 255 > points[stop_index][0])
			stop_index++, start_index++;
		float t_a = ((float(i) / 255 - points[start_index][0]) * points[stop_index][1] + (points[stop_index][0] - float(i) / 255) * points[start_index][1]) / (points[stop_index][0] - points[start_index][0]);
		glColor4f(((float(i) / 255 - points[start_index][0]) * colors[stop_index][0] + (points[stop_index][0] - float(i) / 255) * colors[start_index][0]) / (points[stop_index][0] - points[start_index][0]),
				  ((float(i) / 255 - points[start_index][0]) * colors[stop_index][1] + (points[stop_index][0] - float(i) / 255) * colors[start_index][1]) / (points[stop_index][0] - points[start_index][0]),
				  ((float(i) / 255 - points[start_index][0]) * colors[stop_index][2] + (points[stop_index][0] - float(i) / 255) * colors[start_index][2]) / (points[stop_index][0] - points[start_index][0]),
				  t_a);
		// glColor3f(1, 1, 1);
		glVertex2f(float(i) / 256, 0);
		glVertex2f(float(i) / 256, histogram[i] * 5);
		glVertex2f(float(i + 1) / 256, histogram[i] * 5);
		glVertex2f(float(i + 1) / 256, 0);
	}
	glEnd();

	glBegin(GL_QUADS);

	start_index = 0, stop_index = 1;
	for (int i = 0; i < 256; i++)
	{
		if (float(i) / 255 > points[stop_index][0])
			stop_index++, start_index++;
		glColor3f(((float(i) / 255 - points[start_index][0]) * colors[stop_index][0] + (points[stop_index][0] - float(i) / 255) * colors[start_index][0]) / (points[stop_index][0] - points[start_index][0]),
				  ((float(i) / 255 - points[start_index][0]) * colors[stop_index][1] + (points[stop_index][0] - float(i) / 255) * colors[start_index][1]) / (points[stop_index][0] - points[start_index][0]),
				  ((float(i) / 255 - points[start_index][0]) * colors[stop_index][2] + (points[stop_index][0] - float(i) / 255) * colors[start_index][2]) / (points[stop_index][0] - points[start_index][0]));
		// glColor3f(1, 1, 1);

		glVertex2f(float(i) / 256, 0);
		glVertex2f(float(i) / 256, -0.1);
		glVertex2f(float(i + 1) / 256, -0.1);
		glVertex2f(float(i + 1) / 256, 0);
	}
	glEnd();

	float a = points[0][1];
	float r = colors[0][0];
	float g = colors[0][1];
	float b = colors[0][2];
	transferFunction[0] = r;
	transferFunction[1] = g;
	transferFunction[2] = b;
	transferFunction[3] = a;
	for (int i = 0; i < nodeNum - 1; i++)
	{
		float a1 = points[i][1];
		float a2 = points[i + 1][1];
		float r1 = colors[i][0];
		float r2 = colors[i + 1][0];
		float g1 = colors[i][1];
		float g2 = colors[i + 1][1];
		float b1 = colors[i][2];
		float b2 = colors[i + 1][2];
		int x1 = int(points[i][0] * 255);
		int x2 = int(points[i + 1][0] * 255);

		for (int j = x1 + 1; j <= x2; j++)
		{

			float rr1 = (r2 - r1) / (x2 - x1) * (j - x1) + r1;
			float gg1 = (g2 - g1) / (x2 - x1) * (j - x1) + g1;
			float bb1 = (b2 - b1) / (x2 - x1) * (j - x1) + b1;
			float aa1 = (a2 - a1) / (x2 - x1) * (j - x1) + a1;
			float ta = aa1;
			r = rr1;
			g = gg1;
			b = bb1;
			a = ta;
			transferFunction[j * 4 + 0] = r;
			transferFunction[j * 4 + 1] = g;
			transferFunction[j * 4 + 2] = b;
			transferFunction[j * 4 + 3] = a;
		}
	}

	glBegin(GL_QUADS);
	for (int i = 1; i < nodeNum; i++)
	{
		glColor3f(colors[i - 1][0], colors[i - 1][1], colors[i - 1][2]);
		glVertex2f(points[i - 1][0], points[i - 1][1] - LineWidth);
		glVertex2f(points[i - 1][0], points[i - 1][1] + LineWidth);
		glColor3f(colors[i][0], colors[i][1], colors[i][2]);
		glVertex2f(points[i][0], points[i][1] - LineWidth);
		glVertex2f(points[i][0], points[i][1] + LineWidth);
	}
	glEnd();

	glBegin(GL_QUADS);
	for (int i = 0; i < nodeNum; i++)
	{
		glColor3f(1 - colors[i][0], 1 - colors[i][1], 1 - colors[i][2]);
		glVertex2f(points[i][0] - nodeOutSize, points[i][1] - nodeOutSize);
		glVertex2f(points[i][0] - nodeOutSize, points[i][1] + nodeOutSize);
		glVertex2f(points[i][0] + nodeOutSize, points[i][1] + nodeOutSize);
		glVertex2f(points[i][0] + nodeOutSize, points[i][1] - nodeOutSize);
		glColor3f(colors[i][0], colors[i][1], colors[i][2]);
		glVertex2f(points[i][0] - nodeInSize, points[i][1] - nodeInSize);
		glVertex2f(points[i][0] - nodeInSize, points[i][1] + nodeInSize);
		glVertex2f(points[i][0] + nodeInSize, points[i][1] + nodeInSize);
		glVertex2f(points[i][0] + nodeInSize, points[i][1] - nodeInSize);
	}
	glEnd();
	glutSwapBuffers();
}
void changeSize_transferFunction(int w, int h)
{

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = 1.0f * (float)w / (float)h;

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	tfWidth = w;
	tfHeight = h;
}
int mouseButton_transferFunction = -1;
int mousePos_transferFunction[2];
int selectPoint = -1;

void mouseClick_transferFunction(int button, int state, int x, int y)
{
	int mod = glutGetModifiers();
	mouseButton_transferFunction = button;
	mousePos_transferFunction[0] = x;
	mousePos_transferFunction[1] = y;
	float mousePos[2] = {(float(x) / tfWidth * 2 - 1) / 1.6 + 0.5, ((1 - float(y) / tfHeight) * 2 - 1) / 1.6 + 0.5};
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (mod == GLUT_ACTIVE_SHIFT)
		{
			for (int i = 1; i < nodeNum - 1; i++)
			{
				if (abs(mousePos[0] - points[i][0]) < nodeOutSize && abs(mousePos[1] - points[i][1]) < nodeOutSize)
				{
					for (int j = i; j < nodeNum - 1; j++)
					{
						points[j][0] = points[j + 1][0];
						points[j][1] = points[j + 1][1];
						colors[j][0] = colors[j + 1][0];
						colors[j][1] = colors[j + 1][1];
						colors[j][2] = colors[j + 1][2];
					}
					nodeNum--;
				}
			}
		}
		else
		{
			for (int i = 0; i < nodeNum; i++)
			{
				if (abs(mousePos[0] - points[i][0]) < nodeOutSize && abs(mousePos[1] - points[i][1]) < nodeOutSize)
				{
					colors[i][0] = float(rand()) / RAND_MAX;
					colors[i][1] = float(rand()) / RAND_MAX;
					colors[i][2] = float(rand()) / RAND_MAX;
					break;
				}
			}
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (mod == GLUT_ACTIVE_SHIFT)
		{
			if (mousePos[0] > 1)
				mousePos[0] = 1;
			if (mousePos[0] < 0)
				mousePos[0] = 0;
			if (mousePos[1] > 1)
				mousePos[1] = 1;
			if (mousePos[1] < 0)
				mousePos[1] = 0;
			for (int i = 1; i < nodeNum; i++)
			{
				if (mousePos[0] < points[i][0])
				{
					for (int j = nodeNum + 1; j > i; j--)
					{
						points[j][0] = points[j - 1][0];
						points[j][1] = points[j - 1][1];
						colors[j][0] = colors[j - 1][0];
						colors[j][1] = colors[j - 1][1];
						colors[j][2] = colors[j - 1][2];
					}

					points[i][0] = mousePos[0];
					points[i][1] = mousePos[1];
					colors[i][0] = float(rand()) / RAND_MAX;
					colors[i][1] = float(rand()) / RAND_MAX;
					colors[i][2] = float(rand()) / RAND_MAX;
					nodeNum++;
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < nodeNum; i++)
			{
				if (abs(mousePos[0] - points[i][0]) < nodeOutSize && abs(mousePos[1] - points[i][1]) < nodeOutSize)
				{
					selectPoint = i;
					break;
				}
			}
		}
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		selectPoint = -1;
	}
	glutPostRedisplay();
}

void mouseMove_transferFunction(int x, int y)
{
	if (mouseButton_transferFunction == GLUT_LEFT_BUTTON)
	{
		if (selectPoint != -1)
		{
			float newX = (float(x) / tfWidth * 2 - 1) / 1.6 + 0.5;
			float newY = ((1 - float(y) / tfHeight) * 2 - 1) / 1.6 + 0.5;
			if (newX > 1)
				newX = 1;
			if (newY > 1)
				newY = 1;
			if (newX < 0)
				newX = 0;
			if (newY < 0)
				newY = 0;

			if (selectPoint != 0 && selectPoint != nodeNum - 1)
			{
				points[selectPoint][0] = newX;
				if (newX < points[selectPoint - 1][0])
				{
					float t[2] = {points[selectPoint][0], points[selectPoint][1]};
					points[selectPoint][0] = points[selectPoint - 1][0];
					points[selectPoint][1] = points[selectPoint - 1][1];
					points[selectPoint - 1][0] = t[0];
					points[selectPoint - 1][1] = t[1];
					float t2[3] = {colors[selectPoint][0], colors[selectPoint][1], colors[selectPoint][2]};
					colors[selectPoint][0] = colors[selectPoint - 1][0];
					colors[selectPoint][1] = colors[selectPoint - 1][1];
					colors[selectPoint][2] = colors[selectPoint - 1][2];
					colors[selectPoint - 1][0] = t2[0];
					colors[selectPoint - 1][1] = t2[1];
					colors[selectPoint - 1][2] = t2[2];
					selectPoint--;
				}
				if (newX > points[selectPoint + 1][0])
				{
					float t[2] = {points[selectPoint][0], points[selectPoint][1]};
					points[selectPoint][0] = points[selectPoint + 1][0];
					points[selectPoint][1] = points[selectPoint + 1][1];
					points[selectPoint + 1][0] = t[0];
					points[selectPoint + 1][1] = t[1];
					float t2[3] = {colors[selectPoint][0], colors[selectPoint][1], colors[selectPoint][2]};
					colors[selectPoint][0] = colors[selectPoint + 1][0];
					colors[selectPoint][1] = colors[selectPoint + 1][1];
					colors[selectPoint][2] = colors[selectPoint + 1][2];
					colors[selectPoint + 1][0] = t2[0];
					colors[selectPoint + 1][1] = t2[1];
					colors[selectPoint + 1][2] = t2[2];
					selectPoint++;
				}
			}
			points[selectPoint][1] = newY;
		}
	}
	glutPostRedisplay();
}

void init_transferFunction()
{
	nodeNum = 2;
	points[0][0] = 0, points[0][1] = 0;
	points[1][0] = 1, points[1][1] = 1;
	colors[0][0] = 0, colors[0][1] = 0, colors[0][2] = 0;
	colors[1][0] = 1, colors[1][1] = 1, colors[1][2] = 1;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char **argv)
{

	// For transfer functoin
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 700);
	glutInitWindowSize(600, 300);
	transferFunctionWindow = glutCreateWindow("Transfer Function");

	// register callbacks
	glutDisplayFunc(renderScene_transferFunction);
	glutReshapeFunc(changeSize_transferFunction);

	glutMouseFunc(mouseClick_transferFunction);
	glutMotionFunc(mouseMove_transferFunction);
	glutIdleFunc(idle);

	init_transferFunction();

	// init GLUT and create Window
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	volumeRenderingWindow = glutCreateWindow("Volume Rendering");

	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keyboard);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);

	glutIdleFunc(idle);

	glEnable(GL_DEPTH_TEST);

	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
		printf("Ready for OpenGL 3.3\n");
	else
	{
		printf("OpenGL 3.3 is not supported\n");
		exit(1);
	}

	// Create shader program
	p = createGLSLProgram("../volumeRendering.vert", NULL, "../volumeRendering.frag");

	init();

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}
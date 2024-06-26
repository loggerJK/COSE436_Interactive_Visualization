/*
 * Skeleton code for COSE436 Fall 2023 Assignment 1
 *
 * Won-Ki Jeong, wkjeong@korea.ac.kr
 1. 코드 정리
 2. Vertex Normal 계산
 3. Lighting

 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <assert.h>
#include "textfile.h"
#include "Angel.h"
inline mat4 LookAt_(const vec3 &eye, const vec3 &at, const vec3 &up)
{
	vec4 n = vec4(normalize(eye - at), 0);
	vec4 u = vec4(normalize(cross(up, n)), 0.0);
	vec4 v = vec4(normalize(cross(n, u)), 0.0);
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);
	return c;
}
// Shader programs
GLuint p;
int modelNum = 0;
int light_source = 0;
char viewSelection = 'p';

float lastPos[3] = {0.0, 0.0, 0.0};
float angle = 0.0, axis[3], trans[3];
bool enableRotation = false, enableTranslation = false, enableZooming = false;

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;

vec3 eye = {0, 0.125, 0.4};
vec3 origin = {0, 0, 0};
// vec3 at = origin;
vec3 at = {0, 0.125, 0};
vec3 up = {0, 1, 0};
mat4 cur_modelView = (LookAt_(eye, at, up) * Translate(-eye));
// mat4 cur_modelView = mat4(1.0f);
mat4 scale = mat4(1.0f);
mat4 translation = mat4(1.0f);

vec3 *vertices;
int *indices;
vec3 *normals;
int num_vertices, num_faces;
size_t size_vertices = 0;
size_t size_normals = 0;
size_t size_indices = 0;

// Initialize shader lighting parameters
vec4 light_ambient(1.0, 1.0, 1.0, 1.0);
vec4 light_position[2];
vec4 light_diffuse[2];
vec4 light_specular[2];
float alpha[2] = {10.0, 10.0};
float k_d[2] = {1.0, 1.0};
float k_s[2] = {0.5, 0.5};
float k_a = 1.0;

int readAssignment()
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;

	fp = fopen("../mesh-data/bunny.off", "r");
	if (fp == NULL)
	{
		printf("Error: could not open file\n");
		exit(EXIT_FAILURE);
	}
	fscanf(fp, "OFF\n %d %d 0\n", &num_vertices, &num_faces);

	vertices = (vec3 *)malloc(num_vertices * sizeof(vec3));
	for (int i = 0; i < num_vertices; i++)
	{
		fscanf(fp, "%f %f %f\n", &vertices[i].x, &vertices[i].y, &vertices[i].z);
	}

	indices = (int *)malloc(num_faces * 3 * sizeof(int));
	for (int i = 0; i < num_faces; i++)
	{
		fscanf(fp, "3 %d %d %d\n", &indices[3 * i], &indices[3 * i + 1], &indices[3 * i + 2]);
	}

	// Vertex Normal
	normals = (vec3 *)malloc(num_vertices * sizeof(vec3));
	for (int i = 0; i < num_vertices; i++)
		normals[i] = vec3(0, 0, 0);

	for (int i = 0; i < num_faces; i++)
	{
		vec3 v1 = vertices[indices[3 * i + 1]] - vertices[indices[3 * i]];
		vec3 v2 = vertices[indices[3 * i + 2]] - vertices[indices[3 * i]];
		vec3 normal = normalize(cross(v1, v2)); // Surface normal
		// Add surface normal to vertex normal
		normals[indices[3 * i]] += normal;
		normals[indices[3 * i + 1]] += normal;
		normals[indices[3 * i + 2]] += normal;
	}

	for (int i = 0; i < num_vertices; i++)
		normals[i] = normalize(normals[i]);

	size_vertices = num_vertices * sizeof(vec3);
	size_indices = num_faces * 3 * sizeof(int);
	size_normals = num_vertices * sizeof(vec3);

	fclose(fp);

	return 0;
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

void trackball_ptov(int x, int y, int width, int height, float v[3])
{
	float d, a;
	/* project x,y onto a hemi-sphere centered within width,
	height */
	v[0] = (2.0f * x - width) / width;	 // [-1, 1]
	v[1] = (height - 2.0f * y) / height; // [-1, 1] // 이거 왜 반대임?
	// printf("x : %d | y : %d\n", x, y);
	// printf("v[1] : %.3f\n", v[1]);
	d = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
	v[2] = (float)cos((M_PI / 2.0F) * ((d < 1.0f) ? d : 1.0f)); // radius 1 , 이거 왜 0 아님?

	// Normalize
	a = 1.0F / (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] *= a;
	v[1] *= a;
	v[2] *= a;
}
void mouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		float curPos[3], dx = 0, dy = 0, dz = 0;
		int winWidth = 600, winHeight = 600;
		switch (state)
		{
		case GLUT_DOWN:
			// startMotion(x, y);
			enableRotation = true;
			trackball_ptov(x, y, winWidth, winHeight, curPos);
			for (int i = 0; i < 3; i++)
				lastPos[i] = curPos[i];
			break;
		case GLUT_UP:
			// stopMotion(x, y);
			enableRotation = false;
			for (int i = 0; i < 3; i++)
				axis[i] = 0;
			break;
		}
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		float curPos[3], dx = 0, dy = 0, dz = 0;
		int winWidth = 600, winHeight = 600;
		switch (state)
		{
		case GLUT_DOWN:
			enableTranslation = true;
			trackball_ptov(x, y, winWidth, winHeight, curPos);
			for (int i = 0; i < 3; i++)
				lastPos[i] = curPos[i];
			break;
		case GLUT_UP:
			enableTranslation = false;
			translation = mat4(1.0f);
			break;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		float curPos[3], dx = 0, dy = 0, dz = 0;
		int winWidth = 600, winHeight = 600;
		switch (state)
		{
		case GLUT_DOWN:
			enableZooming = true;
			trackball_ptov(x, y, winWidth, winHeight, curPos);
			for (int i = 0; i < 3; i++)
				lastPos[i] = curPos[i];
			break;
		case GLUT_UP:
			enableZooming = false;
			scale = mat4(1.0f);
			break;
		}
	}
}
void mouseMotion(int x, int y)
{
	float curPos[3], dx = 0, dy = 0, dz = 0;
	/* compute position on hemisphere */
	int winWidth = 600, winHeight = 600;
	trackball_ptov(x, y, winWidth, winHeight, curPos);
	/* compute the change in position
	on the hemisphere */
	dx = curPos[0] - lastPos[0];
	dy = curPos[1] - lastPos[1];
	dz = curPos[2] - lastPos[2];
	if (dx || dy || dz)
	{
		if (enableRotation)
		{
			/* compute theta and cross product */
			angle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz);
			axis[0] = lastPos[1] * curPos[2] - lastPos[2] * curPos[1];
			axis[1] = lastPos[2] * curPos[0] - lastPos[0] * curPos[2];
			axis[2] = lastPos[0] * curPos[1] - lastPos[1] * curPos[0];
			/* update position */
			lastPos[0] = curPos[0];
			lastPos[1] = curPos[1];
			lastPos[2] = curPos[2];
		}
		if (enableTranslation)
		{
			translation = (Translate(dx / 1000, dy / 1000, 0));
		}
		if (enableZooming)
		{
			if (dy > 0)
				scale = scale * transpose(Scale(1.0001, 1.0001, 1.0001));
			else
				scale = scale * transpose(Scale(0.9999, 0.9999, 0.9999));
		}
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	// ToDo
	if (key == 'm')
	{
		modelNum++;
		modelNum %= 3;
		// printf("Model Num : %d\n", modelNum);
	}
	if (key == 'p')
		viewSelection = key;
	if (key == 'o')
		viewSelection = key;
	if (key == 's')
	{
		light_source++;
		light_source %= 2;
		if (light_source == 0)
			printf("light source : 0 (purple)\n");
		else
			printf("light source : 1 (green)\n");
	}
	if (key == '1')
	{
		k_d[light_source] -= 0.01;
		if (k_d[light_source] <= 0)
			k_d[light_source] = 0;
	}
	if (key == '3')
	{
		k_d[light_source] += 0.01;
		if (k_d[light_source] >= 1)
			k_d[light_source] = 1;
	}
	if (key == '4')
	{
		k_a -= 0.01;
		if (k_a <= 0)
			k_a = 0;
	}
	if (key == '6')
	{
		k_a += 0.01;
		if (k_a >= 1)
			k_a = 1;
	}
	if (key == '7')
	{
		k_s[light_source] -= 0.01;
		if (k_s[light_source] <= 0)
			k_s[light_source] = 0;
	}
	if (key == '9')
	{
		k_s[light_source] += 0.01;
		if (k_s[light_source] >= 1)
			k_s[light_source] = 1;
	}
	if (key == '-')
	{
		alpha[light_source] -= 1;
		if (alpha[light_source] <= 1)
			alpha[light_source] = 1;
	}
	if (key == '+')
	{
		alpha[light_source] += 1;
		if (alpha[light_source] >= 100)
			alpha[light_source] = 100;
	}
	glutPostRedisplay();
}

void renderScene(void)
{
	// ToDo

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rotating Model View
	mat4 rotation = (RotateX(axis[0]) * RotateY(axis[1]) * RotateZ(axis[2]));

	cur_modelView = cur_modelView * scale;
	cur_modelView = Translate((cur_modelView * at)) * rotation * Translate(-(cur_modelView * at)) * cur_modelView;
	// cur_modelView = Translate((cur_modelView * at)) * RotateY(0.1) * Translate(-(cur_modelView * at)) * cur_modelView;
	cur_modelView = translation * cur_modelView;
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, cur_modelView);

	// Projection
	mat4 projection = mat4(1.0f);
	if (viewSelection == 'p')
		projection = Perspective(45.0f, 1.0f, 0.1f, 100.0f);
	else
		projection = Ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f);

	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

	glValidateProgram(p);
	GLint status;
	glGetProgramiv(p, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("Error : Validating program");
	}

	light_ambient = vec4(0.2);

	light_position[0] = vec4(0.0, 1.0, 0.0, 0.0);
	light_diffuse[0] = vec4(0.961, 0.78, 0.949, 1.0);
	light_specular[0] = vec4(1.0, 1.0, 1.0, 1.0);

	glUniform4fv(glGetUniformLocation(p, "AmbientProduct_0"),
				 1, light_ambient);
	glUniform4fv(glGetUniformLocation(p, "DiffuseProduct_0"),
				 1, light_diffuse[0]);
	glUniform4fv(glGetUniformLocation(p, "SpecularProduct_0"),
				 1, light_specular[0]);

	glUniform4fv(glGetUniformLocation(p, "LightPosition_0"),
				 1, light_position[0]);

	glUniform1f(glGetUniformLocation(p, "k_a"), k_a);
	glUniform1f(glGetUniformLocation(p, "k_d_0"), k_d[0]);
	glUniform1f(glGetUniformLocation(p, "k_s_0"), k_s[0]);
	glUniform1f(glGetUniformLocation(p, "alpha_0"), alpha[0]);

	light_position[1] = vec4(0.0, -1.0, 0.0, 0.0);
	light_diffuse[1] = vec4(0.784, 0.961, 0.78, 1.0);
	light_specular[1] = vec4(1.0, 1.0, 1.0, 1.0);

	glUniform4fv(glGetUniformLocation(p, "DiffuseProduct_1"),
				 1, light_diffuse[1]);
	glUniform4fv(glGetUniformLocation(p, "SpecularProduct_1"),
				 1, light_specular[1]);

	glUniform4fv(glGetUniformLocation(p, "LightPosition_1"),
				 1, light_position[1]);

	glUniform1f(glGetUniformLocation(p, "k_d_1"), k_d[1]);
	glUniform1f(glGetUniformLocation(p, "k_s_1"), k_s[1]);
	glUniform1f(glGetUniformLocation(p, "alpha_1"), alpha[1]);

	glDrawElements(GL_TRIANGLES, num_faces * 3, GL_UNSIGNED_INT, 0);

	glutSwapBuffers();
}

void idle()
{
	glutPostRedisplay();
}

void init()
{

	// Create a vertex array object
	// GLuint vao;
	// glGenVertexArrays(1, &vao);
	// glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, size_vertices + size_normals, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size_vertices, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, size_vertices, size_normals, normals);

	// Create IBO
	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_indices, indices, GL_STATIC_DRAW);

	// Create shader program
	// p = createGLSLProgram("../vshader_goaroud.vert", NULL, "../fshader_goaroud.frag");
	p = createGLSLProgram("../vshader.vert", NULL, "../fshader.frag");

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(p, "ModelView");
	Projection = glGetUniformLocation(p, "Projection");

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(p, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(p, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(size_vertices));

	// Attenuation parameter
	vec3 attenuation = vec3(1, 0.1, 0.1);
	glUniform3fv(glGetUniformLocation(p, "attenuation"),
				 1, attenuation);

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	printf("Default light source : 0 (purple)\n");
}

int main(int argc, char **argv)
{
	readAssignment();
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("COSE436 - Assignment 1");

	// register callbacks
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutIdleFunc(idle);

	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
		printf("Ready for OpenGL 3.3\n");
	else
	{
		printf("OpenGL 3.3 is not supported\n");
		exit(1);
	}

	init();

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}
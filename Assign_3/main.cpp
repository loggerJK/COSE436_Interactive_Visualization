/*
 * Skeleton code for COSE436 Fall 2023 Assignment 3
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
#include "Angel.h"
#include <vector>
#include <map>
using namespace std;
// ############# MOUSE TRACKBALL #############
int modelNum = 0;
int light_source = 0;
char viewSelection = 'p';
float lastPos[3] = {0.0, 0.0, 0.0};
float angle = 0.0, axis[3], trans[3];
bool enableRotation = false, enableTranslation = false, enableZooming = false;
mat4 scale = mat4(1.0f);
mat4 translation = mat4(1.0f);
int winWidth = 600, winHeight = 600;

inline mat4 LookAt_(const vec3 &eye, const vec3 &at, const vec3 &up)
{
	vec4 n = vec4(normalize(eye - at), 0);
	vec4 u = vec4(normalize(cross(up, n)), 0.0);
	vec4 v = vec4(normalize(cross(n, u)), 0.0);
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 c = mat4(u, v, n, t);
	return c;
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
			translation = (Translate(dx / 100, dy / 100, 0));
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
	if (key == 'p')
		viewSelection = key;
	if (key == 'o')
		viewSelection = key;
	glutPostRedisplay();
}
// ###########################################

// Model-view and projection matrices uniform location
GLuint ModelView, Projection, World, skyboxFlag, fskyboxFlag;
vec3 eye = {20, 5, 20};
vec3 origin = {0, 0, 0};
// vec3 at = origin;
vec3 at = {0, 5, 0};
vec3 up = {0, 1, 0};
mat4 cur_modelView = (LookAt_(eye, at, up) * Translate(-eye));
mat4 cur_modelView_skybox = cur_modelView;
vector<mat4> World_list;

struct PackedVertex
{
	vec3 position;
	vec2 uv;
	vec3 normal;
	bool operator<(const PackedVertex that) const
	{
		return memcmp((void *)this, (void *)&that, sizeof(PackedVertex)) > 0;
	};
};

bool getSimilarVertexIndex_fast(
	PackedVertex &packed,
	map<PackedVertex, unsigned short> &VertexToOutIndex,
	unsigned short &result)
{
	map<PackedVertex, unsigned short>::iterator it = VertexToOutIndex.find(packed);
	if (it == VertexToOutIndex.end())
	{
		return false;
	}
	else
	{
		result = it->second;
		return true;
	}
}

void indexVBO(
	vector<vec3> &in_vertices,
	vector<vec2> &in_uvs,
	vector<vec3> &in_normals,

	vector<unsigned short> &out_indices, // 최종 IBO
	vector<vec3> &out_vertices,			 // 최종 VBO
	vector<vec2> &out_uvs,				 // 최종 VBO
	vector<vec3> &out_normals)			 // 최종 VBO
{
	map<PackedVertex, unsigned short> VertexToOutIndex;

	// For each input vertex
	for (unsigned int i = 0; i < in_vertices.size(); i++)
	{

		PackedVertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

		if (found)
		{ // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else
		{ // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);

			// Push
			unsigned short newindex = (unsigned short)out_vertices.size() - 1;
			out_indices.push_back(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
}

// Definitions
vector<vec3> vertices;
vector<vec2> uvs;
vector<vec3> normals;
vector<vector<unsigned short>> indices_list;
vector<GLuint> ibo_list;
vector<GLuint> color_tex_list;
size_t size_vertices = 0;
size_t size_normals = 0;
size_t size_uvs = 0;
size_t size_indices = 0;
vector<string> image_list = {
	"../textures/steve.bmp",
	"../textures/spider.bmp",
	"../textures/blackspider.bmp",
	"../textures/grass.bmp",
	"../textures/stones.bmp",
};
vector<string> obj_list = {
	"../objs/steve.obj",
	"../objs/spider.obj",
	"../objs/blackspider.obj",
	"../objs/grass.obj",
	"../objs/stones.obj",
	"../objs/cube.obj",
	"../objs/skybox.obj",
};
vector<string> skybox_list = {
	"../textures/skybox/right.bmp",
	"../textures/skybox/left.bmp",
	"../textures/skybox/bottom.bmp",
	"../textures/skybox/top.bmp",
	"../textures/skybox/front.bmp",
	"../textures/skybox/back.bmp"};

typedef struct
{
	unsigned char x, y, z, w;
} uchar4;
typedef unsigned char uchar;
vector<uchar4 *> dst_list;

// BMP loader
void LoadBMPFile(uchar4 **dst, int *width, int *height, const char *name);

// Shader programs
GLuint p, p2, p3;
// Texture
GLuint color_tex;
int old_t;

void loadObj()
{
	for (int i = 0; i < obj_list.size(); i++)
	{
		vector<vec3> temp_vertices;
		vector<vec2> temp_uvs;
		vector<vec3> temp_normals;
		vector<vec3> in_vertices;
		vector<vec2> in_uvs;
		vector<vec3> in_normals;
		vector<unsigned short> temp_indices;

		FILE *file;
		file = fopen(obj_list[i].c_str(), "r");

		if (file == NULL)
		{
			printf("Error: could not open file\n");
			exit(EXIT_FAILURE);
		}

		// Read each line of the obj file
		while (1)
		{
			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.

			// else : parse lineHeader
			if (strcmp(lineHeader, "v") == 0)
			{
				vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9)
				{
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					exit(EXIT_FAILURE);
				}
				// 각 face attribute들에 대해서, 실제 position, uv, normal을 가져와서 배열에 저장한다.
				in_vertices.push_back(temp_vertices[vertexIndex[0] - 1]);
				in_vertices.push_back(temp_vertices[vertexIndex[1] - 1]);
				in_vertices.push_back(temp_vertices[vertexIndex[2] - 1]);
				in_uvs.push_back(temp_uvs[uvIndex[0] - 1]);
				in_uvs.push_back(temp_uvs[uvIndex[1] - 1]);
				in_uvs.push_back(temp_uvs[uvIndex[2] - 1]);
				in_normals.push_back(temp_normals[normalIndex[0] - 1]);
				in_normals.push_back(temp_normals[normalIndex[1] - 1]);
				in_normals.push_back(temp_normals[normalIndex[2] - 1]);
			}
		}

		// 최종 indices, vertices, uvs, normals를 생성한다.
		indexVBO(in_vertices, in_uvs, in_normals, temp_indices, vertices, uvs, normals);
		indices_list.push_back(temp_indices);

		// Close the file
		fclose(file);
	}
	size_vertices = vertices.size() * sizeof(vec3);
	size_normals = normals.size() * sizeof(vec3);
	size_uvs = uvs.size() * sizeof(vec2);
}

GLuint skybox_tex;
void loadSkyboxTexture()
{
	glGenTextures(1, &skybox_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	for (int i = 0; i < skybox_list.size(); i++)
	{
		uchar4 *dst;
		int width, height;
		LoadBMPFile(&dst, &width, &height, skybox_list[i].c_str());
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
void changeSize(int w, int h)
{

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = 1.0f * (float)w / (float)h;

	winHeight = h;
	winWidth = w;

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
}

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Scailing
	for (int i = 0; i < obj_list.size() - 1; i++)
		World_list[i] = scale * World_list[i];

	// Rotating Model View
	mat4 rotation = (RotateX(1.5 * axis[0]) * RotateY(1.5 * axis[1]) * RotateZ(1.5 * axis[2]));
	// cur_modelView = cur_modelView * scale;
	cur_modelView = Translate((cur_modelView * at)) * rotation * Translate(-(cur_modelView * at)) * cur_modelView;
	cur_modelView = translation * cur_modelView;
	cur_modelView_skybox = rotation * cur_modelView_skybox;

	// Projection
	mat4 projection = mat4(1.0f);
	if (viewSelection == 'p')
		projection = Perspective(45.0f, 1.0f, 0.1f, 200.0f);
	else
		projection = Ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 200.0f);

	glColor3f(1, 0, 0);

	// Draw Skybox
	glDepthMask(GL_FALSE);
	int skybox_idx = obj_list.size() - 1;
	glUseProgram(p2);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, (cur_modelView_skybox));
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_list[skybox_idx]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	glUniformMatrix4fv(World, 1, GL_TRUE, World_list[skybox_idx]);
	glDrawElements(GL_TRIANGLES, indices_list[skybox_idx].size(), GL_UNSIGNED_SHORT, 0);
	glDepthMask(GL_TRUE);

	// Draw textured objects
	glUseProgram(p);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, cur_modelView);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	for (int i = 0; i < image_list.size(); i++)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_list[i]);
		glBindTexture(GL_TEXTURE_2D, color_tex_list[i]);
		glUniformMatrix4fv(World, 1, GL_TRUE, World_list[i]);
		glDrawElements(GL_TRIANGLES, indices_list[i].size(), GL_UNSIGNED_SHORT, 0);
	}
	// Draw Glossy box
	int cube_idx = obj_list.size() - 2;
	glUseProgram(p3);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, cur_modelView);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_list[cube_idx]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	glUniformMatrix4fv(World, 1, GL_TRUE, World_list[cube_idx]);
	glDrawElements(GL_TRIANGLES, indices_list[cube_idx].size(), GL_UNSIGNED_SHORT, 0);

	glutSwapBuffers();
}

void idle()
{
	int t;
	/* Delta time in seconds. */
	float dt;
	t = glutGet(GLUT_ELAPSED_TIME);
	dt = (t - old_t) / 1000.0;
	if (dt > 0.01)
	{
		for (int i = 0; i < 3; i++)
			World_list[i] = RotateY(1) * World_list[i];
		old_t = t;
	}

	glutPostRedisplay();
}

void init()
{
	old_t = glutGet(GLUT_ELAPSED_TIME);
	// Load obj file
	loadObj();

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, size_vertices + size_normals + size_uvs, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size_vertices, &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, size_vertices, size_normals, &normals[0]);
	glBufferSubData(GL_ARRAY_BUFFER, size_vertices + size_normals, size_uvs, &uvs[0]);

	// Create IBO and buffer datas
	for (int i = 0; i < obj_list.size(); i++)
	{
		GLuint ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_list[i].size() * sizeof(unsigned short), &indices_list[i][0], GL_STATIC_DRAW);
		ibo_list.push_back(ibo);
	}

	// Create shader program
	vector<GLuint> programs;
	p3 = createGLSLProgram("../vshader.vert", NULL, "../fshader_3.frag");
	p2 = createGLSLProgram("../vshader.vert", NULL, "../fshader_2.frag");
	p = createGLSLProgram("../vshader.vert", NULL, "../fshader.frag");
	programs.push_back(p);
	programs.push_back(p2);
	programs.push_back(p3);

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(p, "ModelView");
	Projection = glGetUniformLocation(p, "Projection");
	World = glGetUniformLocation(p, "World");

	for (int i = 0; i < obj_list.size(); i++)
		World_list.push_back(mat4(1.0f));

	// set up vertex arrays

	for (int i = 0; i < programs.size(); i++)
	{
		GLuint vPosition = glGetAttribLocation(programs[i], "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
							  BUFFER_OFFSET(0));

		GLuint vNormal = glGetAttribLocation(programs[i], "vNormal");
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
							  BUFFER_OFFSET(size_vertices));

		GLuint vTexCoord = glGetAttribLocation(programs[i], "vTexCoord");
		glEnableVertexAttribArray(vTexCoord);
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
							  BUFFER_OFFSET(size_vertices + size_normals));
	}


	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Load image for texture
	int width, height;
	for (int i = 0; i < image_list.size(); i++)
	{
		uchar4 *dst;
		LoadBMPFile(&dst, &width, &height, image_list[i].c_str());
		// Create an RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &color_tex);
		glBindTexture(GL_TEXTURE_2D, color_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
		color_tex_list.push_back(color_tex);
	}
	// Load Skybox texture
	loadSkyboxTexture();
}

int main(int argc, char **argv)
{

	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("COSE436 - Assignment 3");

	// register callbacks
	glutDisplayFunc(renderScene);
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

	// Entering GLUT event processing cycle
	glutMainLoop();

	return 1;
}
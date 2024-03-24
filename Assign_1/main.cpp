/*
 * Skeleton code for COSE436 Fall 2023 Assignment 1
 *
 * Won-Ki Jeong, wkjeong@korea.ac.kr
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
char viewSelection = 'o';

float lastPos[3] = {0.0, 0.0, 0.0};
bool enableRotation = false, enableTranslation = false, enableZooming = false;
float angle = 0.0, axis[3], trans[3];

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;

// vec3 eye = {0, 0, 1};
vec3 eye = {0, 0.3, 0.5};
vec3 origin = {0, 0, 0};
// vec3 at = origin;
vec3 at = {0, 0.3, 0};
vec3 up = {0, 1, 0};
mat4 cur_modelView = (LookAt_(eye, at, up) * Translate(-eye));
// mat4 cur_modelView = mat4(1.0f);
mat4 scale = mat4(1.0f);
mat4 translation = mat4(1.0f);

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
    v[0] = (2.0f * x - width) / width;   // [-1, 1]
    v[1] = (height - 2.0f * y) / height; // [-1, 1]
    // printf("v[1] : %.3f\n", v[1]);
    d = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
    printf("x : %d | y : %d | d : %d\n", x, y, d);
    v[2] = (float)cos((M_PI / 2.0F) * ((d < 1.0f) ? d : 1.0f)); // max 1

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

    glutPostRedisplay();
}

void renderScene(void)
{
    // ToDo

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mat4 identity;

    // // Model View
    // mat4 model_view = mat4(1.0f);
    // vec4 eye = {0.4, 0.4, 0.4, 1};
    // // vec4 eye = {2, 2, 2, 1};
    // vec4 at = vec4(origin, 1);
    // vec4 up = {0, 1, 0, 1};
    // model_view = LookAt(eye, at, up);
    // cur_modelView = model_view;
    // glUniformMatrix4fv(ModelView, 1, GL_TRUE, cur_modelView); // 실제로는 Transpose되어서 들어감

    // Rotating Model View
    // printf("last_pos : (% e, % e, % e)\n", lastPos[0], lastPos[1], lastPos[2]);
    // printf("angle : (% e)\n", angle);
    // printf("axis : (% e, % e, % e)\n", axis[0], axis[1], axis[2]);
    mat4 rotation = (RotateX(axis[0]) * RotateY(axis[1]) * RotateZ(axis[2]));
    // mat4 rotation = (RotateX(0.1));
    // mat4 rotation = (RotateY(axis[1]) * RotateX(axis[0]));

    // Translation
    // mat4 translation = (Translate(0.00001, 0, 0));

    // Scale
    // scale = scale * (Scale(1.000, 1.000, 1.0001));

    // cur_modelView = LookAt_(eye, at, up) * Translate(-eye);
    cur_modelView = cur_modelView * scale;
    // cur_modelView = Translate((cur_modelView * at)) * rotation * Translate(-(cur_modelView * at)) * cur_modelView;

    cur_modelView = Translate((cur_modelView * at)) * RotateY(0.1) * Translate(-(cur_modelView * at)) * cur_modelView;

    cur_modelView = translation * cur_modelView;

    // mat4 cur_modelView_ = cur_modelView * RotateX(5);
    // cur_modelView = transpose(LookAt_(eye, at, up)) * Translate(eye) * cur_modelView;
    glUniformMatrix4fv(ModelView, 1, GL_TRUE,
                       cur_modelView);

    // Projection
    mat4 projection = mat4(1.0f);
    if (viewSelection == 'p')
        projection = Perspective(45.0f, 1.0f, 0.1f, 100.0f);
    else
        projection = Ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    // test vector
    vec3 test = vec3(0, 0, 0);
    // vec4 test_ = (LookAt_(eye, at, up) * Translate(-eye)) * test;
    vec4 test_ = -(cur_modelView * at);

    printf("x : %f | y : %f | z : %f | w : %f\n", test_[0], test_[1], test_[2], test_[3]);

    // // Debugging Purpose
    // vec4 test = {0, 0, -0.5, 1};
    // vec4 test_ = model_view * test;

    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();

    // 참고 결국은 Rotation + Translation인데, 이러한 행렬 연산은 현재 그려져 있는 모든 Object에 적용된다
    // 이때 Object는 Camera를 포함한다
    // 따라서, Object를 그린 후에 LookAt을 호출하는 것은 의미가 없다.
    // Camera와 Object가 동시에 이동하기 때문이다.
    // gluLookAt(0.4, 0.4, 0.4, 0, 0, 0, 0, 1, 0);

    glColor3f(1, 0, 0); // I will draw red from now on
    switch (modelNum)
    {
    case 0:
        glutWireTeapot(0.5);
        break;

    case 1:
        glutWireCube(0.5);
        break;

    case 2:
        glutWireCone(1, 1, 100, 100);
        break;
    default:
        break;
    }
    glColor3f(1, 0, 0); // Red, X axis
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glEnd();

    glColor3f(0, 1, 0); // Green, Y axis
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    glColor3f(0, 0, 1); // Blue, Z axis
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();

    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}

void init()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Create shader program
    p = createGLSLProgram("../vshader.vert", NULL, "../fshader.frag");

    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(p, "ModelView");
    Projection = glGetUniformLocation(p, "Projection");
}

int main(int argc, char **argv)
{
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
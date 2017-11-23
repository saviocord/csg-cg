/*
** Doing CSG with stencil
*/
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
//#include <windows.h>

enum {CSG_A, CSG_B, CSG_A_OR_B, CSG_A_AND_B, CSG_A_SUB_B, CSG_B_SUB_A};
enum {SPHERE = 1, CONE};

GLfloat rotX, rotY, rotZ;
int csg_op = CSG_B;
int x_ini,y_ini, botao;
GLfloat lightpos[] = {-25.f, 0.f, 50.f, 1.f};
static GLfloat sphere_mat[] = {1.f, .5f, 0.f, 1.f};
static GLfloat cone_mat[] = {0.f, .5f, 1.f, 1.f};
GLUquadricObj *sphere, *cone, *base;
GLfloat coneX = 0.f, coneY = 0.f, coneZ = 0.f;
GLfloat sphereX = 0.f, sphereY = 0.f, sphereZ = 0.f;

/*
** Set stencil buffer to show the part of a (front or back face)
** that's inside b's volume.
** Requirements: GL_CULL_FACE enabled, depth func GL_LESS
** Side effects: depth test, stencil func, stencil op
*/
void firstInsideSecond(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void), GLenum face, GLenum test) {
    glEnable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glCullFace(face); /* controls which face of a to use*/
    desenhaObjetoA(); /* draw a face of a into depth buffer */

    /* use stencil plane to find parts of a in b */
    glDepthMask(GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glCullFace(GL_BACK);
    desenhaObjetoB(); /* increment the stencil where the front face of b is drawn */
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    glCullFace(GL_FRONT);
    desenhaObjetoB(); /* decrement the stencil buffer where the back face of b is drawn */
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glStencilFunc(test, 0, 1);
    glDisable(GL_DEPTH_TEST);

    glCullFace(face);
    desenhaObjetoA(); /* draw the part of a that's in b */
}

void corrigeProfundidade(void(*desenhaObjetoA)(void)) {
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_ALWAYS);
    desenhaObjetoA(); /* draw the front face of a, fixing the depth buffer */
    glDepthFunc(GL_LESS);
}

/* "or" is easy; simply draw both objects with depth buffering on */
void uniao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)()) {
    glEnable(GL_DEPTH_TEST);
    desenhaObjetoA();
    desenhaObjetoB();
    glDisable(GL_DEPTH_TEST);
}

/* "and" two objects together */
void interseccao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void)) {
    firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_BACK, GL_NOTEQUAL);
    corrigeProfundidade(desenhaObjetoB);
    firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_NOTEQUAL);
    glDisable(GL_STENCIL_TEST); /* reset things */
}

/* subtract b from a */
void subtracao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void)) {
    firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_FRONT, GL_NOTEQUAL);
    corrigeProfundidade(desenhaObjetoB);
    firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_EQUAL);
    glDisable(GL_STENCIL_TEST); /* reset things */
}

void desenhaCone(void) {
    glPushMatrix();
    glColor3f(0.0, 0.0, 1.0);
    glTranslatef(coneX, coneY, coneZ);
    glTranslatef(0.f, 0.f, -30.f);
    glCallList(CONE);
    glPopMatrix();
}

void desenhaEsfera(void) {
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glTranslatef(sphereX, sphereY, sphereZ);
    glCallList(SPHERE);
    glPopMatrix();
}

void desenhaCubo(void) {

}

void desenhaCilindro(void) {

}

/* just draw single object */
void draw(void(*desenhaObjetoA)(void)) {
    glEnable(GL_DEPTH_TEST);
    desenhaObjetoA();
    glDisable(GL_DEPTH_TEST);
}

void redraw() {
    /* clear stencil each time */
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(rotX, 1, 0, 0);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(rotZ, 0, 0, 1);

    glPushMatrix();

    switch(csg_op) {
        case CSG_A:
            draw(desenhaCone);
            break;
        case CSG_B:
            draw(desenhaEsfera);
            break;
        case CSG_A_OR_B:
            uniao(desenhaCone, desenhaEsfera);
            break;
        case CSG_A_AND_B:
            interseccao(desenhaCone, desenhaEsfera);
            break;
        case CSG_A_SUB_B:
            subtracao(desenhaCone, desenhaEsfera);
            break;
        case CSG_B_SUB_A:
            subtracao(desenhaEsfera, desenhaCone);
            break;
    }
    glPopMatrix();
    glutSwapBuffers();
}

/* special keys, like array and F keys */
void specialKey(int key, int, int) {
    switch(key) {
        case GLUT_KEY_LEFT:
            rotY -= 1;
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT:
            rotY += 1;
            glutPostRedisplay();
            break;
        case GLUT_KEY_UP:
            rotX -= 1;
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN:
            rotX += 1;
            glutPostRedisplay();
            break;
    }
}

void key(unsigned char key, int, int) {
    switch(key) {
        case 'a':
            rotZ -= 1;
            glutPostRedisplay();
            break;
        case 's':
            rotZ += 1;
            glutPostRedisplay();
            break;
        case 'q':
            exit(0);
    }
}

void menu(int csgop) {
    csg_op = csgop;
    glutPostRedisplay();
}

void menuCallback(void){
    glutCreateMenu(menu);
    glutAddMenuEntry("A only", CSG_A);
    glutAddMenuEntry("B only", CSG_B);
    glutAddMenuEntry("A or B", CSG_A_OR_B);
    glutAddMenuEntry("A and B", CSG_A_AND_B);
    glutAddMenuEntry("A sub B", CSG_A_SUB_B);
    glutAddMenuEntry("B sub A", CSG_B_SUB_A);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void  mouse(int key, int state, int x, int y) {

}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutInitDisplayMode(GLUT_STENCIL|GLUT_DEPTH|GLUT_DOUBLE);
    (void)glutCreateWindow("CSG");
    glClearColor(1, 1, 1, 0);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-50., 50., -50., 50., -50., 50.);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_COLOR_MATERIAL);

    glutDisplayFunc(redraw);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouse);
    menuCallback();

    glNewList(SPHERE, GL_COMPILE);
    sphere = gluNewQuadric();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere_mat);
    gluSphere(sphere, 20.f, 64, 64);
    gluDeleteQuadric(sphere);
    glEndList();

    glNewList(CONE, GL_COMPILE);
    cone = gluNewQuadric();
    base = gluNewQuadric();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cone_mat);
    gluQuadricOrientation(base, GLU_INSIDE);
    gluDisk(base, 0., 15., 64, 1);
    gluCylinder(cone, 15., 0., 60., 64, 64);
    gluDeleteQuadric(cone);
    gluDeleteQuadric(base);
    glEndList();

    glutMainLoop();

    return 0;
}
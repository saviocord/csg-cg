/*
** Doing CSG with stencil
*/
#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>

enum {CSG_A, CSG_B, CSG_C,
      CSG_A_U_B, CSG_A_U_C, CSG_B_U_C,
      CSG_A_I_B, CSG_A_I_C, CSG_B_I_C,
      CSG_A_S_B, CSG_B_S_A,
      CSG_A_S_C, CSG_C_S_A,
      CSG_B_S_C, CSG_C_S_B};

GLfloat rotX, rotY, rotZ;
int csg_op = CSG_B_I_C;
GLfloat lightpos[] = {-25.f, 0.f, 50.f, 1.f};

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
    glDisable(GL_STENCIL_TEST);
}

void corrigeProfundidade(void(*desenhaObjetoA)(void)) {
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    desenhaObjetoA();
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
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
}

/* subtract b from a */
void subtracao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void)) {
    firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_FRONT, GL_NOTEQUAL);
    corrigeProfundidade(desenhaObjetoB);
    firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_EQUAL);
}

void desenhaCone(void) {
    glPushMatrix();
    glColor3f(0.0, 1.0, 0.0);
    glTranslatef(-3, 0, 0);
    glRotatef(90, 0, 1, 0);
    glutSolidCone(2.5, 8, 40, 40);
    glPopMatrix();
}

void desenhaEsfera(void) {
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glTranslatef(0, 0, 0);
    glutSolidSphere(2.5, 40, 40);
    glPopMatrix();
}

void desenhaCubo(void) {
    glPushMatrix();
    glColor3f(0.0, 0.0, 1.0);
    glTranslatef(0, 0, 0);
    glutSolidCube(4);
    glPopMatrix();
}

void desenhaCilindro(void) {

}


void drawBitmapText(char *string,float x,float y,float z)
{
    glPushMatrix();
    glLoadIdentity();
    char *c;
    glRasterPos3f(x, y,z);

    for (c=string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
    glPopMatrix();
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

    //glPushMatrix();

    switch(csg_op) {
        case CSG_A:
            draw(desenhaCone);
            drawBitmapText("Objeto A",-2,9,0);
            break;
        case CSG_B:
            draw(desenhaEsfera);
            drawBitmapText("Objeto B",-2,9,0);
            break;
        case CSG_C:
            draw(desenhaCubo);
            drawBitmapText("Objeto C",-2,9,0);
            break;
        case CSG_A_U_B:
            uniao(desenhaCone, desenhaEsfera);
            drawBitmapText("A uniao B",-2,9,0);
            break;
        case CSG_A_U_C:
            uniao(desenhaCone, desenhaCubo);
            drawBitmapText("A uniao C",-2,9,0);
            break;
        case CSG_B_U_C:
            uniao(desenhaEsfera, desenhaCubo);
            drawBitmapText("B uniao C",-2,9,0);
            break;
        case CSG_A_I_B:
            interseccao(desenhaCone, desenhaEsfera);
            drawBitmapText("A interseccao B",-3,9,0);
            break;
        case CSG_A_I_C:
            interseccao(desenhaCone, desenhaCubo);
            drawBitmapText("A interseccao C",-3,9,0);
            break;
        case CSG_B_I_C:
            interseccao(desenhaEsfera, desenhaCubo);
            drawBitmapText("B interseccao C",-3,9,0);
            break;
        case CSG_A_S_B:
            subtracao(desenhaCone, desenhaEsfera);
            drawBitmapText("A subtracao B",-2,9,0);
            break;
        case CSG_B_S_A:
            subtracao(desenhaEsfera, desenhaCone);
            drawBitmapText("B subtracao A",-2,9,0);
            break;
        case CSG_A_S_C:
            subtracao(desenhaCone, desenhaCubo);
            drawBitmapText("A subtracao C",-2,9,0);
            break;
        case CSG_C_S_A:
            subtracao(desenhaCubo, desenhaCone);
            drawBitmapText("C subtracao A",-2,9,0);
            break;
        case CSG_B_S_C:
            subtracao(desenhaEsfera, desenhaCubo);
            drawBitmapText("B subtracao C",-2,9,0);
            break;
        case CSG_C_S_B:
            subtracao(desenhaCubo, desenhaEsfera);
            drawBitmapText("C subtracao B",-2,9,0);
            break;
    }
    //glPopMatrix();
    //drawBitmapText("todos os direitos reservados",-5,-9,0); //kkkkk
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
        case 'q':
            exit(0);
        case '-':
            rotZ -= 1;
            glutPostRedisplay();
            break;
        case '+':
            rotZ += 1;
            glutPostRedisplay();
            break;
        case 'a':
            rotY -= 1;
            glutPostRedisplay();
            break;
        case 'd':
            rotY += 1;
            glutPostRedisplay();
            break;
        case 'w':
            rotX -= 1;
            glutPostRedisplay();
            break;
        case 's':
            rotX += 1;
            glutPostRedisplay();
            break;
    }
}

void menu(int csgop) {
    csg_op = csgop;
    glutPostRedisplay();
}

void menuCallback(void){


    glutCreateMenu(menu);
    glutAddMenuEntry("Apenas A", CSG_A);
    glutAddMenuEntry("Apenas B", CSG_B);
    glutAddMenuEntry("Apenas C", CSG_C);

    glutAddMenuEntry("A Uniao B", CSG_A_U_B);
    glutAddMenuEntry("A Uniao C", CSG_A_U_C);
    glutAddMenuEntry("B Uniao C", CSG_B_U_C);

    glutAddMenuEntry("A interseccao B", CSG_A_I_B);
    glutAddMenuEntry("A interseccao C", CSG_A_I_C);
    glutAddMenuEntry("B interseccao C", CSG_B_I_C);

    glutAddMenuEntry("A subtracao B", CSG_A_S_B);
    glutAddMenuEntry("B subtracao A", CSG_B_S_A);

    glutAddMenuEntry("A subtracao C", CSG_A_S_C);
    glutAddMenuEntry("C subtracao A", CSG_C_S_A);

    glutAddMenuEntry("B subtracao C", CSG_B_S_C);
    glutAddMenuEntry("C subtracao B", CSG_C_S_B);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void  mouse(int key, int state, int x, int y) {

}

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_STENCIL|GLUT_DEPTH|GLUT_DOUBLE);
    glutCreateWindow("CSG");
    glClearColor(1, 1, 1, 0);

    glMatrixMode(GL_PROJECTION);
    glOrtho(-10, 10, -10, 10, -10, 10);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(redraw);
    glutKeyboardFunc(key);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouse);
    menuCallback();

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_COLOR_MATERIAL);

    glutMainLoop();

    return 0;
}

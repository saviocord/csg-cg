/*
** Doing CSG with stencil
*/
#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>


enum {CSG_A, CSG_B, CSG_C, CSG_D,
    CSG_A_U_B, CSG_A_I_B,
    CSG_A_S_B, CSG_B_S_A,
    CSG_HELP};

int ObjetoA = -1, ObjetoB = -1;
GLfloat rotX, rotY, rotZ;
int csg_op = CSG_A;
GLfloat lightpos[] = {-25.f, 0.f, 50.f, 1.f};

/*
** Seta o stencil buffer para mostrar a parte do objetoA (face da frente e de tras)
** que esta dentro do volume do objetoB
*/
void firstInsideSecond(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void), GLenum face, GLenum test) {
    glEnable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glCullFace(face); /* controla qual face do objetoA sera usada*/
    desenhaObjetoA(); /* desenha a face do objetoA no depth buffer */

    /* usar stencil plane para encontrar partes do objetoA no objetoB*/
    glDepthMask(GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glCullFace(GL_BACK);
    desenhaObjetoB(); /* incrementa o stencil onde a face da frente do objetoB é desenhada*/
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    glCullFace(GL_FRONT);
    desenhaObjetoB(); /* decrementa o stencil buffer onde a face de tras do objetoB é desenhada*/
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glStencilFunc(test, 0, 1);
    glDisable(GL_DEPTH_TEST);

    glCullFace(face);
    desenhaObjetoA(); /* desenha a parte do objetoA que esta no objetoB*/
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

/* simplesmente desenha os dois objetos com o depth buffering*/
void uniao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)()) {
    glEnable(GL_DEPTH_TEST);
    desenhaObjetoA();
    desenhaObjetoB();
    glDisable(GL_DEPTH_TEST);
}

/* junta dois objetos*/
void interseccao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void)) {
    firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_BACK, GL_NOTEQUAL);
    corrigeProfundidade(desenhaObjetoB);
    firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_NOTEQUAL);
}

/* subtrai o objetoB do objetoA */
void subtracao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void)) {
    firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_FRONT, GL_NOTEQUAL);
    corrigeProfundidade(desenhaObjetoB);
    firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_EQUAL);
}

/*OBJETO A*/
void desenhaCone(void) {
    glPushMatrix();
    glColor3f(0.0, 1.0, 0.0);
    glTranslatef(-3, 0, 0);
    glRotatef(90, 0, 1, 0);
    glutSolidCone(2.5, 8, 40, 40);
    glPopMatrix();
}

/*OBJETO B*/
void desenhaEsfera(void) {
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glTranslatef(0, 0, 0);
    glutSolidSphere(2.5, 40, 40);
    glPopMatrix();
}

/*OBJETO C*/
void desenhaCubo(void) {
    glPushMatrix();
    glColor3f(0.0, 0.0, 1.0);
    glTranslatef(0, 0, 0);
    glutSolidCube(4);
    glPopMatrix();
}

/*OBJETO D*/
void desenhaCilindro(void) {
    glPushMatrix();
    glColor3f(1.0, 1.0, 0.0);
    glTranslatef(0, 3, 0);
    glRotatef(90, 1, 0, 0);
    glutSolidCylinder(1.2, 5, 60, 60);
    glPopMatrix();
}

/*Desenha o texto*/
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

/* desenha um unico objeto */
void draw(void(*desenhaObjetoA)(void)) {
    glEnable(GL_DEPTH_TEST);
    desenhaObjetoA();
    glDisable(GL_DEPTH_TEST);
}

void redraw() {
    /* limpa o stencil toda vez*/
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(rotX, 1, 0, 0);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(rotZ, 0, 0, 1);

    switch(csg_op) {
        case CSG_HELP:
            drawBitmapText("Para Selecionar o Objeto A Precione F1",-7,0,0);
            drawBitmapText("Para Selecionar o Objeto B Precione F2",-7,-1,0);
            drawBitmapText("Cone Precione 1",-7,-2,0);
            drawBitmapText("Esfera Precione 2",-7,-3,0);
            drawBitmapText("Cubo Precione 3",-7,-4,0);
            drawBitmapText("Cilindro Precione 4",-7,-5,0);
            break;
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
        case CSG_D:
            draw(desenhaCilindro);
            drawBitmapText("Objeto D",-2,9,0);
            break;
        case CSG_A_U_B:
            if((ObjetoA == 0)&&(ObjetoB == 1)){
                uniao(desenhaCone, desenhaEsfera);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 2)){
                uniao(desenhaCone, desenhaCubo);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 3)){
                uniao(desenhaCone, desenhaCilindro);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 2)){
                uniao(desenhaEsfera, desenhaCubo);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 3)){
                uniao(desenhaEsfera, desenhaCilindro);
            }
            else if ((ObjetoA == 2)&&(ObjetoB == 3)){
                uniao(desenhaCubo, desenhaCilindro);
            }
            else{
               drawBitmapText("Objetos A e B nao foram Selecionados",-5,0,0);
            }
            drawBitmapText("A uniao B",-2,9,0);
            break;
        case CSG_A_I_B:
            if((ObjetoA == 0)&&(ObjetoB == 1)){
                interseccao(desenhaCone, desenhaEsfera);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 2)){
                interseccao(desenhaCone, desenhaCubo);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 3)){
                interseccao(desenhaCone, desenhaCilindro);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 2)){
                interseccao(desenhaEsfera, desenhaCubo);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 3)){
                interseccao(desenhaEsfera, desenhaCilindro);
            }
            else if ((ObjetoA == 2)&&(ObjetoB == 3)){
                interseccao(desenhaCubo, desenhaCilindro);
            }
            else{
               drawBitmapText("Objetos A e B nao foram Selecionados",-5,0,0);
            }
            drawBitmapText("A interseccao B",-3,9,0);
            break;
        case CSG_A_S_B:
            if((ObjetoA == 0)&&(ObjetoB == 1)){
                subtracao(desenhaCone, desenhaEsfera);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 2)){
                subtracao(desenhaCone, desenhaCubo);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 3)){
                subtracao(desenhaCone, desenhaCilindro);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 2)){
                subtracao(desenhaEsfera, desenhaCubo);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 3)){
                subtracao(desenhaEsfera, desenhaCilindro);
            }
            else if ((ObjetoA == 2)&&(ObjetoB == 3)){
                subtracao(desenhaCubo, desenhaCilindro);
            }
            else{
               drawBitmapText("Objetos A e B nao foram Selecionados",-5,0,0);
            }
            drawBitmapText("A subtracao B",-2,9,0);
            break;
        case CSG_B_S_A:
            if((ObjetoA == 0)&&(ObjetoB == 1)){
                subtracao(desenhaEsfera, desenhaCone);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 2)){
                subtracao(desenhaCubo, desenhaCone);
            }
            else if ((ObjetoA == 0)&&(ObjetoB == 3)){
                subtracao(desenhaCilindro, desenhaCone);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 2)){
                subtracao(desenhaCubo, desenhaEsfera);
            }
            else if ((ObjetoA == 1)&&(ObjetoB == 3)){
                subtracao(desenhaCilindro, desenhaEsfera);
            }
            else if ((ObjetoA == 2)&&(ObjetoB == 3)){
                subtracao(desenhaCilindro, desenhaCubo);
            }
            else{
               drawBitmapText("Objetos A e B nao foram Selecionados",-5,0,0);
            }
            drawBitmapText("B subtracao A",-2,9,0);
            break;
    }

    if(ObjetoA == 0){
       drawBitmapText("ObjetoA = Cone",-10,-8,0);
    }
    else if(ObjetoA == 1){
       drawBitmapText("ObjetoA = Esfera",-10,-8,0);
    }
    else if(ObjetoA == 2){
       drawBitmapText("ObjetoA = Cubo",-10,-8,0);
    }
    else if(ObjetoA == 3){
       drawBitmapText("ObjetoA = Cilindro",-10,-8,0);
    }

    if(ObjetoB == 0){
       drawBitmapText("ObjetoB = Cone",-10,-9,0);
    }
    else if(ObjetoB == 1){
       drawBitmapText("ObjetoB = Esfera",-10,-9,0);
    }
    else if(ObjetoB == 2){
       drawBitmapText("ObjetoB = Cubo",-10,-9,0);
    }
    else if(ObjetoB == 3){
       drawBitmapText("ObjetoB = Cilindro",-10,-9,0);
    }

    //drawBitmapText("todos os direitos reservados",-5,-9,0); //kkkkk
    glutSwapBuffers();
}

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
        case GLUT_KEY_F1:
            ObjetoA = csg_op;
            redraw();
            break;
        case GLUT_KEY_F2:
            ObjetoB = csg_op;
            redraw();
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
        case '1':
            csg_op = 0;
            redraw();
            break;
        case '2':
            csg_op = 1;
            redraw();
            break;
        case '3':
            csg_op = 2;
            redraw();
            break;
        case '4':
            csg_op = 3;
            redraw();
            break;
    }
}

void menu(int csgop) {
    csg_op = csgop;
    glutPostRedisplay();
}

void menuCallback(void){

    glutCreateMenu(menu);
    glutAddMenuEntry("Help", CSG_HELP);
    glutAddMenuEntry("Apenas A", CSG_A);
    glutAddMenuEntry("Apenas B", CSG_B);
    glutAddMenuEntry("Apenas C", CSG_C);
    glutAddMenuEntry("Apenas D", CSG_D);
    glutAddMenuEntry("A Uniao B", CSG_A_U_B);
    glutAddMenuEntry("A interseccao B", CSG_A_I_B);
    glutAddMenuEntry("A subtracao B", CSG_A_S_B);
    glutAddMenuEntry("B subtracao A", CSG_B_S_A);

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

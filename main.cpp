/*
** Doing CSG with stencil
*/
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
//#include <windows.h>

enum {CSG_A, CSG_B, CSG_A_OR_B, CSG_A_AND_B, CSG_A_SUB_B, CSG_B_SUB_A};


/* just draw single object */
void draw(void(*desenhaObjetoA)(void))
{
  glEnable(GL_DEPTH_TEST);
  desenhaObjetoA();
  glDisable(GL_DEPTH_TEST);
}

/*
** Set stencil buffer to show the part of a (front or back face)
** that's inside b's volume.
** Requirements: GL_CULL_FACE enabled, depth func GL_LESS
** Side effects: depth test, stencil func, stencil op
*/
void firstInsideSecond(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void), GLenum face, GLenum test)
{
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

void corrigeProfundidade(void(*desenhaObjetoA)(void))
{
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glDepthFunc(GL_ALWAYS);
  desenhaObjetoA(); /* draw the front face of a, fixing the depth buffer */
  glDepthFunc(GL_LESS);
}

/* "or" is easy; simply draw both objects with depth buffering on */
void uniao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)())
{
  glEnable(GL_DEPTH_TEST);
  desenhaObjetoA();
  desenhaObjetoB();
  glDisable(GL_DEPTH_TEST);
}

/* "and" two objects together */
void interseccao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void))
{
  firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_BACK, GL_NOTEQUAL);
  corrigeProfundidade(desenhaObjetoB);
  firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_NOTEQUAL);
  glDisable(GL_STENCIL_TEST); /* reset things */
}

/* subtract b from a */
void subtracao(void(*desenhaObjetoA)(void), void(*desenhaObjetoB)(void))
{
  firstInsideSecond(desenhaObjetoA, desenhaObjetoB, GL_FRONT, GL_NOTEQUAL);
  corrigeProfundidade(desenhaObjetoB);
  firstInsideSecond(desenhaObjetoB, desenhaObjetoA, GL_BACK, GL_EQUAL);
  glDisable(GL_STENCIL_TEST); /* reset things */
}

enum {SPHERE = 1, CONE};

/* Draw a cone */
GLfloat coneX = 0.f, coneY = 0.f, coneZ = 0.f;
void desenhaCone(void)
{
  glPushMatrix();
  glTranslatef(coneX, coneY, coneZ);
  glTranslatef(0.f, 0.f, -30.f);
  glCallList(CONE);
  glPopMatrix();
}

/* Draw a sphere */
GLfloat sphereX = 0.f, sphereY = 0.f, sphereZ = 0.f;
void desenhaEsfera(void)
{
  glPushMatrix();
  glTranslatef(sphereX, sphereY, sphereZ);
  glCallList(SPHERE);
  glPopMatrix();

}

int csg_op = CSG_A_OR_B;

/* add menu callback */

void menu(int csgop)
{
  csg_op = csgop;
  glutPostRedisplay();
}

GLfloat viewangle;

void redraw()
{
  /* clear stencil each time */
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    glPushMatrix();
    glRotatef(viewangle, 0.f, 1.f, 0.f);

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

/* animate scene by rotating */
enum {ANIM_LEFT, ANIM_RIGHT};
int animDirection = ANIM_LEFT;

void anim(void)
{
  if(animDirection == ANIM_LEFT)
    viewangle -= 3.f;
  else
    viewangle += 3.f;
  glutPostRedisplay();
}

/* special keys, like array and F keys */
void special(int key, int x, int y)
{
  switch(key) {
  case GLUT_KEY_LEFT:
    glutIdleFunc(anim);
    animDirection = ANIM_LEFT;
    break;
  case GLUT_KEY_RIGHT:
    glutIdleFunc(anim);
    animDirection = ANIM_RIGHT;
    break;
  case GLUT_KEY_UP:
  case GLUT_KEY_DOWN:
    glutIdleFunc(0);
    break;
  }
}

void key(unsigned char key, int x, int y)
{
  switch(key) {
  case 'a':
    viewangle -= 10.f;
    glutPostRedisplay();
    break;
  case 's':
    viewangle += 10.f;
    glutPostRedisplay();
    break;
  case '\033':
    exit(0);
  }
}


int picked_object;
int xpos = 0, ypos = 0;
int newxpos, newypos;
int startx, starty;

void
mouse(int button, int state, int x, int y)
{
  if(state == GLUT_UP) {
      picked_object = button;
      xpos += newxpos;
      ypos += newypos;
      newxpos = 0;
      newypos = 0;
  } else { /* GLUT_DOWN */
    startx = x;
    starty = y;
  }
}

#define DEGTORAD (2 * 3.1415 / 360)
void motion(int x, int y)
{
  GLfloat r, objx, objy, objz;

  newxpos = x - startx;
  newypos = starty - y;

  r = (newxpos + xpos) * 50.f/512.f;
  objx = r * (float)cos(viewangle * DEGTORAD);
  objy = (newypos + ypos) * 50.f/512.f;
  objz = r * (float)sin(viewangle * DEGTORAD);

  switch(picked_object) {
  case CSG_A:
    coneX = objx;
    coneY = objy;
    coneZ = objz;
    break;
  case CSG_B:
    sphereX = objx;
    sphereY = objy;
    sphereZ = objz;
    break;
  }
  glutPostRedisplay();
}

int
main(int argc, char **argv)
{
    static GLfloat lightpos[] = {25.f, 50.f, -50.f, 1.f};
    static GLfloat sphere_mat[] = {1.f, .5f, 0.f, 1.f};
    static GLfloat cone_mat[] = {0.f, .5f, 1.f, 1.f};
    GLUquadricObj *sphere, *cone, *base;

    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_STENCIL|GLUT_DEPTH|GLUT_DOUBLE);
    (void)glutCreateWindow("csg");
    glutDisplayFunc(redraw);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutCreateMenu(menu);
    glutAddMenuEntry("A only", CSG_A);
    glutAddMenuEntry("B only", CSG_B);
    glutAddMenuEntry("A or B", CSG_A_OR_B);
    glutAddMenuEntry("A and B", CSG_A_AND_B);
    glutAddMenuEntry("A sub B", CSG_A_SUB_B);
    glutAddMenuEntry("B sub A", CSG_B_SUB_A);
    glutAttachMenu(GLUT_RIGHT_BUTTON);


    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);


    /* make display lists for sphere and cone; for efficiency */

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

    glMatrixMode(GL_PROJECTION);
    glOrtho(-50., 50., -50., 50., -50., 50.);
    glMatrixMode(GL_MODELVIEW);
    glutMainLoop();

    return 0;
}

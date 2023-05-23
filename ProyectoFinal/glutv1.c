#if WIN32
#include <windows.h>
#include <GL/glut.h>
#endif
#if __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

GLfloat girax = 0, giray = 0;
GLboolean malla = GL_FALSE, ejes = GL_TRUE;
// variable que guarde un vector de 3 colores
GLfloat rojo[3] = {0.9, 0.0, 0.0};
GLfloat negro[3] = {0.0, 0.0, 0.0};
GLfloat azul[3] = {0, 0, 1};
GLboolean iniciando = GL_TRUE;
float giro = 0;
float valorX;
float valorY;
float valorZ;

// Rotacion XY y Zoom
void mover(void)
{
    glRotatef(giray, 0.0, 1.0, 0.0);
    glRotatef(girax, 1.0, 0.0, 0.0);
}

// Crear malla
void creaMalla(int long_eje)
{
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    int i;
    for (i = -long_eje; i <= long_eje; i++)
    {
        glVertex3f(i, 0, -long_eje);
        glVertex3f(i, 0, long_eje);
        glVertex3f(-long_eje, 0, i);
        glVertex3f(long_eje, 0, i);
    }
    glEnd();
}

// Crear ejes
void creaEjes(void)
{
    glColor3fv(negro);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(11.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 11.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 11.0);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3f(11.0, 0.0, 0.0); /* eje x */
    glVertex3f(10.5, 0.0, -.50);
    glVertex3f(10.5, 0.0, .50);
    glColor3f(0.25, 1, 0.25); /* eje y */
    glVertex3f(0.0, 11.0, 0.0);
    glVertex3f(-.50, 10.5, 0.0);
    glVertex3f(.50, 10.5, 0.0);
    glColor3f(0.25, 0.25, 1.0); /* eje z */
    glVertex3f(0.0, 0.0, 11.0);
    glVertex3f(-.50, 0.0, 10.5);
    glVertex3f(.50, 0.0, 10.5);
    glEnd();
}

void dibuja(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotatef(giray, 0.0, 1.0, 0.0);
    glRotatef(girax, 1.0, 0.0, 0.0);
    if (malla)
        creaMalla(10);
    if (ejes)
        creaEjes();

    // Dibuja aqui

    glPopMatrix();

    glutSwapBuffers();
}

void anima(int v)
{
    FILE *file = fopen("values.csv", "r");
    if (file == NULL)
    {
        printf("Failed to open the file.\n");
    }

    while (!feof(file))
    {
        fscanf(file, "%f,%f,%f", &valorX, &valorY, &valorZ);                    // Lee los 3 valores numericos separados por comas
        printf("valorX: %f, valorY: %f, valorZ: %f\n", valorX, valorY, valorZ); // Imprime los valores
    }
    fclose(file);

    if (iniciando)
    {
        if (giro < 180)
        {
            giro += 5;
        }
        else
        {
            iniciando = GL_FALSE;
        }
    }
    else
    {
        if (giro > 0)
        {
            giro -= 5;
        }
        else
        {
            iniciando = GL_TRUE;
        }
    }
    glutTimerFunc(70, anima, 0);
    glutPostRedisplay();
}

void teclado(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
    case 'm': // activa/desactiva la malla
        malla = !malla;
        break;
    case 'e': // activa/desactiva los ejes
        ejes = !ejes;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void rotar(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_LEFT: // rotacion en el eje Y
        giray -= 15;
        break;
    case GLUT_KEY_RIGHT: // rotacion en el eje Y
        giray += 15;
        break;
    case GLUT_KEY_UP: // rotacion en el eje X
        girax -= 15;
        break;
    case GLUT_KEY_DOWN: // rotacion en el eje X
        girax += 15;
        break;
    }
    glutPostRedisplay();
}

void ajusta(int ancho, int alto)
{
    glClearColor(1.0, 1.0, 1.0, 0.0); // color de fondo

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-15, 15, -10 + 5, 10 + 5, -15, 15);

    // Habilitar iluminacion y color de los materiales
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glMatrixMode(GL_MODELVIEW); // matriz de modelado
    glLoadIdentity();           // matriz identidad
    glEnable(GL_DEPTH_TEST);    // activa el Z-buffer
}

int main(int argc, char **argv)
{
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    float x = 800;
    glutInitWindowSize(x * (3 / 2), x); // Relacion 3:2
    glutInit(&argc, argv);
    glutCreateWindow("GLUT");
    glutDisplayFunc(dibuja);
    glutReshapeFunc(ajusta);
    glutKeyboardFunc(teclado);
    glutTimerFunc(1, anima, 0);
    glutSpecialFunc(rotar);
    glutMainLoop();
    return 0;
}

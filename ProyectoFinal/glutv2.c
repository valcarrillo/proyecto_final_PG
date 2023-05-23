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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>


#ifdef _WIN32
#define drand48() ((float)rand()/RAND_MAX)
#endif

#define PS_GRAVITY -5
//Propiedades luz
GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
   GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat position[] = {0.0, 3.0, 3.0, 0.0};
   GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
   GLfloat local_view[] = {0.0};

typedef struct {
    float x, y, z;
    float radius;
} PSsphere;

typedef struct {
    float position[3];			/* current position */
    float previous[3];			/* previous position */
    float velocity[3];			/* velocity (magnitude & direction) */
    float dampening;			/* % of energy lost on collision */
    int alive;				/* is this particle alive? */
} PSparticle;


PSparticle* particles = NULL;
PSsphere    sphere = { 0, 1, 0,1.25 };
int num_particles = 5000;
int points = 1;
int frame_rate = 1;
float frame_time = 0;
float flow = 500;
float slow_down = 1;
GLfloat girax = 0, giray= 0, zoom=0, movimiento=0, movimiento_x=0;
int x=0;
int cambio=0;

float
timedelta(void)
{
    static long begin = 0;
    static long finish, difference;

#if defined(_WIN32)
#include <sys/timeb.h>
    static struct timeb tb;

    ftime(&tb);
    finish = tb.time*1000+tb.millitm;
#else
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
    static struct tms tb;

    finish = times(&tb);
#endif

    difference = finish - begin;
    begin = finish;

    return (float)difference/(float)1000;  /* CLK_TCK=1000 */
}


   
void
text(int x, int y, char* s) 
{
    int lines;
    char* p;

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 
	    0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3ub(0, 0, 0);
    glRasterPos2i(x+1, y-1);
    for(p = s, lines = 0; *p; p++) {
	if (*p == '\n') {
	    lines++;
	    glRasterPos2i(x+1, y-1-(lines*18));
	}
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glColor3ub(128, 0, 255);
    glRasterPos2i(x, y);
    for(p = s, lines = 0; *p; p++) {
	if (*p == '\n') {
	    lines++;
	    glRasterPos2i(x, y-(lines*18));
	}
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}



void
psTimeStep(PSparticle* p, float dt)
{
    if (p->alive == 0)
	return;

    p->velocity[0] += 0;
    p->velocity[1] += PS_GRAVITY*dt;
    p->velocity[2] += 0;

    p->previous[0] = p->position[0];
    p->previous[1] = p->position[1];
    p->previous[2] = p->position[2];

    p->position[0] += p->velocity[0]*dt;
    p->position[1] += p->velocity[1]*dt;
    p->position[2] += p->velocity[2]*dt;
}


void
psNewParticle(PSparticle* p, float dt)
{
	p->velocity[0] = 1*(drand48()-0.5);
	p->velocity[1] = 0;
	p->velocity[2] = 0.5*(drand48()-0.0);
	//lluvia
	p->position[0] = 0;
	p->position[1] = 15;
	p->position[2] = 0;
	p->previous[0] = p->position[0];
	p->previous[1] = p->position[1];
	p->previous[2] = p->position[2];
	p->dampening = 0.45*drand48();
	p->alive = 1;

    psTimeStep(p, 2*dt*drand48());
}



void
psCollide(PSparticle* p)
{
    float vx = p->position[0] - sphere.x;
    float vy = p->position[1] - sphere.y;
    float vz = p->position[2] - sphere.z;
    float distance;

    if (p->alive == 0)
	return;

    distance = sqrt(vx*vx + vy*vy + vz*vz);

    if (distance < sphere.radius) {
#if 0
	vx /= distance;  vy /= distance;  vz /= distance;
	d = 2*(-vx*p->velocity[0] + -vy*p->velocity[1] + -vz*p->velocity[2]);
	p->velocity[0] += vx*d*2;
	p->velocity[1] += vy*d*2;
	p->velocity[2] += vz*d*2;
	d = sqrt(p->velocity[0]*p->velocity[0] + 
		 p->velocity[1]*p->velocity[1] +
		 p->velocity[2]*p->velocity[2]);
	p->velocity[0] /= d;
	p->velocity[1] /= d;
	p->velocity[2] /= d;
#else
	p->position[0] = sphere.x+(vx/distance)*sphere.radius;
	p->position[1] = sphere.y+(vy/distance)*sphere.radius;
	p->position[2] = sphere.z+(vz/distance)*sphere.radius;
	p->previous[0] = p->position[0];
	p->previous[1] = p->position[1];
	p->previous[2] = p->position[2];
	p->velocity[0] = vx/distance;
	p->velocity[1] = vy/distance;
	p->velocity[2] = vz/distance;
#endif
    }
}


void
reshape(int width, int height)
{
	glClearColor(0.89,1.0,1.0,0.0);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 1, 3, 0, 1, 0, 0, 1, 0);
     glEnable(GL_COLOR_MATERIAL);
     glPointSize(2);
    glEnable(GL_POINT_SMOOTH);
    	glEnable(GL_DEPTH_TEST);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
   glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_AUTO_NORMAL);
   glEnable(GL_NORMALIZE);
   glFrontFace(GL_CW);  
   glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    timedelta();
}

// Rotacion XY y Zoom
void mover (void) {
	glRotated( giray, 0.0, 1.0,0.0);
	glRotated( girax, 1.0, 0.0, 0.0);
	glTranslatef( 0.0,0.0,movimiento);
}



void billy(){
	//Cilindros
	GLUquadricObj *qobj;
	qobj = gluNewQuadric ();
	gluQuadricDrawStyle(qobj,GLU_FILL);
	glPushMatrix();	
		glPushMatrix();
	//glutSolidSphere(sphere.radius, 20, 20);
	glPopMatrix();
	glScalef(0.1,0.1,0.1);
	glTranslatef(0,10,0);
		glPushMatrix();
			glRotatef(90,1,0,0);
			glScalef(3,0.8,0.1);
			glTranslatef(0,-1,50);
			glColor3ub(75,54,33);
			glutSolidCube(7);
		glPopMatrix();
		glPushMatrix();
				glColor3ub(161,130,98);
				glRotatef(90,1,0,0);
				glTranslatef(9.5,0.0,-30);
				gluCylinder(qobj, 0.2, 0.2,35, 20, 20);
		glPopMatrix();
		glPushMatrix();
				glColor3ub(161,130,98);
				glRotatef(90,1,0,0);
				glTranslatef(-9.5,0.0,-30);
				gluCylinder(qobj, 0.2, 0.2,35, 20, 20);
		glPopMatrix();
		//Billy
		glPushMatrix();
			glScalef(1.5,1,1.5);
			glTranslatef(0,5,0);
			glColor3ub(255,200,39);
			glutSolidCube(5.0); 
			//ojos
			glPushMatrix();
				glColor3ub(0,0,0);
				glScalef(0.7,0.7,1);
				glTranslatef(0,0.5,2.6);
				//derecho
				glPushMatrix();
					glTranslatef(2.0,0,0);
					glutSolidSphere(0.5,20,20);
				glPopMatrix();
				//izquierdo
				glPushMatrix();
					glTranslatef(-2.0,0,0);
					glutSolidSphere(0.5,20,20);
				glPopMatrix();
			glPopMatrix();
			//nariz
			glPushMatrix();
				glColor3ub(0,0,0);
				glScalef(0.7,0.7,1);
				glTranslatef(0,-1.0,2.6);
				glutSolidCube(0.7); 
			glPopMatrix();
			// boca
			glPushMatrix();//boca
					glColor3ub(0,0,0);
					glTranslatef(0.2,-.2,0);
					glRotatef(5,0,0,1);
					glLineWidth(3);
					glBegin(GL_POLYGON);
						glVertex3f(-1.2,-1,2.7);
						glVertex3f(-1.1,-1.4,2.7);
						glVertex3f(-0.9,-1.6,2.7);
						glVertex3f(-0.7,-1.6,2.7);
						glVertex3f(-0.2,-1.4,2.7);
					glEnd();
			glPopMatrix();
			//Caja izq
			glPushMatrix();
			glScalef(1,2,2.0);
			glTranslatef(3,0,0);
			glColor3ub(255,0,0);
			glutSolidCube(1.0); 
			glPopMatrix();
			//Caja der
			glPushMatrix();
			glScalef(1,2,2.0);
			glTranslatef(-3,0,0);
			glColor3ub(255,0,0);
			glutSolidCube(1.0); 
			glPopMatrix();
		glPopMatrix();
		//Antena
		glPushMatrix();
			glColor3ub(56,73,85);
			glRotatef(90,1,0,0);
			glTranslatef(0.0,0.0,-12);
			gluCylinder(qobj, 0.2, 0.2, 5, 20, 20);
		glPopMatrix();
		//Cubo antena
		glPushMatrix();
			glColor3ub(5,65,131);
			glTranslatef(0,12,0);
			glutSolidCube(1.5); 
		glPopMatrix();
		//Cuello
		glPushMatrix();
			glColor3ub(56,73,85);
			glRotatef(90,1,0,0);
			glTranslatef(0.0,0.0,-3);
			gluCylinder(qobj, 0.3, 0.3, 2, 20, 20);
		glPopMatrix();
		//Cuerpo
		glPushMatrix();
			glScalef(1.5,1,1.5);
			glTranslatef(0,-1,0);
			glColor3ub(255,200,39);
			glutSolidCube(4.6); 
			glPushMatrix();
				glScalef(1.5,1,1.5);
				glTranslatef(0,-2.5,0);
				glColor3ub(255,0,0);
				glutSolidSphere(1.3,20,20);
			glPopMatrix();
			glPushMatrix();
				glScalef(0.6,1,2);
				glTranslatef(0,0,0.5);
				glColor3ub(8,172,92);
				glRotatef(45,0,0,1);
				glutSolidCube(1.5);
			glPopMatrix();
			
			//Hombros
			//Caja izq
			glPushMatrix();
				glScalef(1,2,1.5);
				glRotatef(90,1,0,0);
				glTranslatef(2.5,-0.5,-0.5);
				glColor3ub(153,118,25);
				glutSolidCube(1.0); 
			glPopMatrix();
			//Caja der
			glPushMatrix();
				glScalef(1,2,1.5);
				glRotatef(90,1,0,0);
				glTranslatef(-2.5,-0.5,-0.5);
				glColor3ub(153,118,25);
				glutSolidCube(1.0); 
			glPopMatrix(); 
		glPopMatrix();
		//Piernas
		glPushMatrix();
			glPushMatrix();
				glColor3ub(56,73,85);
				glTranslatef(-1.5,-4.5,0);
				gluCylinder(qobj, 0.2, 0.2, 2.5, 20, 20);
				glPushMatrix();
					glColor3ub(56,73,85);
					glRotatef(90,1,0,0);
					glTranslatef(0,2.5,-.2);
					gluCylinder(qobj, 0.2, 0.2, 5, 20, 20);
					//PIES
						glPushMatrix();
							glRotatef(-15,1,0,0);
							glScalef(1.5,2,0.7);
							glTranslatef(0,0,6.5);
							glColor3ub(255,200,39);
							glutSolidCube(1.5);
						glPopMatrix();
				glPopMatrix();
			glPopMatrix();
			//PIERNA DER
			glPushMatrix();
				glColor3ub(56,73,85);
				glTranslatef(1.5,-4.5,0);
				gluCylinder(qobj, 0.2, 0.2, 2.5, 20, 20);
				glPushMatrix();
					glColor3ub(56,73,85);
					glRotatef(90,1,0,0);
					glTranslatef(0,2.5,-.2);
					gluCylinder(qobj, 0.2, 0.2, 5, 20, 20);
					//PIES
						glPushMatrix();
							glRotatef(-15,1,0,0);
							glScalef(1.5,2,0.7);
							glTranslatef(0,0,6.5);
							glColor3ub(255,200,39);
							glutSolidCube(1.5);
						glPopMatrix();
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();
		//Brazos
		glPushMatrix();
		//DER
			glPushMatrix();
				glColor3ub(56,73,85);
				glRotatef(90,0,1,0);
				glRotatef(-45,1,0,0);
				glTranslatef(1,-3,2.5);
				gluCylinder(qobj, 0.2, 0.2, 7, 20, 20);
				glPushMatrix();
					glScalef(2.0,2,2);
					glTranslatef(0,0,4);
					glColor3ub(255,200,39);
					glutSolidCube(1.0);
						//Dedo 1
						glPushMatrix();
							glScalef(0.5,0.25,0.75);
							glTranslatef(0,1.3,0.75);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
						//Dedo 2
						glPushMatrix();
							glScalef(0.5,0.25,0.75);
							glTranslatef(0,-1.3,0.75);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
						//Dedo 3
						glPushMatrix();
							glScalef(0.5,0.25,0.75);
							glTranslatef(0,0,0.75);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
						//Dedo 4
						glPushMatrix();
							glScalef(0.5,0.6,0.25);
							glTranslatef(0,1,0);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
				glPopMatrix();
			glPopMatrix();
		//IZQ
			glPushMatrix();
				glColor3ub(56,73,85);
				glRotatef(-90,0,1,0);
				glRotatef(-45,1,0,0);
				glTranslatef(-1,-3,2.5);
				gluCylinder(qobj, 0.2, 0.2, 7, 20, 20);
				glPushMatrix();
					glScalef(2.0,2,2);
					glTranslatef(0,0,4);
					glColor3ub(255,200,39);
					glutSolidCube(1.0);
						//Dedo 1
						glPushMatrix();
							glScalef(0.5,0.25,0.75);
							glTranslatef(0,1.3,0.75);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
						//Dedo 2
						glPushMatrix();
							glScalef(0.5,0.25,0.75);
							glTranslatef(0,-1.3,0.75);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
						//Dedo 3
						glPushMatrix();
							glScalef(0.5,0.25,0.75);
							glTranslatef(0,0,0.75);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
						//Dedo 4
						glPushMatrix();
							glScalef(0.5,0.6,0.25);
							glTranslatef(0,1,0);
							glColor3ub(255,200,39);
							glutSolidCube(1.0);
						glPopMatrix();
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		if(movimiento==0){
		glTranslatef(0,1,0);
		glColor4ub(56,73,85,20);
		glutSolidSphere(sphere.radius,20,10);
		}
	glPopMatrix();

}

void
display(void)
{
    static int i;
    static float c;
    static char s[32];
    static int frames = 0;
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
	glPushMatrix();
	mover();
	billy();
	glPopMatrix();
	glBegin(GL_POINTS);
		for (i = 0; i < num_particles; i++) {
		    if (particles[i].alive == 0)
			continue;
		    glColor3ub(0,0, 255);
		    for(int j=-3;j<3;j++){
		    	if(j==0){
		    		glVertex3f(particles[i].position[0],particles[i].position[1]+j,particles[i].position[2]+j);
				}else if (j>0){
				glVertex3f(particles[i].position[0],particles[i].position[1]+j,particles[i].position[2]+j-1);	
				}
				else{
					glVertex3f(particles[i].position[0],particles[i].position[1],particles[i].position[2]+2*j);		
				}
			}
		}	
	glEnd();
    glutSwapBuffers();
}

void
idleFunc(void)
{
    static int i;
    static int living = 0;		/* index to end of live particles */
    static float dt;

    dt = timedelta();
    frame_time += dt;

#if 1
    /* slow the simulation if we can't keep the frame rate up around
       10 fps */
    if (dt > 0.1) {
	slow_down = 0.75;
    } else if (dt < 0.1) {
	slow_down = 1;
    }
#endif

    dt *= slow_down;

    /* resurrect a few particles */
    for (i = 0; i < flow*dt; i++) {
	psNewParticle(&particles[living], dt);
	living++;
	if (living >= num_particles)
	    living = 0;
    }

    for (i = 0; i < num_particles; i++) {
	psTimeStep(&particles[i], dt);

	/* collision with sphere? */
	    psCollide(&particles[i]);
    }

    glutPostRedisplay();
}

void
visible(int state)
{
    if (state == GLUT_VISIBLE) {
        timedelta();
	glutIdleFunc(idleFunc);
    } else {
	glutIdleFunc(NULL);
    }
}


void
bail(int code)
{
    free(particles);
    exit(code);
}

void rotar(int key, int x, int y) {
	switch(key) {
		
		case GLUT_KEY_LEFT:  
			if(sphere.z==0){
				   giray-= 15;             
			}
			break;
		case GLUT_KEY_RIGHT:
			if(sphere.z==0){
				   giray+= 15;             
			}			
		break;        
		case GLUT_KEY_UP:
			if(movimiento+1<2){
				girax-=20;
				movimiento++;
				sphere.z++;
			} 
		break;
		case GLUT_KEY_DOWN: 
		if(movimiento-1>-4){
			girax+=20;
			movimiento--;
			sphere.z--;
					}	
		 break;
	}
	glutPostRedisplay();
}

int
main(int argc, char** argv)
{
    glutInitDisplayMode(GLUT_RGB|GLUT_DEPTH|GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
    glutInit(&argc, argv);
      glutCreateWindow("Proyecto");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(rotar);
    particles = (PSparticle*)malloc(sizeof(PSparticle) * num_particles);
    glutVisibilityFunc(visible);
    glutMainLoop();
    return 0;
}

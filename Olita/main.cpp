// Cubica
 
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include <random>

using namespace std;

#define B 0x100
#define BM 0xff
#define N 0x1000

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;

static void init_noise(void);
static int p[B + B + 2];
static float g2[B + B + 2][2];
static int start = 1;
#define s_curve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

float speed = 0.5f;
float length = 3.0f;
float amplitude = 0.4f;
float frequency = (2.0*3.14)/length;
float amplitude_noise = 30.0f;
float offset_noise = 0.5f;
float height_noise = 1.0f;
double size_turb = 16.0;
 
float variableKnots[25] = {0.0, 0.0,0.0,0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,18.0,18.0,18.0};
float ctlpoints[21][21][3];
GLUnurbsObj *theNurb;
float time = 2.0;
 
void changeViewport(int w, int h) {
     
    float aspectratio;
 
    if (h==0)
        h=1;
 
     
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(30, (GLfloat) w/(GLfloat) h, 1.0, 200.0);
   glMatrixMode (GL_MODELVIEW);
 
}

static void normalize2(float v[2])
{
	float s;

	s = sqrt(v[0] * v[0] + v[1] * v[1]);
	if(s != 0){
		v[0] = v[0] / s;
		v[1] = v[1] / s;
	}
}

static void normalize3(float v[3])
{
	float s;

	s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}


float noise2(float vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register int i, j;

	if (start) {
		start = 0;
		init_noise();
	}

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

	#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}


float turbulence(float x, float z, double size)
{
    float value = 0.0, initialSize = size;
	float datos[2];
    
    while(size >= 1)
    {
		datos[0] = (x*amplitude_noise+offset_noise)/size;
		datos[1] = (z*amplitude_noise+offset_noise)/size;
        value += noise2(datos) * size;
        size /= 2.0;
    }
    
    return(128.0 * value / initialSize);
}
float scalarP(float vect1[], float vect2[]) {

	return vect1[0]*vect2[0]+vect1[1]*vect2[1];

}

float olaFunc(float x, float z, float t) {
	float diCircular[2] = {0,0};
	float sq = sqrt((x*x)+(z*z));
	if(sq != 0) diCircular[0] = (x)/sq;
	if(sq != 0) diCircular[1] = (z)/sq;

	float points[2] = {x,z};

	float scalar = scalarP(diCircular, points);	
	return amplitude*sin(scalar*frequency+t*speed); 
}
 
void init_surface() {
	for (int i = -10; i < 11; i++)
	{
		for (int j = -10; j < 11; j++)
		{
			ctlpoints[i+10][j+10][0] = float(i);
			//ctlpoints[i+10][j+10][1] = 0.0;//olaFunc(float(i), float(j), time)+height_noise*0.005*turbulence(float(i), float(j), size_turb);
			ctlpoints[i+10][j+10][2] = float(j);
		}
	}
}


void ciclo_surface() {
	for (int i = -10; i < 11; i++)
	{
		for (int j = -10; j < 11; j++)
		{

			//Parte de la ola
			float diCircX = 0, diCircZ = 0;
			float speedy = speed*frequency;
			float sq = sqrt((i*i)+(j*j));
			if(sq != 0) {
				diCircX = (i)/sq;
				diCircZ = (j)/sq;
			}

			float scalar = diCircX*i+diCircZ*j;
			//Fin de parte de la ola

			//Turbulence
			float value = 0.0, initialSize = size_turb;
			float datos[2];
			float size_turb_var = size_turb; 
    
			while(size_turb_var >= 1)
			{
				datos[0] = (i*amplitude_noise+offset_noise)/size_turb_var;
				datos[1] = (j*amplitude_noise+offset_noise)/size_turb_var;

				//noise2

				//Fin noise2

				value += noise2(datos) * size_turb_var;
				size_turb_var /= 2.0;
			}
			//Fin Turbulence
			
			ctlpoints[i+10][j+10][1] = (amplitude*sin(scalar*frequency+time*speedy))+height_noise*0.005*(128.0 * value / initialSize);
		}
	}
}



static void init_noise(void)
{
	int i, j, k;

	for (i = 0 ; i < B ; i++) {
		p[i] = i;

		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((rand() % (B + B)) - B) / B;
		normalize2(g2[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = rand() % B];
		p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
	}
}

void animacion(int value) {
    glutTimerFunc(100,animacion,1);
    glutPostRedisplay();
    time-=0.5;
    ciclo_surface();
}
 
void init(){
	/*variableKnots[0] = 0.0;
	variableKnots[1] = 0.0;
	variableKnots[2] = 0.0;
	variableKnots[3] = 0.0;
	float val = 1.0;
	for (int i = 4; i < 21; i++)
	{
		variableKnots[i] = val;
		val++;
	}
	variableKnots[21] = val;
	variableKnots[22] = val;
	variableKnots[23] = val;
	variableKnots[24] = val;*/

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
 
	init_surface();
	//init_noise();
	theNurb = gluNewNurbsRenderer();
	gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 15.0);
	gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
	glutTimerFunc(10,animacion,1);
}
 
 
 
void Keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
   case 27:            
        exit (0);
        break;
	case 'a':
		amplitude += 0.1;
		break;
	case 'z':
		amplitude -= 0.1;
		break;
     
  }
}

 
void render(){

	//GLfloat* variablePuntosControl = ;
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    glLoadIdentity ();                      
    gluLookAt (25.0, 12.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
     
 
    // Luz y material
 
    GLfloat mat_diffuse[] = { 0.6, 0.6, 0.9, 1.0 };
    GLfloat mat_specular[] = { 0.8, 0.8, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 60.0 };
     
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
     
 
    GLfloat light_ambient[] = { 0.0, 0.0, 0.2, 1.0 };
    GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat light_specular[] = { 0.6, 0.6, 0.6, 1.0 };
    GLfloat light_position[] = { -10.0, -5.0, 0.0, 1.0 };
 
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);  
 
 
    //Suaviza las lineas
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_LINE_SMOOTH );

    glPushMatrix();
 
    gluBeginSurface(theNurb);
		gluNurbsSurface(theNurb, 
                   25, variableKnots, 25, variableKnots,
                   21 * 3, 3, (GLfloat *)&ctlpoints, 
                   4, 4, GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
     
    glPopMatrix();
             
 
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
 
    glutSwapBuffers();
}
 

 
int main (int argc, char** argv) {
 
    glutInit(&argc, argv);
 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
 
    glutInitWindowSize(960,540);
 
    glutCreateWindow("Test Opengl");
	
 
    init();
 
    glutReshapeFunc(changeViewport);
    glutDisplayFunc(render);
    glutKeyboardFunc (Keyboard);
         
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW error");
        return 1;
    }
    
    glutMainLoop();
    return 0;
 
}
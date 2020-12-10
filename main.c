/*
*/

#include <stdlib.h>


#include <stdio.h>
#include <malloc.h>
#include <GL/glut.h>
#include <math.h>
#include <windows.h>

//The particle structure
typedef struct  {
    double px, py, pz;
    double dx, dy, dz;
    double speed, scale, state;
    double rx, ry, rz,ra;
    double r, g, b;
    double radius;
    double transparency;
    int age;
    struct particle *next;

}particle;

//The global Structure
typedef struct {
    particle *head;
    particle *tail;
    int fire;
    double sprayfactor;


}glob;
glob global;

//This method creates a new particle
particle * createParticle(double x, double y, double z){
    particle *p;
    p = (particle *)malloc(sizeof(particle));
    p->age = 0;
    p->px = x; //position variables
    p->py = y;
    p->pz = z;
    p->r = 0.5;
    p->g = 0.5;
    p->b = 0.5;
    p->dx = (((double)rand() / RAND_MAX) - 0.3)*global.sprayfactor;//direction values
    p->dy = (double)rand() / RAND_MAX;
    p->dz = (((double)rand() / RAND_MAX) - 0.3)*global.sprayfactor;
    p->speed = 1;//particle speed
    p->scale = 0.5;//change size if applicable
    p->state = 0;//state for camera or not
    p->transparency = 1.0;
    p->radius=0.0;
    p->next = 0;

    return p;
}
//manually throw a new particle
void throwParticle(double px, double py, double pz){
    particle *newP;
    newP = createParticle(px,py,pz);
    if (global.head){
        global.tail->next = newP;
        global.tail = newP;
    }
    else{
        global.head = newP;
        global.tail = newP;
    }
}
//Draw the particle at the new location
void updateParticles(particle *current){

    glColor4f(current->r, current->g, current->b, current->transparency);
    current->transparency = current->transparency - 0.005;

    glPushMatrix(); //remember current matrix
    glTranslatef(current->px,current->py,current->pz);
    glBegin(GL_QUADS); /* render something */
        GLUquadric *myQuad;
        GLdouble radius = current->radius;
        if(radius < 3) {
            current->radius = current->radius + 0.08;
        }
        GLint slices, stacks;
        myQuad=gluNewQuadric();
        slices = stacks = 32;
        gluSphere( myQuad , radius , slices , stacks  );
    glEnd();
    glPopMatrix();

    current->age++;
}

//Update particle position
void updatePositions(){
    particle *current;
    current = global.head;
    do{
        //this makes it do a lil bounce when it hits the ground
//        if (current ->py > 0 && current->py < current->scale && current->pz <30 && current->pz > -30 && current->px > -30 && current->px < 30){
//            if (!((current->px > -20 && current->px < -10) && (current->pz>5 && current->pz < 15))){
//                current->dy = -0.95*current->dy;
//                current->dx = 0.95*current->dx;
//                current->dz = 0.95*current->dz;
//                if (current->dy < 0.0005 && current->dx <0.0005 && current->dz <0.0005){
//                    current->speed = 0;
//                }
//            }
//        }
          //this makes it go down
//        if(abs(current->py)>current->scale)  current->dy -= 0.01;
        current->px += current->speed*current->dx;
        current->py += current->speed*current->dy;
        current->pz += current->speed*current->dz;
        if (current->state == 1){
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            gluLookAt(current->px, current->py, current->pz, current->speed*current->dx, current->speed*current->dy, current->speed*current->dz, 0.0, 1.0, 0.0);
        }
        current = current->next;
    } while (current != 0);
}
//Change particle speeds
void changeSpeeds(){
    particle *current;
    current = global.head;
    while (current != 0){
        current->speed = (double)rand() / RAND_MAX;
        current = current->next;
    }
}
//Remove stationary particles, old particles and particles below screen
void cleanParticles(){
    particle *current,*temp;
    current = global.head;

    while (current != 0){

        temp = current->next;
        if (temp != 0 && (temp->py < -60||temp->speed==0||temp->age >100)){
            current->next = temp->next;
            if (temp->state == 1){
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(20, 30, 70, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
            }
            if (temp == global.tail) global.tail == current;
            free(temp);

        }
        current = current->next;
    }

}
//reset to original setup
void reset(){
    particle *current;
    current = global.head;
    while (current != 0){
        current->age = 4000;

        current = current->next;
    }
    global.fire = 1;
    global.sprayfactor = 1;
    global.tail = global.head;
}

//Main draw frame loop
void drawStage(void){
    particle *current = global.head;
    int i;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Clear Color and Depth Buffers
    // Draw ground
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(30.0f, -2.0f, 30.0f);
    glVertex3f(-30.0f, -2.0f, 30.0f);
    glVertex3f(-30.0f, -2.0f, -30.0f);
    glVertex3f(30.0f, -2.0f, -30.0f);
    glEnd();
    //draw origin point
    for (i = 0; i < 6; ++i) {
        glColor3f(0.0f,0.6f,0.5f);
        glutSolidTeapot(2);
    }
    if (global.fire)throwParticle(0,1,0);
    if (current !=NULL) updatePositions();
    while (current != 0){
        if (current->state != 1 && current != global.head){
            updateParticles(current);
        }
        current = current->next;
    }
    cleanParticles();
    glutPostRedisplay();
    glutSwapBuffers();
}
/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 0x1B:
        case'q':
        case 'Q':
            exit(0);
        case'p':
        case'P':
            throwParticle(0,1,0);
            break;
        case's':
        case'S':
            changeSpeeds();
            break;
        case'f':
        case'F':
            if (global.fire)
                global.fire = 0;
            else global.fire = 1;
            break;
        case'R':
        case'r':
            reset();
            break;
        case'e':
        case'E':
            global.tail->state = 1;
            break;
        case't':
        case'T':
            if (global.sprayfactor == 0.2)
                global.sprayfactor = 1;
            else global.sprayfactor = 0.2;
            break;
    }
}//keyboard
int main(int argc, char** argv)
{
    printf("Q:Quit\nP:Throw particle\nS:Change speed\nF:Start/stop Stream\nR:Reset\nE:Throw camera\nT:Change spread\n");
    global.sprayfactor = 0.5;
    global.fire = 1;
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Smoke");
    glutDisplayFunc(drawStage);
    glutKeyboardFunc(keyboard);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT), 1, 200.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(20, 30, 70, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glClearColor(0.2, 0.6, 0.6,0.9);
    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
    return 0;
}
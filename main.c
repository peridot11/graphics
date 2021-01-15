/*
*/

#include <stdlib.h>

#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>


#define MOVING_VIEW 1
#define STANDARD_VIEW 2
#define DEG_TO_RAD 0.01745329
#define RUN_SPEED 10.0
#define MASS 0.001
#define EARTH_GRAVITY 9.8
#define SCARY_GRAVITY 980.0


GLdouble lon;              /* View angles (degrees)    */
GLdouble eyex, eyey, eyez;    /* Eye point                */
GLdouble centerx, centery, centerz; /* Look point               */
GLdouble upx, upy, upz;     /* View up vector           */
clock_t current_ticks, delta_ticks;
clock_t fps = 0;
int TOGGLE_GRAVITY;
int TOGGLE_MULTIPLE_SOURCES;
int current_view;
int max_age;


//The particle structure
typedef struct {
    double px, py, pz;
    double dx, dy, dz;
    double speed, scale;
    double r, g, b;
    double radius;
    double transparency;
    int age;
    struct particle *next;

} particle;

//The global Structure
typedef struct {
    particle *head;
    particle *tail;
    int smoke;
    double sprayfactor;


} glob;
glob global;


void setStandardViewCoordinates();

//This method creates a new particle
particle *createParticle(double x, double y, double z) {
    particle *p;
    p = (particle *) malloc(sizeof(particle));
    p->age = 0;
    p->px = x; //position variables
    p->py = y;
    p->pz = z;
    p->r = 0.5;
    p->g = 0.5;
    p->b = 0.5;
    p->dx = (((double) rand() / RAND_MAX) - 0.2) * global.sprayfactor;//direction values
    p->dy = (double) rand() / RAND_MAX;
    p->dz = (((double) rand() / RAND_MAX) - 0.2) * global.sprayfactor;
    p->speed = 0.2;//particle speed
    p->scale = 0.5;//change size if applicable
    p->transparency = 1.0;
    p->radius = 0.0;
    p->next = 0;

    return p;
}

//manually throw a new particle
void throwParticle(double px, double py, double pz) {
    particle *newP;
    newP = createParticle(px, py, pz);
    if (global.head) {
        global.tail->next = newP;
        global.tail = newP;
    } else {
        global.head = newP;
        global.tail = newP;
    }
}

//Draw the particle at the new location
void updateParticles(particle *current) {

    glColor4f(current->r, current->g, current->b, current->transparency);
    current->transparency = current->transparency - 0.003;

    glPushMatrix(); //remember current matrix
    glTranslatef(current->px, current->py, current->pz);
    glBegin(GL_QUADS); /* render something */
    GLUquadric *myQuad;
    GLdouble radius = current->radius;
    if (radius < 1 ) {
        current->radius = current->radius + 0.1;
    }
    if(radius < 4  && current->py > 5){
        current->radius = current->radius + 0.05;
    }
    GLint slices, stacks;
    myQuad = gluNewQuadric();
    slices = stacks = 64;
    gluSphere(myQuad, radius, slices, stacks);
    glEnd();
    glPopMatrix();

    current->age++;
}

//Update particle position
void updatePositions() {
    particle *current;
    current = global.head;
    do {
        current->px += current->speed * current->radius * current->dx;
        current->py += current->speed * current->radius * current->dy;
        current->pz += current->speed * current->radius * current->dz;
        //earth gravity
        if(TOGGLE_GRAVITY == 1) current->py = fmax(current->py - MASS * EARTH_GRAVITY,0);
        //random planet with super intense gravity
        if(TOGGLE_GRAVITY == 2) current->py = fmax(current->py - MASS * SCARY_GRAVITY,0);


        current = current->next;
    } while (current != 0);
}

//Change particle speeds
void changeSpeeds() {
    particle *current;
    current = global.head;
    while (current != 0) {
        current->speed = current->speed +0.2;
        current = current->next;
    }
}

void changeMaxAge() {
    if (max_age == 100) max_age = 500;
    else max_age -= 100;
    printf("max_age: %d\n", max_age);
}

//Remove stationary particles, old particles and particles below screen
void cleanParticles() {
    particle *current, *temp;
    current = global.head;

    while (current != 0) {

        temp = current->next;
        if (temp != 0 && (temp->age > max_age || temp->transparency < 0.3)) {
            current->next = temp->next;
            if (temp == global.tail) global.tail = current;
//            free(temp);

        }
        current = current->next;
    }

}

//reset to original setup
void reset() {
    particle *current;
    current = global.head;
    while (current != 0) {
        current->age = 4000;

        current = current->next;
    }
    global.smoke = 1;
    global.sprayfactor = 1;
    global.tail = global.head;
    TOGGLE_MULTIPLE_SOURCES = 1;
    TOGGLE_GRAVITY = 0;
    max_age = 300;
}


void setView(void) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    switch (current_view) {
        case MOVING_VIEW:
            gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
            break;
        case STANDARD_VIEW:
            gluLookAt(20, 30, 70, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
            break;
    }
}

void countFPS() {
    delta_ticks = clock() - current_ticks; //the time, in ms, that took to render the scene
    if(delta_ticks > 0)
        fps = CLOCKS_PER_SEC / delta_ticks;
    printf("%ld \n", fps);
}

//Main draw frame loop
void drawStage(void) {
    current_ticks = clock();
    particle *current = global.head;
    // Clear Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Draw ground
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(50.0f, -2.0f, 50.0f);
    glVertex3f(-50.0f, -2.0f, 50.0f);
    glVertex3f(-50.0f, -2.0f, -50.0f);
    glVertex3f(50.0f, -2.0f, -50.0f);
    glEnd();
    //draw origin point
    glColor3f(0.0f, 0.6f, 0.5f);
    glutSolidTeapot(2);

    setView();

    //yeet the particles
    if (global.smoke){
        throwParticle(0, 1, 0);
        if(TOGGLE_MULTIPLE_SOURCES > 1){
            throwParticle(30, 1, 30);
        }
        if(TOGGLE_MULTIPLE_SOURCES > 2){
            throwParticle(-30,1, 30);
        }
        if(TOGGLE_MULTIPLE_SOURCES > 3){
            throwParticle(-30,1, -30);
        }
        if(TOGGLE_MULTIPLE_SOURCES > 4){
            throwParticle(30,1, -30);
        }
    }
    if (current != NULL) updatePositions();
    while (current != 0) {
        if (current != global.head) {
            updateParticles(current);
        }
        current = current->next;
    }
    cleanParticles();
    glutPostRedisplay();
    glutSwapBuffers();
    countFPS();
}

/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 0x1B:
        case 'q':
        case 'Q':
            exit(0);
        case 'p':
        case 'P':
            changeSpeeds();
            break;
        case 'a':
        case 'A':
            changeMaxAge();
            break;
        case 'R':
        case 'r':
            reset();
            break;
        case 't':
        case 'T':
            if (global.sprayfactor == 0.2)
                global.sprayfactor = 1;
            else global.sprayfactor = 0.2;
            break;
        case 'G':
        case 'g':
            if (TOGGLE_GRAVITY == 2)
                TOGGLE_GRAVITY = 0;
            else TOGGLE_GRAVITY += 1;
            break;
        case 'M':
        case 'm':
            if (TOGGLE_MULTIPLE_SOURCES == 5)
                TOGGLE_MULTIPLE_SOURCES = 1;
            else TOGGLE_MULTIPLE_SOURCES += 1;
            break;
        case 's':
        case 'S':
            eyez = eyez + cos(lon * DEG_TO_RAD) * RUN_SPEED;
            eyex = eyex + sin(lon * DEG_TO_RAD) * RUN_SPEED;
            break;
        case 'w':
        case 'W':
            eyez = eyez - cos(lon * DEG_TO_RAD) * RUN_SPEED;
            eyex = eyex - sin(lon * DEG_TO_RAD) * RUN_SPEED;
            break;
    }
}//keyboard
void menu(int menuentry) {
    switch (menuentry) {
        case 1:
            current_view = MOVING_VIEW;
            break;
        case 2:
            setStandardViewCoordinates();
            break;
        case 3:
            global.smoke = 0;
            break;
        case 4:
            global.smoke = 1;
            break;
        case 5:
            exit(0);
    }
}

void init(void) {
    glutCreateMenu(menu);
    glutAddMenuEntry("Moving view", 1);
    glutAddMenuEntry("Standard view", 2);
    glutAddMenuEntry("Stop smoke", 3);
    glutAddMenuEntry("Start smoke", 4);
    glutAddMenuEntry("Quit", 5);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    TOGGLE_MULTIPLE_SOURCES = 1;
    global.sprayfactor = 1;
    global.smoke = 1;
    setStandardViewCoordinates();

    max_age = 300;
}

void setStandardViewCoordinates() {/* Set initial view parameters */
    eyex = 20.0; /* Set eyepoint at eye height within the scene */
    eyey = 30.0;
    eyez = 70.0;

    upx = 0.0;   /* Set up direction to the +Y axis  */
    upy = 1.0;
    upz = 0.0;

    lon = 0.0;   /* ... along the +Z axis  */


    current_view = STANDARD_VIEW;
}

int main(int argc, char **argv) {
    printf("Q:Quit\nP:Change speed\nF:Start/stop Stream\nR:Reset\nT:Change spread\nA:Change lifetime of particles\nG:Toggle Gravity\n");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Smoke");
    init();
    glutDisplayFunc(drawStage);
    glutKeyboardFunc(keyboard);
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, glutGet(GLUT_WINDOW_WIDTH*2) / glutGet(GLUT_WINDOW_HEIGHT*2), 1, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.2, 0.6, 0.6, 0.9);
    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
    return 0;
}
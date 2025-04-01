#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_RADIUS 25
#define BOX_HEIGHT 50

int *rope_position;
int shmid;
int score =10;
time_t startTime;

void initSharedMemory() {
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }

    shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    rope_position = (int *)shmat(shmid, NULL, 0);
    if (rope_position == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    *rope_position = SCREEN_WIDTH / 2; // Initialize the rope position
    startTime = time(NULL);
}

void cleanupSharedMemory() {
    if (shmdt(rope_position) == -1) {
        perror("shmdt failed");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    }
}

void drawText(int x, int y, const char *str) {
    glRasterPos2i(x, y);
    while (*str) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);
        str++;
    }
}

void drawBox(int x, int y, int width, int height) {
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();
}

void drawScore() {
    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    drawBox(20, SCREEN_HEIGHT - 20, 100, 40);
    drawText(30, SCREEN_HEIGHT - 40, scoreText);
}

void drawTime() {
    char timeText[20];
    int elapsed = (int)(time(NULL) - startTime);
    sprintf(timeText, "Time: %d s", elapsed);
    drawBox(SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20, 100, 40);
    drawText(SCREEN_WIDTH - 110, SCREEN_HEIGHT - 40, timeText);
}

void drawCircle(float cx, float cy, float r, float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i += 10) {
        float theta = i * 3.14159f / 180;
        glVertex2f(cx + r * cos(theta), cy + r * sin(theta));
    }
    glEnd();
}

void drawFace(float cx, float cy) {
    // Draw eyes
    glColor3f(0, 0, 0);
    drawCircle(cx - 7, cy + 7, 5, 0, 0, 0);
    drawCircle(cx + 7, cy + 7, 5, 0, 0, 0);
    
    // Draw eyelashes
    glBegin(GL_LINES);
    glVertex2f(cx - 10, cy + 15);
    glVertex2f(cx - 5, cy + 10);
    glVertex2f(cx + 5, cy + 10);
    glVertex2f(cx + 10, cy + 15);
    glEnd();
}

void drawPlayer(int x, float y, float r, float g, float b, const char *name, int effort) {
    drawCircle(x, y, PLAYER_RADIUS, r, g, b);
    drawFace(x, y);
    drawText(x - 15, y - PLAYER_RADIUS - 20, name);
    
    // Draw effort box above the player
    int boxY = y + PLAYER_RADIUS + 30;
    drawBox(x - 20, boxY, 40, 20);
    char effortText[10];
    sprintf(effortText, "%d", effort);
    drawText(x - 10, boxY + 5, effortText);
}

void drawRope() {
    glColor3f(0, 1, 0);
    glLineWidth(10);
    glBegin(GL_LINES);
    glVertex2f(*rope_position - 300, SCREEN_HEIGHT / 2);
    glVertex2f(*rope_position + 300, SCREEN_HEIGHT / 2);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawRope();
    drawScore();
    drawTime();

    // Left team
    drawPlayer(150, SCREEN_HEIGHT / 2, 1, 0, 0, "Ghassan", 80);
    drawPlayer(230, SCREEN_HEIGHT / 2, 1, 0, 0, "nono", 70);
    drawPlayer(290, SCREEN_HEIGHT / 2, 1, 0, 0, "bobo", 60);
    drawPlayer(360, SCREEN_HEIGHT / 2, 1, 0, 0, "Sohaib", 65);

    // Right team
    drawPlayer(SCREEN_WIDTH - 150, SCREEN_HEIGHT / 2, 0, 0, 1, "Mohammad Ataya", 80);
    drawPlayer(SCREEN_WIDTH - 220, SCREEN_HEIGHT / 2, 0, 0, 1, "3bsy", 75);
    drawPlayer(SCREEN_WIDTH - 290, SCREEN_HEIGHT / 2, 0, 0, 1, "momo", 60);
    drawPlayer(SCREEN_WIDTH - 360, SCREEN_HEIGHT / 2, 0, 0, 1, "zozo", 65);

    // Referee
    drawPlayer(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 100, 0, 1, 0, "Hanna Bullata", 0);

    glutPostRedisplay(); // Ensure continuous updates
    glFlush();
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, timer, 0);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

void initOpenGL() {
    glClearColor(1, 1, 1, 1);
}

int main(int argc, char **argv) {
    initSharedMemory();
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Rope Pulling Game");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    initOpenGL();
    glutMainLoop();
    
    cleanupSharedMemory();
    return 0;
}

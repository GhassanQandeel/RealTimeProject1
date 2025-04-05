#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <signal.h>
#include "../include/config.h"
char fifo_name[50];

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_RADIUS 25
#define BOX_HEIGHT 50
 const char *team1[4] = { "Ghassan", "3bsi", "Ghaith", "Zein" };
 const char *team2[4] = {"Suhad", "Elien", "Sohaib","Ataya" };
int *rope_position;
int shmid;
time_t startTime;
int previous_socre_1 =0;
int previous_socre_2 =0;
int team_win=0;

void initSharedMemory() {
    key_t key = ftok("./bin/shmfile", 65);
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
    // Set fill color
    glColor3f(0.7f, 0.7f, 0.7f); // Light gray fill color
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();

    // Set border color
    glColor3f(0.0f, 0.0f, 0.0f); // Black border
    glLineWidth(2.0f); // Make border slightly thicker
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();
}


void drawScore(int score1,int score2) {
    char scoreText1[20];
    char scoreText2[20];
    sprintf(scoreText1, "Score Team 1: %d", score1);
    sprintf(scoreText2, "Score Team 2: %d", score2);
    
    drawBox(90, SCREEN_HEIGHT - 140, 180, 40);
    drawText(100, SCREEN_HEIGHT - 160, scoreText1);
    drawBox(SCREEN_WIDTH - 250, SCREEN_HEIGHT - 140, 180, 40);
    drawText(SCREEN_WIDTH - 240, SCREEN_HEIGHT - 160, scoreText2);
    
    
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
    
    // Draw eyelashes (moved up by 3 units)
    glBegin(GL_LINES);
    glVertex2f(cx - 10, cy + 18);  // was cy + 15
    glVertex2f(cx - 5, cy + 13);   // was cy + 10
    glVertex2f(cx + 5, cy + 13);   // was cy + 10
    glVertex2f(cx + 10, cy + 18);  // was cy + 15
    glEnd();
}

void drawPlayer(int x, float y, float r, float g, float b, const char *name, int effort) {
    drawCircle(x, y, PLAYER_RADIUS, r, g, b);
    drawFace(x, y);

    // Center the name under the face
    int textWidth = strlen(name) * 8; // Assuming 8 pixels per character
    drawText(x - textWidth / 2, y - PLAYER_RADIUS - 20, name);

    // Draw effort box above the player
    int boxY = y + PLAYER_RADIUS + 30;
    drawBox(x - 20, boxY + 20, 40, 20);

    // Center the effort value in the box
    char effortText[10];
    sprintf(effortText, "%d", effort);
    int effortTextWidth = strlen(effortText) * 8;
    drawText(x - effortTextWidth / 2, boxY + 5, effortText);
}

void drawReferee(int x, float y, float r, float g, float b, const char *name, int effort) {
    drawCircle(x, y, PLAYER_RADIUS, r, g, b);
    drawFace(x, y);

    // Center the name under the face
    int textWidth = strlen(name) * 8; // Assuming 8 pixels per character
    drawText(x - textWidth / 2, y - PLAYER_RADIUS - 20, name);


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

    // Read the message from FIFO
    Opengl_Message message;
    int fd_opengl = open(fifo_name, O_RDONLY);
    if (fd_opengl < 0) {
        perror("open failed for FIFO in display");
        exit(EXIT_FAILURE);
    }

    // Read the message
    ssize_t bytesRead = read(fd_opengl, &message, sizeof(message));
    if (bytesRead < 0) {
        perror("read failed from FIFO");
        exit(EXIT_FAILURE);
    }
    
    

    // Draw the rope
    drawRope();

    if (previous_socre_1 !=message.score_team_1 &&previous_socre_1!=0)
       team_win=1;
    else if(previous_socre_2 !=message.score_team_2&&previous_socre_1!=0)    
	team_win=2;
    // Draw the score
    drawScore(message.score_team_1,message.score_team_2);
    
    previous_socre_1 = message.score_team_1;
    previous_socre_2 = message.score_team_2;
	
    	
	
    // Draw the time
    drawTime();
// Left team (using current effort values from the message)
// Start from the leftmost position for index 0
for (int i = 3; i >= 0; i--) {
    drawPlayer(150 + (3 - i) * 70, SCREEN_HEIGHT / 2, 1, 0, 0,team1[message.current_effort_team_1[1][i]], message.current_effort_team_1[0][i]);
}

// Right team (using current effort values from the message)
// Start from the rightmost position for index 0
for (int i = 3; i >= 0; i--) {
    drawPlayer(SCREEN_WIDTH - 150 - (3 - i) * 70, SCREEN_HEIGHT / 2, 0, 0, 1, team2[message.current_effort_team_2[1][i]], message.current_effort_team_2[0][i]);
}
    if(team_win==1)
    	drawText(100, SCREEN_HEIGHT - 200, "Team 1 Win ");
    else 
        drawText(SCREEN_WIDTH - 240, SCREEN_HEIGHT - 200, "Team 2 Win"); 

    // Referee
    drawReferee(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 100, 0, 1, 0, "Dr.Hanna Bullata", 0);
    

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
    
   strncpy(fifo_name, argv[1], sizeof(fifo_name) - 1); // Safe copy of argv[1] into fifo_name
    
    
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

#ifndef __opengl_h__
#define __opengl_h__

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_RADIUS 25
#define BOX_HEIGHT 50




void drawText(int x, int y, const char *str);
void drawBox(int x, int y, int width, int height);
void drawScore();
void drawTime();
void drawCircle(float cx, float cy, float r, float red, float green, float blue);
void drawFace(float cx, float cy);
void drawPlayer(int x, float y, float r, float g, float b, const char *name, int effort);
void drawRope();

#endif        

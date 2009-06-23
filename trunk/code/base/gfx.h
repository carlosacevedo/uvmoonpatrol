#ifndef __GFX_H__
#define __GFX_H__

#include <SDL/SDL.h>
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define GAME_WIDTH 240
#define GAME_HEIGHT 248
#define COLORKEY 0,255,0 // transparent color
#define KEY_R 0
#define KEY_G 255
#define KEY_B 0

// Sprite - DrPetter from 7th Swarming
struct Sprite
{
	GLuint texture;
	int width;
	int height;
};

int InitGL();
SDL_Surface *LoadBMP2RGBA(char *filename, bool texSizeCheck=true);
void DrawSprite(Sprite & sprite, int x, int y, bool flip);
bool generateSprite(char * filename, Sprite * sprite);

#endif // __GFX_H__

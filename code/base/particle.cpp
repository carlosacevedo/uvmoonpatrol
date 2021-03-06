#include "particle.h"
#include <math.h>

#define PI 3.14159265

typedef enum {
    MOVING=0,
    GROUND
};

Particle::Particle()
{
    pX = pY = 0.0;
    pState = GROUND;
}

Particle::Particle(int x, int y, int angle, double length, double g, int l, char * filename, unsigned int type, SpriteManager * newManager)
{
    sManager = newManager;
    pState = MOVING;
    pType = type;
    pX = double(x);
    pY = double(y);
    vX = length * cos(angle*PI/180);
    vY = length * sin(angle*PI/180);
    gravity = g;
    life = maxLife = l;
    pSprite = sManager->newSprite(filename);
}

Particle::~Particle()
{
    sManager->removeSprite(pSprite);
}

void Particle::update(int newScroll)
{
    scrollX = newScroll;
    if ( pType == FOUNTAIN)
    {
        if ( pState == MOVING )
        {
            vY -= gravity;
            pX += vX;
            pY -= vY;
            if ( pY >= 200 )
            {
                pState = GROUND;
                pY = 200;
            }
        }
        else if ( pState == GROUND )
        {
            if ( life > 0 ){
                life--;
            }
        }
    }
    else if ( pType == CLOUD )
    {
        pX += vX;
        pY -= vY;
        life--;
    }
    else if ( pType == POP )
    {
        if ( pState == MOVING )
        {
            pX += vX;
            pY -= vY;
            vY -= gravity;
            life--;
            if ( pY >= 200 )
            {
                pState = GROUND;
                pY = 200;
            }
        }
        else if ( pState == GROUND )
        {
            if ( life > 0 ){
                life--;
            }
        }
    }
    else if ( pType == JET )
    {
        pX += vX;
        pY -= vY;
        vY -= gravity;
        life--;
    }
}

void Particle::draw()
{
    if ( ((int)pX-scrollX) > 0)
    {
        DrawSprite(*pSprite, ((int)pX-scrollX), int(pY), FALSE);
    }
}

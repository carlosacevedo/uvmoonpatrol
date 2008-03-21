#ifndef __CANNON_H__
#define __CANNON_H__

#include "base.h"
#include "gfx.h"

class Cannon
{
public:
    Cannon();
    Cannon(int,int);
    ~Cannon();
    void update(int);
    void draw();
    int getLife(){return life;}
    int getX(){return canX;}
    int getY(){return canY;}
    int width();
    int height();
private:
    Sprite cannonSprite;
    Sprite explosionSprite[3];
    int canX,canY,startCanX;
    int levelX;
    int life;
};

#endif // __CANNON_H__

#include "game.h"

Game::Game(){
}

Game::~Game(){
    Shutdown();
    if ( currentScene == INTRO){
    }
    else if ( currentScene == SPLASH ){
    }
    else if ( currentScene == INGAME ){
        ShutdownGame();
    }
}

int Game::Init(){
    srand((unsigned)time(0));
    gLastTick = SDL_GetTicks();

    gKeyLeft=gKeyRight=gKeyUp=gKeyDown=gKeySpace=gKeyCtrl=done=FALSE;
    fullscreen=FALSE;

    /* Initialize SDL */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Create Our OpenGL Window
    if (!CreateGLWindow("Ultraviolent Moon Patrol", SCREEN_WIDTH, SCREEN_HEIGHT, 32, fullscreen))
    {
        SDL_Quit();
        return 0;				// Quit If Window Was Not Created
    }

    if (!InitGL())			// Initialize Our Newly Created GL Window
    {
        KillGLWindow();			// Reset The Display
        return -1;			// Return Error
    }

    return 1;
}

void Game::Shutdown(){
    // Shutdown
    KillGLWindow();			// Kill The Window
    SDL_Quit();
}

void Game::Run(){
    if ( Init() )
    {
        currentScene = INGAME;
        InitGame();
        while(!done) {
            if ( currentScene == INTRO ){
                IntroLogic();
                IntroRender();
            }
            else if ( currentScene == SPLASH ){
            }
            else if ( currentScene == INGAME ){
                GameLogic();
                GameRender();
                // HUGE HUGE HUGE DEBUG!!!
                if ( gameCar->isCrashed() == TRUE ){
                    ShutdownGame();
                    InitGame();
                }
                //
            }
            Timer();
        }
    }
}

void Game::InitIntro()
{
}

void Game::ShutdownIntro()
{
}

void Game::IntroRender()
{
}

void Game::IntroLogic()
{
    KeyInput();
}

void Game::InitGame()
{
    spriteManager = new SpriteManager();    // sprite manager to handle generating textures
    actorPool = new ActorPool();            // pool to hold our groups
    playerGroup = actorPool->createGroup(); // player objects
    enemyGroup = actorPool->createGroup();  // enemy/level objects
    extraGroup = actorPool->createGroup();  // non-collidables (particles, effects)
    guiSprite = spriteManager->newSprite("data\\gui.png");
    gameCar = new Car(spriteManager);
    actorPool->addActor(gameCar, playerGroup); // add the car to the player group

    gameLevel = new Level(spriteManager);
    gameFont = new Font(spriteManager);
    carCannon = 0; // nothing for this yet!

    playerScore = 0;
    highScore = 15000; // will be filled in later
    currentTime = currentTimeBuffer = 0;
}

void Game::ShutdownGame()
{
    delete gameFont;
    //delete gameCar;
    delete gameLevel;
    if ( carCannon != 0 ){
        delete carCannon;
    }
    while(!carMissile.empty()){
        delete carMissile.back(); carMissile.pop_back();
    }
    delete actorPool;
    delete spriteManager;
}

void Game::GameRender()				// Draw Everything
{
    // Clear Screen, Depth Buffer & Stencil Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw our sprites
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    gameLevel->draw();
    actorPool->drawPool();

    if ( carCannon != 0 ){
        carCannon->draw();
    }

    DrawSprite(*guiSprite, 0, 0, FALSE);
    gameFont->drawOrangeNum(16,10,highScore,6);     // draw high score
    gameFont->drawYellowNum(26,25,playerScore,6);   // x = 0, y = 0, score = 13525, buffer = 6
    gameFont->drawOrangeNum(233,17,2,1);
    gameFont->drawOrangeNum(122,25,currentTime,3);

    glPopMatrix();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glFlush();

    SDL_GL_SwapBuffers();
}

void Game::GameLogic()
{
    KeyInput();

    if (gKeyLeft)
        gameCar->moveLeft();
    else if (gKeyRight)
        gameCar->moveRight();
    else
        gameCar->stopMove();
    if (gKeySpace)
        gameCar->jump();
    if (gKeyCtrl){
        if ( carCannon == 0){
            carCannon = new Cannon(gameCar->getX()+32,gameCar->getY()+16, spriteManager);
        }
        carMissile.push_back(new Missile(gameCar->getX()-gameCar->getScreenX()+9,gameCar->getY()+5, spriteManager));
        gKeyCtrl = FALSE;
    }

    int scrollX = gameCar->getScreenX();
    actorPool->updatePhysics(gameLevel, scrollX);
    gameLevel->update(scrollX, actorPool, enemyGroup);

    if ( carCannon!= 0){
        if ( carCannon->getLife() <= 0 ){
            delete carCannon;
            carCannon = 0;
        }
        else{
            carCannon->update(scrollX);
        }
    }

    /* iterate through all the missiles on the screen */
    std::vector<Missile*>::iterator iter = carMissile.begin();
    while ( iter != carMissile.end()){
        Missile * m = *(iter);
        if ( m->getY() < 10){
            iter = carMissile.erase(iter);
        }
        else{
            m->update();
            ++iter;
        }
    }
    /* iteratore through all the moon men on the screen *
    std::vector<MoonMan*>::iterator moonIter = gameMoonMen.begin();
    while ( moonIter != gameMoonMen.end()){
        MoonMan * mMan = *(moonIter);
        mMan->update(gameLevel, scrollX);
        if (rectCollision(gameCar->getX(),gameCar->getY(),gameCar->width(),gameCar->height()-10, // 10 compensates for bottom of the car
                mMan->getX(),mMan->getY(),mMan->width(),mMan->height())){
            if ( gameCar->isAirBorne() ){
                BloodFountain * newFountain = new BloodFountain(mMan->getX()+3,mMan->getY()+15, 90, 3.0, 100, 0.05, spriteManager);
                newFountain->setSpurting();
                mmFountain.push_back(newFountain);
            }
            else{
                BloodFountain * newFountain = new BloodFountain(mMan->getX(),mMan->getY(), 90, 2.0, 50, 0.01, spriteManager);
                newFountain->setPop();
                mmFountain.push_back(newFountain);
            }
            if ( gameCar->isAirBorne() ){
                gameCar->boostUp();
            }
            else{
                gameCar->slowDown();
            }
            moonIter = gameMoonMen.erase(moonIter);
            playerScore += 500;
        }
        else if (carCannon != 0 && (rectCollision(carCannon->getX(),carCannon->getY()-4,carCannon->width(),carCannon->height()+8, // 10 compensates for bottom of the car
                mMan->getX(),mMan->getY(),mMan->width(),mMan->height()))){
            BloodFountain * newFountain = new BloodFountain(mMan->getX(),mMan->getY(), 90, 4.0, 50, 0.25, spriteManager);
            newFountain->setPop();
            mmFountain.push_back(newFountain);
            delete carCannon;
            carCannon = 0;
            moonIter = gameMoonMen.erase(moonIter);
            playerScore += 500;
        }
        else{
            ++moonIter;
        }
    }

    std::vector<JetMan*>::iterator jetIter = gameJetMen.begin();
    std::vector<JetFountain*>::iterator jfIter = jmFountain.begin();
    while ( jetIter != gameJetMen.end() ){
        JetMan * jMan = *(jetIter);
        JetFountain * jFountain = *(jfIter);
        jMan->update(scrollX, jfIter,gameCar->getX(),gameCar->getY());
        jFountain->update(scrollX);
        if (rectCollision(gameCar->getX()+5,gameCar->getY()+30,gameCar->width()-5,gameCar->height()-30, // only 6 pixels will actually collide
                jMan->getX()+3,jMan->getY()+3,jMan->width()-3,jMan->height()-3)){
            BloodFountain * newFountain = new BloodFountain(jMan->getX(),jMan->getY(), 270, 2.0, 50, 0.3, spriteManager);
            newFountain->setPop();
            mmFountain.push_back(newFountain);
            gameCar->boostUp();
            jetIter = gameJetMen.erase(jetIter);
            jfIter = jmFountain.erase(jfIter);
            playerScore += 650;
        }
        else if (carCannon != 0 && (rectCollision(carCannon->getX(),carCannon->getY()-4,carCannon->width(),carCannon->height()+8, // 10 compensates for bottom of the car
                jMan->getX(),jMan->getY(),jMan->width(),jMan->height()))){
            BloodFountain * newFountain = new BloodFountain(jMan->getX(),jMan->getY(), 0, 2.0, 40, 0.1, spriteManager);
            newFountain->setPop();
            mmFountain.push_back(newFountain);
            delete carCannon;
            carCannon = 0;
            jetIter = gameJetMen.erase(jetIter);
            jfIter = jmFountain.erase(jfIter);
            playerScore += 650;
        }
        else{
            BOOL missileHit = FALSE;
            std::vector<Missile*>::iterator mIter = carMissile.begin();
            while(mIter != carMissile.end()){
                Missile * m = *(mIter);
                if ( rectCollision(jMan->getX(),jMan->getY(),jMan->width(),jMan->height(),
                        m->getX()+scrollX,m->getY(),m->width(),m->height())){
                    missileHit = TRUE;
                    carMissile.erase(mIter);
                    mIter = carMissile.end();
                }
                else{
                    ++mIter;
                }
            }

            if (missileHit == TRUE){
                BloodFountain * newFountain = new BloodFountain(jMan->getX(),jMan->getY(), 0, 2.0, 30, 0.15, spriteManager);
                newFountain->setPop();
                mmFountain.push_back(newFountain);
                jetIter = gameJetMen.erase(jetIter);
                jfIter = jmFountain.erase(jfIter);
                playerScore += 650;

            }
            else{
                ++jetIter;
                ++jfIter;
            }
        }
    }
    std::vector<BloodFountain*>::iterator bfIter = mmFountain.begin();
    while( bfIter != mmFountain.end() ){
        BloodFountain * bf = *(bfIter);
        bf->update(scrollX);
        if ( bf->getLife() <= 0){
            bfIter = mmFountain.erase(bfIter);
        }
        else{
            ++bfIter;
        }
    }

    std::vector<Bus*>::iterator busIter = gameBuses.begin();
    while ( busIter != gameBuses.end() ){
        Bus * bus = *(busIter);
        bus->update(scrollX, gameCar->getX());
        if ( bus->isActive()){
            if ( rectCollision(gameCar->getX()+5,gameCar->getY(),gameCar->width()-5,gameCar->height()-10, // 10 compensates for bottom of the car
                        bus->getX(),bus->getY()+2,bus->width(),bus->height())){
                gameCar->causeCrash();
            }
            else if (carCannon != 0 && (rectCollision(carCannon->getX(),carCannon->getY()-4,carCannon->width(),carCannon->height()+8, // 10 compensates for bottom of the car
                        bus->getX(),bus->getY(),bus->width(),bus->height()))){
                bus->setExplode(); // sets our boulder to explode!
                fprintf(stdout, "EXPLODING THE BUS!");
                delete carCannon;
                carCannon = 0;
            }
            ++busIter;
        }
        else if ( bus->isDead() ){
            busIter = gameBuses.erase(busIter);
        }
        else{
            ++busIter;
        }
    }

    std::vector<DamagedMan*>::iterator manIter = gameDamagedMen.begin();
    while ( manIter != gameDamagedMen.end() ){
        DamagedMan * man = *(manIter);
        man->update(scrollX, gameLevel);
        /
        if ( bus->isActive()){
            if ( rectCollision(gameCar->getX()+5,gameCar->getY(),gameCar->width()-5,gameCar->height()-10, // 10 compensates for bottom of the car
                        bus->getX(),bus->getY()+2,bus->width(),bus->height())){
                gameCar->causeCrash();
            }
            else if (carCannon != 0 && (rectCollision(carCannon->getX(),carCannon->getY()-4,carCannon->width(),carCannon->height()+8, // 10 compensates for bottom of the car
                        bus->getX(),bus->getY(),bus->width(),bus->height()))){
                bus->setExplode(); // sets our boulder to explode!
                fprintf(stdout, "EXPLODING THE BUS!");
                delete carCannon;
                carCannon = 0;
            }
            ++manIter;
        }

        else if ( bus->isDead() ){
            manIter = gameBuses.erase(manIter);
        }
        else{
            ++manIter;
        //}
    }

    std::vector<Boulder*>::iterator bldIter = gameBoulders.begin();
    while ( bldIter != gameBoulders.end() ){
        Boulder * bld = *(bldIter);
        bld->update(gameLevel, scrollX);
        if ( bld->isActive()){
            if ( rectCollision(gameCar->getX()+5,gameCar->getY(),gameCar->width()-5,gameCar->height()-10, // 10 compensates for bottom of the car
                        bld->getX(),bld->getY()+2,bld->width(),bld->height())){
                gameCar->causeCrash();
            }
            else if (carCannon != 0 && (rectCollision(carCannon->getX(),carCannon->getY()-4,carCannon->width(),carCannon->height()+8, // 10 compensates for bottom of the car
                        bld->getX(),bld->getY(),bld->width(),bld->height()))){
                bld->setExplode(); // sets our boulder to explode!
                delete carCannon;
                carCannon = 0;
            }
            ++bldIter;
        }
        else if ( bld->isDead() ){
            bldIter = gameBoulders.erase(bldIter);
        }
        else{
            ++bldIter;
        }
    }
*/

    actorPool->checkCollision(playerGroup, enemyGroup);
    actorPool->updateCollisions();

    // Timer
    currentTimeBuffer++;
    if ( currentTimeBuffer >= PHYSICSFPS ){
        currentTimeBuffer = 0;
        currentTime++;
    }
}

void Game::KeyInput()
{
    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {
        switch (event.type) {
        case SDL_QUIT:
            done=TRUE;			// If So done=TRUE
            break;
        default:
            break;
        }

        if (event.type == SDL_KEYDOWN)
        {
            switch(event.key.keysym.sym){
                case SDLK_LEFT:
                    gKeyLeft=TRUE;
                break;
                case SDLK_RIGHT:
                    gKeyRight=TRUE;
                break;
                case SDLK_UP:
                    gKeyUp=TRUE;
                break;
                case SDLK_DOWN:
                    gKeyDown=TRUE;
                break;
                case SDLK_ESCAPE:
                    done=TRUE;
                break;
                case SDLK_SPACE:
                    gKeySpace=TRUE;
                break;
                case SDLK_LCTRL:
                    gKeyCtrl=TRUE;
                break;
                default:

                break;
            }
        }
        if (event.type == SDL_KEYUP)
        {
            switch(event.key.keysym.sym){
                case SDLK_LEFT:
                    gKeyLeft=FALSE;
                break;
                case SDLK_RIGHT:
                    gKeyRight=FALSE;
                break;
                case SDLK_UP:
                    gKeyUp=FALSE;
                break;
                case SDLK_DOWN:
                    gKeyDown=FALSE;
                break;
                case SDLK_SPACE:
                    gKeySpace=FALSE;
                break;
                case SDLK_LCTRL:
                    gKeyCtrl=FALSE;
                break;
                default:
                break;
            }
        }
    }
}

void Game::Timer()
{
    if((SDL_GetTicks() - startTime) < 1000.0/PHYSICSFPS)
        SDL_Delay(int(1000.0/PHYSICSFPS - (SDL_GetTicks() - startTime)));

    startTime = SDL_GetTicks();
}

void Game::KillGLWindow(void)			// Properly Kill The Window
{
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:			*
 *	title			- Title To Appear At The Top Of The Window	*
 *	width			- Width Of The GL Window Or Fullscreen Mode	*
 *	height			- Height Of The GL Window Or Fullscreen Mode	*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)	*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL Game::CreateGLWindow(char* title, int width, int height, int bits, BOOL fullscreenflag)
{
  Uint32 flags;
  //int size;

  fullscreen=fullscreenflag;	// Set The Global Fullscreen Flag
  flags = SDL_OPENGL;
  if ( fullscreenflag ) {
    flags |= SDL_FULLSCREEN;
  }
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, bits);
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);

  //SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 );
  if ( SDL_SetVideoMode(width, height, bits, flags) == NULL ) {
    return FALSE;
  }

  SDL_WM_SetCaption(title, "opengl");
    SDL_ShowCursor(FALSE); // hide our cursor

  //ReSizeGLScene(width, height);		// Set Up Our Perspective GL Screen
  return TRUE;				// Success
}

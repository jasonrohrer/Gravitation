#include <SDL/SDL.h>


// these can be called once at beginning and end of app execution
// since loaded graphics can be reused for multiple games
void loadWorldGraphics();
void destroyWorldGraphics();
    

// these should be called at the beginning and end of each new game
void initWorld();
void destroyWorld();
        

Uint32 sampleFromWorld( int inX, int inY, double inWeight = 1.0 );

// sample a rectangle from the world
void sampleFromWorld( int inX, int inY, int inWidth, int inHeight,
                      Uint32 *outDestination );


void startHeartAnimation( int inX, int inY );

void setPlayerPosition( int inX, int inY );
void setPlayerSpriteFrame( int inFrame );



char isMezOnScreen();

void hideMez();

char isMezHidden();



void getMezPosition( int *outX, int *outY );

char mezHasBall();

// true if Mez still waiting before being ready to throw ball again
char mezWaiting();

void throwBall();

void getBallPosition( int *outX, int *outY );

void getBallVelocity( double *outDX, double *outDY );

void returnBallToMez();

char stepBall();


// is a prize present at a given pixel location
char isPrize( int inX, int inY );
// frozen prizes block player
char isBlockedByPrize( int inX, int inY );

// touches a prize and causes it to fall to bottom of world
void touchPrize( int inX, int inY );

double pushPrize( int inX, int inY, double inForce );

void stepPrizes();





// push animations forward one step
void stepAnimations();




int getTileWidth();


int getTileHeight();


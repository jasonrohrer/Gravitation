#include <SDL/SDL.h>

#include "common.h"
#include "Gradient.h"


// uses extern int mapWidth, mapHeight for size of particle field

class Particle{
    public:
        Gradient *mGradient;
        double mGradientPosition;
        double mDeltaGradientPosition;
        

        double mFade;
        double mDFade;
        
        char mAdditive;
        

        double mX, mY;
        
        double mDX, mDY;

        
        
    };


void addParticle( Particle inParticle );
        

void stepParticles();


// draws particles into an image
void drawParticles( double inFade, 
                    int inX, int inY, int inWidth, int inHeight,
                    Uint32 *inImage );

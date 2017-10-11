#include "particles.h"

#include "minorGems/util/SimpleVector.h"

#include <math.h>


extern int mapWidth;
extern int mapHeight;

SimpleVector<Particle> particleList;



void addParticle( Particle inParticle ) {
    particleList.push_back( inParticle );
    }


inline int clampZero255( int inValue ) {
    if( inValue > 255 ) {
        inValue = 255;
        }
    else if( inValue < 0 ) {
        inValue = 0;
        }
    
    return inValue;
    }
        


void stepParticles() {

    for( int i=0; i<particleList.size(); i++ ) {
        
        Particle *p = particleList.getElement( i );
        p->mX += p->mDX;
        p->mY += p->mDY;
        
        p->mFade += p->mDFade;
        

        p->mGradientPosition += p->mDeltaGradientPosition;
        if( p->mGradientPosition < 0.0 ) {
            p->mGradientPosition = 0.0;
            }
        if( p->mGradientPosition > 1.0 ) {
            p->mGradientPosition = 1.0;
            }
                
        int x = (int)round( p->mX );
        int y = (int)round( p->mY );
        
        if( x < 0 || x >= mapWidth 
            ||
            y < 0 || y >= mapHeight ||
            p->mFade <= 0 ) {
        
            // out of bounds or faded out
            // remove
            particleList.deleteElement( i );
            
            i--;
            }
        
        if( p->mFade > 1.0 ) {
            // cap
            p->mFade = 1.0;
            }
        }
    }



inline double clamp255( double inValue ) {
    if( inValue > 255 ) {
        inValue = 255;
        }
    return inValue;
    }



// draws particles into an image
void drawParticles( double inFade,
                    int inX, int inY, int inWidth, int inHeight,
                    Uint32 *inImage ) {
    
    for( int i=0; i<particleList.size(); i++ ) {
        
        Particle *p = particleList.getElement( i );
        
        int x = (int)round( p->mX );
        int y = (int)round( p->mY );
        
        if( x >= inX && 
            x < inX + inWidth && 
            y >= inY && 
            y < inY + inHeight ) {
            
            // particle on inImage

            // index to point on inImage
            int index = ( y - inY) * inWidth + ( x - inX );

            double particleFade = inFade * p->mFade;
            double imageFade;
            
            if( p->mAdditive ) {
                imageFade = 1;
                }
            else {
                imageFade = 1 - particleFade;
                }
            
        
            Pixel32 particlePixel, imagePixel, blendPixel;

            particlePixel = 
                rgbColorToPixel32( 
                    p->mGradient->sample( p->mGradientPosition ) );
            
            imagePixel.argbValue = inImage[index];
            
            // always clamp, in case we are in additive blend mode
            blendPixel.channels.r = 
                (unsigned char)( clamp255(
                                     particleFade * particlePixel.channels.r +
                                     imageFade * imagePixel.channels.r ) );
            blendPixel.channels.g = 
                (unsigned char)( clamp255(
                                     particleFade * particlePixel.channels.g +
                                     imageFade * imagePixel.channels.g ) );
            blendPixel.channels.b = 
                (unsigned char)( clamp255(
                                     particleFade * particlePixel.channels.b +
                                     imageFade * imagePixel.channels.b ) );
            
            inImage[ index ] = blendPixel.argbValue;
            }
        
        }
    }



#ifndef GRAPHIC_CONTAINER_INCLUDED
#define GRAPHIC_CONTAINER_INCLUDED

#include <SDL/SDL.h>

#include "common.h"

#include "minorGems/graphics/Image.h"



class GraphicContainer {
    
    public:
        
        GraphicContainer( const char *inTGAFileName ) {
        
            Image *image = readTGA( inTGAFileName );
    
            mW = image->getWidth();
            mH = image->getHeight();
            int imagePixelCount = mW * mH;
    
            mRed =  new double[ imagePixelCount ];
            mGreen =  new double[ imagePixelCount ];
            mBlue =  new double[ imagePixelCount ];
            
            mCompositePixels = new Uint32[ imagePixelCount ];
            
            for( int i=0; i<imagePixelCount; i++ ) {
                mRed[i] = 255 * image->getChannel(0)[ i ];
                mGreen[i] = 255 * image->getChannel(1)[ i ];
                mBlue[i] = 255 * image->getChannel(2)[ i ];
                
                mCompositePixels[i] = 
                    (int)mRed[i] << 16 
                    | (int)mGreen[i] << 8 
                    | (int)mBlue[i];
                }
            delete image;
            }
        
        
        ~GraphicContainer() {
            delete [] mRed;
            delete [] mGreen;
            delete [] mBlue;
            delete [] mCompositePixels;
            }
        
        
        double *mRed;
        double *mGreen;
        double *mBlue;
        
        Uint32 *mCompositePixels;
        

        int mW;
        int mH;
        
    
    };


#endif

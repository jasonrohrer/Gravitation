#include "score.h"
#include "common.h"


#include "minorGems/util/stringUtils.h"







ScoreDrawer::ScoreDrawer( const char *inNumeralFileName ) {
    
    numeralImage =  readTGA( (char *)inNumeralFileName );
    
    imagePixelCount = numeralImage->getWidth() * numeralImage->getHeight();
    
    numeralW = numeralImage->getWidth();
    numeralH = ( ( numeralImage->getHeight() + 1 ) / 10 ) - 1;
    

    numeralRed =  new double[ imagePixelCount ];
    numeralGreen =  new double[ imagePixelCount ];
    numeralBlue =  new double[ imagePixelCount ];
    
    numeralARGB = new Uint32[ imagePixelCount ];
    
    for( int i=0; i<imagePixelCount; i++ ) {
        numeralRed[i] = 255 * numeralImage->getChannel(0)[ i ];
        numeralGreen[i] = 255 * numeralImage->getChannel(1)[ i ];
        numeralBlue[i] = 255 * numeralImage->getChannel(2)[ i ];
        
        unsigned char r = 
            (unsigned char)( 
                numeralRed[ i ] );
    
        unsigned char g = 
            (unsigned char)( 
                numeralGreen[ i ] );
        
        unsigned char b = 
            (unsigned char)( 
                numeralBlue[ i ] );
            
        numeralARGB[i] = r << 16 | g << 8 | b;
        }
    }



int ScoreDrawer::getScoreHeight() {
    return numeralH;
    }



void ScoreDrawer::drawScore( Uint32 *inImage, int inWidth, int inHeight, 
                             int inScore ) {

    drawScore( inImage, inWidth, inHeight, inScore,
               inWidth, 0 );
    }



void ScoreDrawer::drawScore( Uint32 *inImage, int inWidth, int inHeight, 
                             int inScore, int inXOffset, int inYOffset,
                             int inZeroPadding ) {
    char *formatString;
    
    if( inZeroPadding > 0 ) {
        formatString = autoSprintf( "%%0%dd", inZeroPadding );
        }
    else {
        formatString = stringDuplicate( "%d" );
        }
    

    char *scoreString = autoSprintf( formatString, inScore );
    
    delete [] formatString;
    
    
    int numDigits = strlen( scoreString );
    
    int xPosition = inXOffset - numDigits * ( numeralW + 1 );
    
    for( int i=0; i<numDigits; i++ ) {
        char digit = scoreString[i];
        
        int digitNumber = digit - '0';
        
        for( int y=0; y<numeralH; y++ ) {
            int numeralY = y + digitNumber * ( numeralH + 1 );
            
            for( int x=0; x<numeralW; x++ ) {
                int imageX = x + xPosition;
                
                // copy pixels
                inImage[ ( y + inYOffset ) * inWidth + imageX ] =
                    numeralARGB[ numeralY * numeralW + x ];
                }
            }
        
        xPosition += numeralW + 1;
        }
    

    delete [] scoreString;
    

    }



ScoreDrawer::~ScoreDrawer() {
    delete numeralImage;
    
    
    delete [] numeralRed;
    delete [] numeralGreen;
    delete [] numeralBlue;

    delete [] numeralARGB;
    
    }

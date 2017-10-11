#include <SDL/SDL.h>


#include "minorGems/graphics/Image.h"


class ScoreDrawer {
        
    public:
        ScoreDrawer( const char *inNumeralFileName );
        
        ~ScoreDrawer();
        

        int getScoreHeight();



        // draws score in upper-right corner of image
        void drawScore( Uint32 *inImage, int inWidth, int inHeight, 
                        int inScore );
        

        // draws score in an image using a specific offset
        void drawScore( Uint32 *inImage, int inWidth, int inHeight, 
                        int inScore, int inXOffset, int inYOffset,
                        int inZeroPadding = 0 );
        

    private:
        
        Image *numeralImage;
        int numeralW;
        int numeralH;
        
        int imagePixelCount;

        double *numeralRed;
        double *numeralGreen;
        double *numeralBlue;
        
        Uint32 *numeralARGB;

        
    };

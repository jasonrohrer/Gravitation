#include <stdio.h>

#include "minorGems/graphics/Image.h"

#include "minorGems/graphics/converters/TGAImageConverter.h"

#include "minorGems/io/file/File.h"

#include "minorGems/io/file/FileOutputStream.h"

#include "Gradient.h"

int main() {

    double anchors[4] = 
        { 0.0, 0.3333, 0.6666, 1.0 };

    rgbColor colors[4] = 
        { 
            { 0.71, 0.28, 0.06 },
            { 0.96, 0.55, 0.18 },
            { 0.95, 0.84, 0.43 },
            { 1.0, 0.98, 0.84 }
            };

    Gradient grad( 4, anchors, colors );
    

    
    int numSteps = 9;


    Image outImage( 1, numSteps + 1, 3 );
    
    double *rChan = outImage.getChannel( 0 );
    double *gChan = outImage.getChannel( 1 );
    double *bChan = outImage.getChannel( 2 );
    


    for( int i=0; i<=numSteps; i++ ) {
        rgbColor color = grad.sample( i / (double)numSteps );
        
        rChan[i] = color.r;
        gChan[i] = color.g;
        bChan[i] = color.b;
        }


    File tgaFile( NULL, "genColors.tga" );
    FileOutputStream tgaStream( &tgaFile );
    
    TGAImageConverter converter;
    
    converter.formatImage( &outImage, &tgaStream );

    }

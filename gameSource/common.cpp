#include "common.h"

#include "minorGems/graphics/converters/TGAImageConverter.h"

#include "minorGems/io/file/File.h"

#include "minorGems/io/file/FileInputStream.h"
#include "minorGems/io/file/FileOutputStream.h"



Image *readTGA( const char *inFileName ) {
    return readTGA( "graphics", inFileName );
    }



Image *readTGA( const char *inFolderName, const char *inFileName ) {
    File tgaFile( new Path( inFolderName ), inFileName );
    FileInputStream tgaStream( &tgaFile );
    
    TGAImageConverter converter;
    
    return converter.deformatImage( &tgaStream );
    }



void writeTGAFromRGBA( Uint32 *inImage, int inWidth, int inHeight, 
                       const char *inFileName ) {
    int numPixels = inWidth * inHeight;
    
    Image outImage( inWidth, inHeight, 3 );
    
    double *r = outImage.getChannel( 0 );
    double *g = outImage.getChannel( 1 );
    double *b = outImage.getChannel( 2 );
    
    for( int i=0; i<numPixels; i++ ) {
        r[i] = ( inImage[i] >> 16 & 0xFF ) / 255.0;
        g[i] = ( inImage[i] >> 8 & 0xFF ) / 255.0;
        b[i] = ( inImage[i] & 0xFF ) / 255.0;
        }
    
    File tgaFile( NULL, inFileName );
    FileOutputStream tgaStream( &tgaFile );
    
    TGAImageConverter converter;
    
    converter.formatImage( &outImage, &tgaStream );
    }



Pixel32 rgbColorToPixel32( rgbColor inColor ) {
    
    Pixel32 p;
    p.channels.a = 0;
    p.channels.r = (unsigned char)( inColor.r * 255 );
    p.channels.g = (unsigned char)( inColor.g * 255 );
    p.channels.b = (unsigned char)( inColor.b * 255 );

    return p;
    }

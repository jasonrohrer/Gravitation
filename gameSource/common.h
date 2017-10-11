#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED



#include "minorGems/graphics/Image.h"

#include <SDL/SDL.h>

// reads a TGA file from the default ("graphics") folder
Image *readTGA( const char *inFileName );


Image *readTGA( const char *inFolderName, const char *inFileName );


void writeTGAFromRGBA( Uint32 *inImage, int inWidth, int inHeight, 
                       const char *inFileName );


struct rgbColorStruct {
    double r;
    double g;
    double b;
    };
typedef struct rgbColorStruct rgbColor;



#include "minorGems/system/endian.h"



// use struct/union data type for fast 
// access to color components without invoking shift operator
// Found in Gamasutra article about fast alpha blending

#if __BYTE_ORDER ==  __LITTLE_ENDIAN
struct ColorChannels {
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned char a;
    };
#else
struct ColorChannels {
        unsigned char a;
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };
#endif


union Pixel32 {
        ColorChannels channels;
        Uint32 argbValue;
    };



// convert from rgbColor tp Pixel32
Pixel32 rgbColorToPixel32( rgbColor inColor );



#endif

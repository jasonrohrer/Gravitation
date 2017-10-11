#ifndef GRADIENT_INCLUDED
#define GRADIENT_INCLUDED

#include "common.h"

#include "minorGems/util/SimpleVector.h"



class Gradient {
    public:
        
        Gradient( int inNumPoints, double *inPointAnchors,
                  rgbColor *inPointColors );
        
        // samples from location in range [0,1] along gradient
        rgbColor sample( double inLocation );
        
    private:
        SimpleVector<double> mAnchors;
        SimpleVector<rgbColor> mColors;
    };


#endif

#include "Gradient.h"



Gradient::Gradient( int inNumPoints, double *inPointAnchors,
                    rgbColor *inPointColors ) {
    
    for( int i=0; i<inNumPoints; i++ ) {
        mAnchors.push_back( inPointAnchors[i] );
        mColors.push_back( inPointColors[i] );
        }
    }


        
rgbColor Gradient::sample( double inLocation ) {
    
    // method copied from minorGems/temp/flame/realtimeFlame.cpp

    int numPoints = mAnchors.size();

    // linear blend of two points that surround inLocation
    int closestPointAbove;
    int closestPointBelow;
    char foundPoints = false;
    for( int i=0; !foundPoints && i<numPoints-1; i++ ) {
        if( *( mAnchors.getElement( i ) ) <= inLocation &&
            *( mAnchors.getElement( i + 1 ) ) >= inLocation ) {
            closestPointAbove = i+1;
            closestPointBelow = i;
            foundPoints = true;
            }
        }

    double positionAbove = *( mAnchors.getElement( closestPointAbove ) );
    double positionBelow = *( mAnchors.getElement( closestPointBelow ) );
    double spread = positionAbove - positionBelow;
    
    rgbColor colorAbove = *( mColors.getElement( closestPointAbove ) );
    rgbColor colorBelow = *( mColors.getElement( closestPointBelow ) );

    // avoid divide-by-zero below
    if( spread == 0 ) {
        return colorAbove;
        }


    double weightAbove = (inLocation - positionBelow) /  spread;
    double weightBelow = (positionAbove - inLocation) /  spread;
    

    rgbColor returnColor;
    
    returnColor.r = weightAbove * colorAbove.r + weightBelow * colorBelow.r;
    returnColor.g = weightAbove * colorAbove.g + weightBelow * colorBelow.g;
    returnColor.b = weightAbove * colorAbove.b + weightBelow * colorBelow.b;
    
    return returnColor;
    }


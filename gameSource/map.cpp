#include "map.h"


extern int tileH;
extern int tileW;


#include "GraphicContainer.h"

GraphicContainer *mapContainer;



void loadMapGraphics() {
    mapContainer = new GraphicContainer( "map.tga" );
    }



void destroyMapGraphics() {
    delete mapContainer;
    }



char isBlocked( int inX, int inY ) {
    // reduce to grid coordinates
    int gridX = inX / tileW;
    int gridY = inY / tileH;
    

    // blocked outside map
    if( gridX < 0 || gridX >= mapContainer->mW ) {
        return true;
        }
    if( gridY < 0 || gridY >= mapContainer->mH ) {
        return true;
        }
    else {
        char blocked =
            mapContainer->mRed[ gridY * mapContainer->mW + gridX ] == 0;
        
        return blocked;
        } 
    }



int getMapWidth() {
    return mapContainer->mW * tileW;
    }
            
int getMapHeight() {
    return mapContainer->mH * tileH;
    }



char isPrizeStartingSpot( int inGridX, int inGridY ) {
    // blocked outside map
    if( inGridX < 0 || inGridX >= mapContainer->mW ) {
        return false;
        }
    if( inGridY < 0 || inGridY >= mapContainer->mH ) {
        return false;
        }
    else {
        return 
            mapContainer->mRed[ inGridY * mapContainer->mW + inGridX ] != 0
            &&
            mapContainer->mGreen[ inGridY * mapContainer->mW + inGridX ] != 0;
        }
    }


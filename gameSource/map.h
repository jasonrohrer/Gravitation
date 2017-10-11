// these can be called once at beginning and end of app execution
// since loaded graphics can be reused for multiple games
void loadMapGraphics();
void destroyMapGraphics();


int getMapWidth();
int getMapHeight();


// checks if position is blocked by wall
char isBlocked( int inX, int inY );


// checks if a given location is the starting spot for a prize
char isPrizeStartingSpot( int inGridX, int inGridY );

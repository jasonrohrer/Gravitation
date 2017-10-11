/*
 * Modification History
 *
 * 2007-September-25   Jason Rohrer
 * Created.  
 */



#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


// for memcpy
#include <string.h>


// let SDL override our main function with SDLMain
#include <SDL/SDL_main.h>

// must do this before SDL include to prevent WinMain linker errors on win32
int mainFunction( int inArgCount, char **inArgs );

int main( int inArgCount, char **inArgs ) {
    return mainFunction( inArgCount, inArgs );
    }


#include <SDL/SDL.h>

#include "blowUp.h"
#include "World.h"
#include "map.h"
#include "Gradient.h"
#include "particles.h"
#include "score.h"
#include "common.h"
#include "musicPlayer.h"

#include "minorGems/system/Time.h"
#include "minorGems/system/Thread.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/random/StdRandomSource.h"


StdRandomSource randSource;



// size of game image
int width = 99;
int height = 90;

ScoreDrawer *scoreDrawer;

// area above game image for score
int scoreHeight = 5;

double totalTime = 480.0;

double timeLeft = totalTime;


ScoreDrawer *timeLeftDrawer;


// size of game image plus scoreboard at top and time board at bottom
int totalImageHeight = height + scoreHeight + scoreHeight;



// size of screen for fullscreen mode
int screenWidth = 640;
int screenHeight = 480;
//int screenWidth = 100;
//int screenHeight = 16;

// blow-up factor when projecting game onto screen
// Max defined by image size vs screen size.
// This is the cap for in-game user-directed blowup adjustment.
int maxBlowUpFactor;

// current blow-up setting
int blowUpFactor;


// step to take when user hits blowUp key
int blowUpStep = -1;
// flag to force update of entire screen
int blowUpChanged = true;


char fullScreen = true;


// lock down to 15 fps
int lockedFrameRate = 15;


// target length of game
int gameTime = 8 * 60;
//int gameTime = 30;


// timeLeft that passes per frame
double timeDelta = - ( timeLeft / ( gameTime * lockedFrameRate ) );
//double timeDelta = -0.0;









// the joystick to read from, or NULL if there's no available joystick
SDL_Joystick *joystick;



// catch an interrupt signal
void catch_int(int sig_num) {
	printf( "Quiting...\n" );
    SDL_Quit();
	exit( 0 );
	signal( SIGINT, catch_int );
	}



char getKeyDown( int inKeyCode ) {
    SDL_PumpEvents();
	Uint8 *keys = SDL_GetKeyState( NULL );
	return keys[ inKeyCode ] == SDL_PRESSED;
    }



char getHatDown( Uint8 inHatPosition ) {
    if( joystick == NULL ) {
        return false;
        }
    
    SDL_JoystickUpdate();

    Uint8 hatPosition = SDL_JoystickGetHat( joystick, 0 );
    
    if( hatPosition & inHatPosition ) {
        return true;
        }
    else {
        return false;
        }
    }


int joyThreshold = 25000;

char getJoyPushed( Uint8 inHatPosition ) {
        
    Sint16 x = SDL_JoystickGetAxis(joystick, 0);
    Sint16 y = SDL_JoystickGetAxis(joystick, 1);
    
    switch( inHatPosition ) {
        case SDL_HAT_DOWN:
            return  y > joyThreshold;
            break;
        case SDL_HAT_UP:
            return  y < -joyThreshold;
            break;
        case SDL_HAT_LEFT:
            return  x < - joyThreshold;
            break;
        case SDL_HAT_RIGHT:
            return  x > joyThreshold;
            break;
        default:
            return false;
        }
            
        
    }



// returns true if hit, returns false if user quits before hitting
char waitForKeyOrButton() {
    SDL_Event event;
    
    while( true ) {
        while( SDL_WaitEvent( &event ) ) {
            switch( event.type ) {
                case SDL_JOYHATMOTION:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    return true;
                    break;
                // watch for quit event
                case SDL_QUIT:
                    return false;
                    break;
                default:
                    break;
                }
            }
        }
    
    
    return false;
    }



Uint32 *gameImage;



// flips back buffer onto screen (or updates rect)
void flipScreen( SDL_Surface *inScreen ) {

    // unlock the screen if necessary
    if( SDL_MUSTLOCK( inScreen ) ) {
        SDL_UnlockSurface( inScreen );
        }
    
    if( ( inScreen->flags & SDL_DOUBLEBUF ) == SDL_DOUBLEBUF ) {
        // need to flip buffer
        SDL_Flip( inScreen );	
        }
    else if( !blowUpChanged ) {
        // just update center

        // small area in center that we actually draw in, black around it
        int yOffset = ( inScreen->h - totalImageHeight * blowUpFactor ) / 2;
        int xOffset = ( inScreen->w - width * blowUpFactor ) / 2;
        
        SDL_UpdateRect( inScreen, xOffset, yOffset, 
                        width * blowUpFactor, 
                        totalImageHeight * blowUpFactor );	
        }
    else {
        // update the whole thing
        SDL_UpdateRect( inScreen, 0, 0, inScreen->w, inScreen->h );
        
        // reset flag
        blowUpChanged = false;
        }
    
    }



void lockScreen( SDL_Surface * inScreen ) {
    // check if we need to lock the screen
    if( SDL_MUSTLOCK( inScreen ) ) {
        if( SDL_LockSurface( inScreen ) < 0 ) {
            printf( "Couldn't lock screen: %s\n", SDL_GetError() );
            }
        }
    }



void flipGameImageOntoScreen( SDL_Surface *inScreen ) {
        
    if( blowUpChanged 
	&&
	( inScreen->flags & SDL_DOUBLEBUF ) == SDL_DOUBLEBUF ) {
   
        // blow up size has changed, and we are double-buffered
        // flip onto screen an additional time.
        // This will cause us to black-out the background in both buffers.

        lockScreen( inScreen );
	
	// when blow-up factor changes:
	// clear screen to prepare for next draw, 
	// which will be bigger or smaller
	SDL_FillRect( inScreen, NULL, 0x00000000 );
        
	blowupOntoScreen( gameImage, 
			  width, totalImageHeight, blowUpFactor, inScreen );
    
	flipScreen( inScreen );
        }

    lockScreen( inScreen );
    if( blowUpChanged ) {
        SDL_FillRect( inScreen, NULL, 0x00000000 );
        }
    blowupOntoScreen( gameImage, 
		      width, totalImageHeight, blowUpFactor, inScreen );
    
    flipScreen( inScreen );

    // we've handled any blow up change
    blowUpChanged = false;
    }







SDL_Surface *screen = NULL;


// play a complete game, from title screen to end, on screen
// returns false if player quits
char playGame();



void createScreen() {
    if( screen != NULL ) {
        // destroy old one first
        SDL_FreeSurface( screen );
        }

    int displayW = width * blowUpFactor;
    int displayH = totalImageHeight * blowUpFactor;

    
    Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
    if( fullScreen ) {
        flags = flags | SDL_FULLSCREEN;
    
        // use letterbox mode in full screen
        displayW = screenWidth;
        displayH = screenHeight;
        }
    
    printf( "Creating %dx%d screen for a %dx%d game image\n",
            displayW, displayH, 
            width * blowUpFactor, totalImageHeight * blowUpFactor );
    

    screen = SDL_SetVideoMode( displayW, displayH, 
                               32, 
                               flags );
    
    if ( screen == NULL ) {
        printf( "Couldn't set %dx%dx32 video mode: %s\n", displayW, 
                displayH,
                SDL_GetError() );
        }
    }



int mainFunction( int inArgCount, char **inArgs ) {

    // let catch_int handle interrupt (^c)
    signal( SIGINT, catch_int );


    Uint32 initFlags = 
        SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE;
    
    #ifdef __mac__
        // SDL_Init is dreadfully slow if we try to init JOYSTICK on MacOSX
        // not sure why---maybe it's searching all devices or something like 
        // that.
        // Couldn't find anything online about this.
        initFlags = 
	    SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
    #endif
    
    if( SDL_Init( initFlags ) < 0 ) {
        printf( "Couldn't initialize SDL: %s\n", SDL_GetError() );
        return -1;
        }
    
    SDL_ShowCursor( SDL_DISABLE );


    // read screen size from settings
    char widthFound = false;
    int readWidth = SettingsManager::getIntSetting( "screenWidth", 
                                                    &widthFound );
    char heightFound = false;
    int readHeight = SettingsManager::getIntSetting( "screenHeight", 
                                                    &heightFound );
    
    if( widthFound && heightFound ) {
        // override hard-coded defaults
        screenWidth = readWidth;
        screenHeight = readHeight;
        }
    
    printf( "Screen dimensions for fullscreen mode:  %dx%d\n",
            screenWidth, screenHeight );
    
    // set maxBlowUpFactor here, since screenWidth and screenHeight may 
    // have changed from default
    
    // don't assume w > height
    if( width >= totalImageHeight ) {
        maxBlowUpFactor = screenWidth / width;
        }
    else {
        maxBlowUpFactor = screenHeight / totalImageHeight;
        }
    
    // default to max
    blowUpFactor = maxBlowUpFactor;


    char fullscreenFound = false;
    int readFullscreen = SettingsManager::getIntSetting( "fullscreen",
                                                         &fullscreenFound );
    if( fullscreenFound ) {
        fullScreen = readFullscreen;
        }
    
    printf( "Starting game in " );
    if( fullScreen ) {
        printf( "fullscreen" );
        }
    else {
        printf( "windowed" );
        }
    printf( " mode.\n" );
    


    createScreen();
    

    // try to open joystick
    int numJoysticks = SDL_NumJoysticks();
    printf( "Found %d joysticks\n", numJoysticks );
    
    if( numJoysticks > 0 ) {
        // open first one by default
        joystick = SDL_JoystickOpen( 0 );
    
        if( joystick == NULL ) {
	    printf( "Couldn't open joystick 0: %s\n", SDL_GetError() );
            }
        int numHats = SDL_JoystickNumHats( joystick );
        
        if( numHats <= 0 ) {
            printf( "No d-pad found on joystick\n" );
            SDL_JoystickClose( joystick );
            joystick = NULL;
            }
        }
    else {
        joystick = NULL;
        }


    
    #ifdef __mac__
        // make sure working directory is the same as the directory
        // that the app resides in
        // this is especially important on the mac platform, which
        // doesn't set a proper working directory for double-clicked
        // app bundles
    
        // arg 0 is the path to the app executable
        char *appDirectoryPath = stringDuplicate( inArgs[ 0 ] );

        printf( "Mac:  app path %s\n", appDirectoryPath );

        char *appNamePointer = strstr( appDirectoryPath,
                                       "Gravitation.app" );

        if( appNamePointer != NULL ) {
            // terminate full app path to get parent directory
            appNamePointer[0] = '\0';
            
            printf( "Mac: changing working dir to %s\n", appDirectoryPath );
            chdir( appDirectoryPath );
            }
        
        delete [] appDirectoryPath;
    #endif



    // load graphics only once, after setting current working dir, if needed
    scoreDrawer = new ScoreDrawer( "numerals.tga" );
    timeLeftDrawer = new ScoreDrawer( "numeralsYellow.tga" );
        
    loadMapGraphics();
        
    loadWorldGraphics();
    

    setMusicLoudness( 0 );
    startMusic( "music.tga" );
    
    // keep playing until player quits
    while( playGame() ) {
        }
    
    stopMusic();
    

    destroyWorldGraphics();
    destroyMapGraphics();
    
    delete scoreDrawer;
    delete timeLeftDrawer;
    

    if( joystick != NULL ) {
        SDL_JoystickClose( joystick );
        }
    
    SDL_Quit();

    return 0;
    }



char isBlockedOrPrize( int inX, int inY ) {
    return isBlocked( inX, inY ) || isBlockedByPrize( inX, inY );
    }


int score = 0;


// 1 = manic
// 0 = depressed
double playerEmotion = 0.4;


// change in player emotion per frame
//double defaultDeltaPlayerEmotion = -0.0005;
double defaultDeltaPlayerEmotion = -0.0010;

double deltaPlayerEmotion = defaultDeltaPlayerEmotion;

// used during natural depression recovery
double upswingDeltaPlayerEmotion = 0;

// upswing 2 times faster that base downswing rate
// ** Actually, since downswing rate increased, these rates are now equal
// thus, if you're stuck away from catch-with-Mez, you don't spend
// the rest of the game waiting for the natural upswing to complete
//double defaultUpswingAmount = 0.0010;
double defaultUpswingAmount = 0.0010;


// used to adjust player motion smoothly toward a target value
// (to avoid abrupt jumps in emotion)
// -1 means no current target
double playerEmotionSmoothTransitionTarget = -1;
double smoothTransitionDelta = 0.02;



// gradient used for hair fire
double fireGradientAnchors[4] = 
{ 0.0, 0.3333, 0.6666, 1.0 };

rgbColor fireGradientColors[4] = 
{ 
    { 0.71, 0.28, 0.06 },
    { 0.96, 0.55, 0.18 },
    { 0.95, 0.84, 0.43 },
    { 1.0, 0.98, 0.84 }
    };

Gradient fireGradient( 4, fireGradientAnchors, fireGradientColors );



char playGame() {
        

    // true == right
    char playerFacing = true;
    
    char playerPushing = false;
    

    int currentSpriteIndex = 2;

    double playerX, playerY;
    
    playerEmotion = 0.4;
    
    deltaPlayerEmotion = defaultDeltaPlayerEmotion;
    
    upswingDeltaPlayerEmotion = 0;
    
    timeLeft = totalTime;
    

    double maxPlayerX = 0;

    score = 0;


    
    initWorld();
    
    
        
    
    // room at top for score
    int totalGameImagePixels = width * totalImageHeight;
    
    gameImage = new Uint32[ totalGameImagePixels ];
    
    int i;
    
    // fill with black
    for( i=0; i<totalGameImagePixels; i++ ) {
        gameImage[i] = 0;
        }
    

    // first, fill the whole thing with black
    // SDL_FillRect( screen, NULL, 0x00000000 );

    
    

    double dX = 0;
    double dY = 0;

    // to track falling and jumping
    double verticalVelocity = 0;
    
    
    
    char done = false;
    

    // start player position
    playerX = getTileWidth() * 6;
    maxPlayerX = playerX;
    
    dX = 0;
    playerY = getMapHeight();
    // scoot up until we hit the lowest floor
    while( isBlockedOrPrize( (int)( playerX ) - 2, (int)floor( playerY ) )
           ||
           isBlockedOrPrize( (int)( playerX ) + 2, (int)floor( playerY ) ) ) {
        playerY --;
        }
    
    dY = playerY - height / 2;
    
    setPlayerPosition( (int)playerX, (int)playerY );
    setPlayerSpriteFrame( currentSpriteIndex );
    

    double lastFrameTimeStamp = Time::getCurrentTime();    

    // first, flip title onto screen
    Image *titleImage = readTGA( "title.tga" );
    int numTitlePixels = titleImage->getWidth() * titleImage->getHeight();
    
    double *titleRed = titleImage->getChannel( 0 );
    double *titleGreen = titleImage->getChannel( 1 );
    double *titleBlue = titleImage->getChannel( 2 );
    
    Uint32 *titlePixels = new Uint32[ numTitlePixels ];
    
    for( int i=0; i<numTitlePixels; i++ ) {
        titlePixels[ i ] =
            (unsigned char )( titleRed[i] * 255 ) << 16
            |
            (unsigned char )( titleGreen[i] * 255 ) << 8
            |
            (unsigned char )( titleBlue[i] * 255 );
        }
    
    // fill screen with title
    memcpy( gameImage, titlePixels, 4 * numTitlePixels );
    

    flipGameImageOntoScreen( screen );
    

    char hit = waitForKeyOrButton();
    
    if( !hit ) {
        // no key pressed...
        // maybe they closed the window
        // count as a "quit"
        done = true;
        }
    else {
        // they pressed a key to start the game
        
        // return to start
        restartMusic();
        
        // turn up music
        setMusicLoudness( 1.0 );
        }
    
        
    

    // fill with black to cover up title
    for( i=0; i<totalGameImagePixels; i++ ) {
        gameImage[i] = 0;
        }



    int frameCount = 0;
    unsigned long startTime = time( NULL );

    char quit = false;
    
    int titleFadeFrame = 0;
    // 6-second fade
    int numTitleFadeFrames = 90;

    // 3-second hold before checking keys to start next game
    // prevents game from starting instantly if player was holding down
    // movement keys during the fade-out (gives them a chance to let go
    // of the movement keys
    int numTitleHoldFrames = 45;
    

    char gameOver = false;
    

    while( !done ) {
        
        int startScreenX = (int)floor( dX );
        int startScreenY = (int)floor( dY );
        
        // make sure screen doesn't slide off top/bottom of map
        if( startScreenY < 0 ) {
            startScreenY = 0;
            }
        if( startScreenY + height >= getMapHeight() ) {
            startScreenY -= 
                ( startScreenY + height ) - getMapHeight();
            }
        

        sampleFromWorld( startScreenX, startScreenY, 
                         width, height, 
                         // skip scoreboard
                         &( gameImage[ scoreHeight * width ] ) );
        
        // draw particle layer, using emotion as fade factor
        // only draw them when emotions > 0.9
        if( playerEmotion >= 0.9 ) {
            
            drawParticles( ( playerEmotion - 0.9 ) / 0.1,
                           startScreenX, startScreenY, 
                           width, height,
                           // skip scoreboard
                           &( gameImage[ scoreHeight * width ] ) );
            }
        
        
        // draw black, except clear box around player, depending on emotion
        
        Uint32 *gameImageAfterScoreboard = 
            &( gameImage[ scoreHeight * width ] );
        

        // int borderWidth = (int)( 40 - playerEmotion * 40 );

        double clearRadius = 10 + playerEmotion * 40;
        
        double clearCenterX = (int)playerX;
        double clearCenterY = (int)playerY - startScreenY - 4;
        
        // prevent clear box from going off edge of screen
        if( clearCenterX - clearRadius < 0 ) {
            clearCenterX = clearRadius;
            }
        if( clearCenterX + clearRadius > width - 1 ) {
            clearCenterX = width - 1 - clearRadius;
            }
        if( clearCenterY - clearRadius < 0 ) {
            clearCenterY = clearRadius;
            }
        if( clearCenterY + clearRadius > height - 1 ) {
            clearCenterY = height - 1 - clearRadius;
            }
            
        

        for( int y=0; y<height; y++ ) {
            double distY;
            
            if( y >= clearCenterY ) {
                distY = y - clearCenterY;
                }
            else {
                distY = clearCenterY - y;
                }
            
            for( int x=0; x<width; x++ ) {
                int index = y * width + x;

                double distX;
                
                if( x >= clearCenterX ) {
                    distX = x - clearCenterX;
                    }
                else {
                    distX = clearCenterX - x;
                    }
                
                double maxDist = distY;
                
                if( maxDist < distX ) {
                    maxDist = distX;
                    }
                
                if( maxDist > clearRadius + 1 ) {
                    
                    gameImageAfterScoreboard[ index ] = 0;
                    }
                else if( maxDist > clearRadius ) {
                    // fuzzy border to smooth transitions as border changes
                    double fuzzFactor = 1.0 - ( maxDist - clearRadius );

                    if( fuzzFactor < 1.0 ) {
                                                
                        // right on border
                        // darken according to fuzz factor
                        Uint32 pixel = gameImageAfterScoreboard[ index ];
                        int r = pixel >> 16 & 0xFF;
                        int g = pixel >> 8 & 0xFF;
                        int b = pixel & 0xFF;
                        
                        r = (int)( r * fuzzFactor );
                        g = (int)( g * fuzzFactor );
                        b = (int)( b * fuzzFactor );
                        
                        gameImageAfterScoreboard[ index ] = 
                            r << 16 | g << 8 | b;
                        }
                    }
                }            
            }
        
        
        // center above/below kiln
        int scoreX = 86;
        

        scoreDrawer->drawScore( gameImage, width, height, score,
                               scoreX, 0,
                               // zero pad to keep centered
                               3 );
        

        // now draw remaining fire time at bottom of screen

        int timerY = totalImageHeight - scoreHeight + 1;
        
        // zero padded, so we don't need to worry about covering up old
        // digits as our count shrinks
        timeLeftDrawer->drawScore( gameImage, width, height, (int)timeLeft,
                                  scoreX, timerY,
                                  // zero pad
                                  3 );
        

        if( gameOver ) {
            // fade to title screen
            double titleWeight = 
                titleFadeFrame / (double)( numTitleFadeFrames - 1 );
            
            // now that we hold title after fade, titleFadeFrame will
            // eventually be larger than numTitleFadeFrames
            // Don't let titleWeight go over 1
            if( titleWeight > 1 ) {
                titleWeight = 1;
                }
            
            
            double gameWeight = 1 - titleWeight;
            
            // wipe from left to right during fade
            // int wipePosition = (int)( titleWeight * width );
            
            // don't wipe
            int wipePosition = width;
            

            // fade out music while we do it
            // music already faded out during last 5th of game
            // setMusicLoudness( 1.0 - titleWeight );
            

            
            for( i=0; i<totalGameImagePixels; i++ ) {
                

                Uint32 gamePixel = gameImage[i];
                
                unsigned char gameRed = gamePixel >> 16 & 0xFF;
                unsigned char gameGreen = gamePixel >> 8 & 0xFF;
                unsigned char gameBlue = gamePixel & 0xFF;
                
                Uint32 titlePixel = titlePixels[i];
                
                unsigned char titleRed = titlePixel >> 16 & 0xFF;
                unsigned char titleGreen = titlePixel >> 8 & 0xFF;
                unsigned char titleBlue = titlePixel & 0xFF;
            
                unsigned char red = 
                    (unsigned char)( 
                        gameWeight * gameRed + titleWeight * titleRed );
                unsigned char green = 
                    (unsigned char)( 
                        gameWeight * gameGreen + titleWeight * titleGreen );
                unsigned char blue = 
                    (unsigned char)( 
                        gameWeight * gameBlue + titleWeight * titleBlue );
                

                int x = i % width;
                if( x <= wipePosition ) {
                    gameImage[i] = red << 16 | green << 8 | blue;
                    }
                
                }
            
            titleFadeFrame ++;
            
            if( titleFadeFrame == numTitleFadeFrames + numTitleHoldFrames ) {
                done = true;
                }
            }
            
        
        flipGameImageOntoScreen( screen );
        

        // done with frame
        double newTimestamp = Time::getCurrentTime();
        
        double frameTime = newTimestamp - lastFrameTimeStamp;
        
        double extraTime = 1.0 / lockedFrameRate - frameTime;
        
        if( extraTime > 0 ) {
            Thread::staticSleep( (int)( extraTime * 1000 ) );
            }

        // start timing next frame
        lastFrameTimeStamp = Time::getCurrentTime();
        


        
        int moveDelta = 1;
        
        
        if( getKeyDown( SDLK_b ) ) {
            // adjust blowup factor
            blowUpFactor += blowUpStep;
            
            if( blowUpFactor > maxBlowUpFactor ) {
                blowUpStep *= -1;
                blowUpFactor = maxBlowUpFactor - 1;
                }
            
            if( blowUpFactor < 1 ) {
                blowUpStep *= -1;
                blowUpFactor = 2;
                }

            if( fullScreen ) {                
                // force redraw of whole screen
                blowUpChanged = true;
                }
            else {
                // create a new screen using the new size
                createScreen();
                }

            }

        if( getKeyDown( SDLK_f ) ) {
            // toggle fullscreen mode
            fullScreen = ! fullScreen;
            
            // create a new screen surface (and destroy old one)
            createScreen();
            }
        
            
        playerPushing = false;
        

            
        if( getKeyDown( SDLK_LEFT ) || getJoyPushed( SDL_HAT_LEFT ) ) {
            char notBlocked = 
                !isBlockedOrPrize( (int)( playerX - 2 - moveDelta ), 
                                   (int)playerY )
                &&
                !isBlockedOrPrize( (int)( playerX - 2 - moveDelta ), 
                            (int)playerY - 7 );
            
                    
            if( notBlocked ) {
                
                playerX -= moveDelta;
                
                if( playerX < 0 ) {
                    // undo
                    playerX += moveDelta;
                    }
                else {
                    // update screen position
                    //dX -= moveDelta;
                    
                    // pick sprite frame based on position in world
                    if( ( (int)playerX / 2 ) % 2 == 0 ) {
                        currentSpriteIndex = 6;
                        }
                    else {
                        currentSpriteIndex = 7;
                        }                 
                    // left
                    playerFacing = false;
                    }
                }
            
            // don't ever need to show pushing animations to left,
            // since we can only push prizes right.

            }
        else if( getKeyDown( SDLK_RIGHT ) || getJoyPushed( SDL_HAT_RIGHT )) {
            char pushingPrize = false;
            
            // push a prize if there is one
                
            // push harder if emotion > 0.90 (in mania)
            // Thus, in full mania, we can push 4 blocks as easily as 1
            // plus, we can push a single block twice as fast
            // (Blocks never slide more than 1 pixel per frame,
            //  but their base rate is 0.5 pixels per frame.)
            double extraForce = 0;
            if( playerEmotion > 0.90 ) {
                extraForce = 4 * ( ( playerEmotion - 0.90) / 0.10 );
                }
                
            double moveAmount = 
                pushPrize( (int)( playerX + 1 + moveDelta ), 
                           (int)playerY, 
                           0.5 + extraForce );

            if( moveAmount > 0 ) {
                pushingPrize = true;
                }

            
            // now check if we're blocked
            char notBlocked = 
                !isBlockedOrPrize( (int)( playerX + 1 + moveDelta ), 
                                   (int)playerY )
                &&
                !isBlockedOrPrize( (int)( playerX + 1 + moveDelta ), 
                            (int)playerY - 7);
            



            if( notBlocked ) {
                
                //dX += moveDelta;
                
                playerX += moveDelta;

                // pick sprite frame based on position in world
                if( ( (int)playerX / 2 ) % 2 == 0 ) {
                    currentSpriteIndex = 3;
                    }
                else {
                    currentSpriteIndex = 2;
                    }
                
                // right
                playerFacing = true;                    
                }
            

            // only show pushing animation if we're blocked by a prize
            // or pushing a prize (i.e., not when we're pushing
            // against a wall)
            char blockedByPrize = 
                isBlockedByPrize( (int)( playerX + 1 + moveDelta ), 
                                   (int)playerY )
                ||
                isBlockedByPrize( (int)( playerX + 1 + moveDelta ), 
                            (int)playerY - 7);

            if( blockedByPrize || pushingPrize ) {
                // pushing against something, or a prize is moving 
                // out of our way
                
                // pick sprite frame based on position in world
                if( ( (int)playerX ) % 2 == 0 ) {
                    currentSpriteIndex = 0;
                    }
                else {
                    currentSpriteIndex = 1;
                    }

                playerPushing = true;
                }
            
            }
        else {
            // not moving
            // switch to standing sprite
            if( playerFacing ) {
                currentSpriteIndex = 3;
                }
            else {
                currentSpriteIndex = 7;
                }
            }
        

        // character can move up with upward velocity if unblocked
        char notBlockedUp =
            !isBlockedOrPrize( (int)playerX - 2, 
                        (int)( playerY - 7 - moveDelta ) )
            &&
            !isBlockedOrPrize( (int)playerX + 1, 
                        (int)( playerY - 7 - moveDelta ) );
        
        if( notBlockedUp && verticalVelocity < 0 ) {
            // slow our rise
            verticalVelocity += moveDelta * ( 1.0 - playerEmotion );
            

            dY += verticalVelocity;
                
            playerY += verticalVelocity;

            // avoid accumulation errors that make y motion jerky
            // when dY is tiny
            playerY = (int)playerY;
            dY = (int)dY;
            

            while( isBlockedOrPrize( (int)playerX - 2, 
                              (int)( playerY - 7 ) )
                   ||
                   isBlockedOrPrize( (int)playerX + 1, 
                              (int)( playerY - 7 ) ) ) {
            
                // hit head inside an obstacle

                // first, stop rising
                verticalVelocity = 0;
                
                // back off
                playerY = (int)playerY;
                playerY ++;
                dY ++;
                }
            
            }
        else {
            // stop rising
            if( verticalVelocity < 0 ) {
                verticalVelocity = 0;
                }
            }


        // character falls if nothing's below him
        char notBlockedDown = 
            !isBlockedOrPrize( (int)playerX - 2, (int)( playerY + moveDelta ) )
            &&
            !isBlockedOrPrize( (int)playerX + 1, 
                        (int)( playerY + moveDelta ) );


        if( notBlockedDown && verticalVelocity >= 0) {
            
            // gravity
            verticalVelocity += moveDelta;
            
            if( verticalVelocity > 5 ) {
                // cap it
                verticalVelocity = 5;
                }
            

            dY += verticalVelocity;
                
            playerY += verticalVelocity;

            // avoid accumulation errors that make y motion jerky
            // when dY is tiny
            playerY = (int)playerY;
            dY = (int)dY;
            
            while( isBlockedOrPrize( (int)playerX - 2, 
                              (int)( playerY ) )
                   ||
                   isBlockedOrPrize( (int)playerX + 1, 
                              (int)( playerY ) ) ) {
            
                // landed inside an obstacle

                // first, stop falling
                verticalVelocity = 0;
                
                // back off
                playerY = (int)playerY;
                playerY --;
                dY --;
                }
            
            }
        else if( !notBlockedDown ) {
            // stop falling
            if( verticalVelocity > 0 ) {
                verticalVelocity = 0;
                }
            
            // standing on firm ground
            // allow a jump
            if( getKeyDown( SDLK_SPACE ) ) {
                verticalVelocity = -5;
                }
            
            }
        
        
        setPlayerPosition( (int)playerX, (int)playerY );
        
        if( !notBlockedDown ) {
            // use standard walking sprite
            setPlayerSpriteFrame( currentSpriteIndex );
            }
        else {
            // override to jumping sprite (legs open frame from walking)
            if( playerFacing ) {
                setPlayerSpriteFrame( 2 );
                }
            else {
                setPlayerSpriteFrame( 6 );
                }
            }
        

        // check for events to quit
        SDL_Event event;
        while( SDL_PollEvent(&event) ) {
            switch( event.type ) {
                case SDL_KEYDOWN:
                    switch( event.key.keysym.sym ) {
                        case SDLK_q:
                        case SDLK_ESCAPE:
                            done = true;
                            quit = true;
                            break;
                        default:
                            break;
                        }
                    break;
                case SDL_QUIT:
                    done = true;
                    quit = true;
                    break;
                default:
                    break;
                }
            }

        //t +=0.25;
        frameCount ++;
        
        
        // add particles for "hair on fire" effect
        Particle p;
        
        p.mGradient = &fireGradient;
        
        // start out cream colored
        p.mGradientPosition = 1.0;
        p.mFade = 1.0;
        p.mAdditive = false;

        // rises slowly
        p.mDX = 0;
        p.mDY = -0.5;
        // fades out
        double baseDFade = -0.1;
        
        
        double baseDeltaGradient = -0.2;
                
        
        // 7 hair pixels, each which generates fire particles
        // offset of hair pixels from player position
        int hairOffsetX[7];
        int hairOffsetY[7];
        
        
        hairOffsetY[0] = -5;
        hairOffsetY[1] = -5;
        hairOffsetY[2] = -6;
        hairOffsetY[3] = -6;
        hairOffsetY[4] = -7;
        hairOffsetY[5] = -7;
        hairOffsetY[6] = -7;
            
        if( playerFacing ) {
            // facing right
            hairOffsetX[0] = -2;
            hairOffsetX[1] = -1;
            hairOffsetX[2] = -2;
            hairOffsetX[3] = -1;
            hairOffsetX[4] = -1;
            hairOffsetX[5] = 0;
            hairOffsetX[6] = 1;
            
            if( playerPushing ) {
                // head moves right by one
                for( int h=0; h<7; h++ ) {
                    hairOffsetX[h] ++;
                    }
                }
            }
        else {
            //facing left
            hairOffsetX[0] = 0;
            hairOffsetX[1] = 1;
            hairOffsetX[2] = 0;
            hairOffsetX[3] = 1;
            hairOffsetX[4] = -2;
            hairOffsetX[5] = -1;
            hairOffsetX[6] = 0;
            }

        // reuse particle object for each spot on hair
        // double number of particles placed to make them thicker
        for( int s=0; s<2; s++ ) {
            
            for( int h=0; h<7; h++ ) {
                p.mX = (int)playerX + hairOffsetX[h];
                p.mY = (int)playerY + hairOffsetY[h];
                

                // fade out faster farther from center
                p.mDFade = baseDFade * ( 1 + 0.25 * fabs( hairOffsetX[h] ) );

                // random vertical speed
                p.mDY = randSource.getRandomBoundedDouble( -2, -0.1 );
                
                // randomly scale gradient delta
                double deltaScale = 
                    randSource.getRandomBoundedDouble( 0.75, 1 );
                
                p.mDeltaGradientPosition = baseDeltaGradient * deltaScale;
                
                // half are additive
                // this allows some pixels to look like smoke when they
                // become darker at the top
                p.mAdditive = randSource.getRandomBoolean();
                    

                addParticle( p );
                }
            }
                 

        // step particles right after placing them to disguise initial 
        // placement pattern
        stepParticles();

        

        // other animations run independent of whether player is moving
        stepAnimations();
                
        
        timeLeft += timeDelta;
        
        if( timeLeft <= 0 ) {
            gameOver = true;
            timeLeft = 0;
            }
        

        // last 3/8 of game
        if( timeLeft <= 0.375 * totalTime ) {
                        
            // mez "sneaks" away if he's off screen near the end of the game
            if( ! isMezOnScreen() ) {
                hideMez();
                }

            }

        // last 1/8 of game
        if( timeLeft <= 0.125 * totalTime ) {
            // music fades out toward end of fire dying
            setMusicLoudness( timeLeft / ( 0.125 * totalTime ) );
            }
        
        

        if( isMezHidden() ) {
            // fix ball in front of where Mez usually stands
            returnBallToMez();
            }
        else {
            // mez visible... handle throw-catch logic
            
            int mezX, mezY;
            getMezPosition( &mezX, &mezY );
        
            int distanceFromMez = (int) sqrt( pow( mezX - playerX, 2 ) +
                                              pow( mezY - playerY, 2 ) );
            
            if( distanceFromMez < 30 
                &&
                // not too close
                distanceFromMez > 15 
                &&
                // standing at same level as Mez
                mezY == (int)playerY - 3
                &&
                mezHasBall() ) {
                
                throwBall();
                }
            
            
            char mezCaughtBall = stepBall();

            if( mezCaughtBall ) {
                playerEmotionSmoothTransitionTarget = playerEmotion + 0.15;
                if( playerEmotionSmoothTransitionTarget > 1 ) {
                    playerEmotionSmoothTransitionTarget = 1;
                    }
                /*
                  playerEmotion += 0.15;
                  if( playerEmotion > 1.0 ) {
                  playerEmotion = 1.0;
                  }
                */
                // return to default descent
                deltaPlayerEmotion = defaultDeltaPlayerEmotion;
                
                // continue any upswing
                // upswingDeltaPlayerEmotion = 0;
                }
            }
        
        

        if( isPrize( (int)playerX, (int)playerY ) ) {
            touchPrize( (int)playerX, (int)playerY );
            }
        
        
        stepPrizes();
        

        if( playerEmotionSmoothTransitionTarget != -1 ) {
            // move emotion toward target
            if( playerEmotion <= playerEmotionSmoothTransitionTarget ) {
                playerEmotion += smoothTransitionDelta;
                
                if( playerEmotion >= playerEmotionSmoothTransitionTarget ) {
                    // hit target
                    playerEmotion = playerEmotionSmoothTransitionTarget;
                    playerEmotionSmoothTransitionTarget = -1;
                    }
                }
            else if( playerEmotion > playerEmotionSmoothTransitionTarget ) {
                playerEmotion -= smoothTransitionDelta;
                
                if( playerEmotion <= playerEmotionSmoothTransitionTarget ) {
                    // hit target
                    playerEmotion = playerEmotionSmoothTransitionTarget;
                    playerEmotionSmoothTransitionTarget = -1;
                    }
                }
            }
        else if( upswingDeltaPlayerEmotion == 0 ) {
            
            // player emotion gradually decreases with time
            playerEmotion += deltaPlayerEmotion;
            }
        else {
            // in upswing
            playerEmotion += upswingDeltaPlayerEmotion;
            }
        

        if( upswingDeltaPlayerEmotion != 0 &&
            playerEmotion >= 0.5 ) {
            // halfway through a natural upswing
            
            // return to default descent rate (for when after upswing is over)
            deltaPlayerEmotion = defaultDeltaPlayerEmotion;
            }
        
        if( playerEmotion < 0 ) {
            playerEmotion = 0;

            // hit bottom, move naturally upward
            upswingDeltaPlayerEmotion = defaultUpswingAmount;

            // keep whatever descent rate (deltaPlayerEmotion) we have, for
            // now, in case we hit another prize to cancel our natural upswing
            }
        else if( playerEmotion > 1 ) {
            playerEmotion = 1;
            
            // upswing over
            upswingDeltaPlayerEmotion = 0;
            }
        }
    
    unsigned long netTime = time( NULL ) - startTime;
    double frameRate = frameCount / (double)netTime;
    
    printf( "Frame rate = %f fps (%d frames)\n", 
            frameRate, frameCount );

    printf( "Game time = %d:%d\n", 
            (int)netTime / 60, (int)netTime % 60 );

    fflush( stdout );
    
    
    delete titleImage;

    delete [] gameImage;
    delete [] titlePixels;
    
    
    destroyWorld();
    
    

    if( quit ) {
        return false;
        }
    else {
        return true;
        }
        
    }


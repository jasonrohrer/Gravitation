#include "World.h"


#include "common.h"
#include "map.h"
#include "GraphicContainer.h"


#include "minorGems/graphics/Image.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/random/StdRandomSource.h"



extern double playerEmotion;
extern double deltaPlayerEmotion;
extern double upswingDeltaPlayerEmotion;
extern double playerEmotionSmoothTransitionTarget;


extern StdRandomSource randSource;



GraphicContainer *tileContainer;
GraphicContainer *backgroundContainer;

GraphicContainer *spriteContainer;

GraphicContainer *mezContainer;
GraphicContainer *ballContainer;

GraphicContainer *prizeContainer;
GraphicContainer *prizeOrbContainer;
GraphicContainer *numeralContainer;

GraphicContainer *kilnContainer;
GraphicContainer *kilnFireContainer;
GraphicContainer *kilnSmokeContainer;
GraphicContainer *kilnAshPitContainer;


GraphicContainer *heartContainer;
GraphicContainer *madContainer;




// dimensions of one tile.  TileImage contains 13 tiles, stacked vertically,
// with blank lines between tiles
int tileW = 11;
int tileH = 11;

int tileImageW;
int tileImageH;

int numTileSets;

int backgroundW = 99;
int backgroundH = 100;

int backgroundImageW;
int backgroundImageH;
int numBackgrounds;





class Animation {
    public:
        
        Animation( int inX, int inY, int inFrameW, int inFrameH,
                   char inAutoStep,
                   char inRemoveAtEnd, GraphicContainer *inGraphics )
                : mX( inX ), mY( inY ), 
                  mFrameW( inFrameW ), mFrameH( inFrameH ),
                  mPageNumber( 0 ),
                  mFrameNumber( 0 ),
                  mAutoStep( inAutoStep ),
                  mStepsPerFrame( 6 ),
                  mTotalStepCount( 0 ),
                  mRemoveAtEnd( inRemoveAtEnd ),
                  mHide( false ),
                  mOnScreen( true ),
                  mTransparency( 1.0 ),
                  mAdditiveBlend( false ),
                  mGraphics( inGraphics ) {
            
            mImageW = mGraphics->mW;
            
            mNumFrames = ( mGraphics->mH - mFrameH ) / mFrameH + 1;
            mNumPages = ( mGraphics->mW - mFrameW ) / mFrameW + 1;
            }
        
        // default constructor so that we can build a vector
        // of Animations
        Animation() {
            }
        
        
        // replaces graphics of animation
        void swapGraphics( GraphicContainer *inNewGraphics ) {
        
            mGraphics = inNewGraphics;
            
            mImageW = mGraphics->mW;
            
            mNumFrames = ( mGraphics->mH - mFrameH ) / mFrameH + 1;
            mNumPages = ( mGraphics->mW - mFrameW ) / mFrameW + 1;
            
            if( mPageNumber >= mNumPages ) {
                mPageNumber = mNumPages - 1;
                }
            if( mFrameNumber >= mNumFrames ) {
                mFrameNumber = mNumFrames - 1;
                }
            }
        
        

        int mX, mY;
        
        
        int mFrameW, mFrameH;
        int mImageW;
        
        // can blend between pages
        double mPageNumber;

        int mNumPages;
        
        int mFrameNumber;
        int mNumFrames;
        char mAutoStep;
        // can be used to slow an animtion down.
        char mStepsPerFrame;
        
        int mTotalStepCount;
        
        
        char mRemoveAtEnd;
        
        char mHide;
        
        // was this animation on the screen the last
        // time the screen was drawn?
        char mOnScreen;
        

        double mTransparency;
        char mAdditiveBlend;
                
        GraphicContainer *mGraphics;
        
    };



// Set default size larger than what we'll ever need
// thus, vector will never be resized, so we can obtain pointers
// to elements of the vector safely.
// This is an abuse of the SimpleVector interface (since we shouldn't be
// able to depend on pointers into the vector, especially when items are being
// added and deleted).  However, this makes memory management much cleaner (if
// we were storying Animation* objects in the vector, they would each need to 
// be allocated on the heap and freed when they are removed).
// Also, all the Animations that we add/remove mid-game are at the end of the 
// vector, past the point where our more static animations (like player and
// prizes, etc.) are located.  Thus, adding and removing items from the end
// won't invalidate these static pointers in to the vector.
//
// Still, it would be nice to make this cleaner and safer... but it works 
SimpleVector<Animation> animationList( 500 );


class PrizeStatus {
    public: 

        PrizeStatus() 
                : mAnimation( NULL ), mScoreAnimation( NULL ),
                  mTouched( false ),
                  mFalling( false ), mFrozen( false ), mScoreTickTimer( 0 ),
                  mScore( 9 ),
                  mSlidingIntoFire( false ),
                  mMelting( false ), 
                  mMeltTimer( 0 ), mOrbSent( false ), mDY( 0 ), mStartY( 0 ),
                  mPushAccumulation( 0 ) {
            }


        PrizeStatus( Animation *inAnimation ) 
                : mAnimation( inAnimation ), mScoreAnimation( NULL ),
                  mTouched( false ),
                  mFalling( false ), mFrozen( false ), mScoreTickTimer( 0 ),
                  mScore( 9 ),
                  mSlidingIntoFire( false ),
                  mMelting( false ), 
                  mMeltTimer( 0 ), mOrbSent( false ), mDY( 0 ), mStartY( 0 ),
                  mPushAccumulation( 0 ) {
            }

        
        Animation *mAnimation;

        Animation *mScoreAnimation;

        char mTouched;
        
        char mFalling;
        
        char mFrozen;

        // used for timing score numeral transitions
        int mScoreTickTimer;
        int mScore;
        

        char mSlidingIntoFire;
        
        char mMelting;
        
        // used for timing melting frames
        int mMeltTimer;
        
        char mOrbSent;
        

        double mDY;
        
        double mStartY;
        
        // where sub-pixel pushes can accumulate to finally move
        // the prize a full pixel
        double mPushAccumulation;
                
    };



SimpleVector<PrizeStatus> prizeList;




Animation *spriteAnimation;


Animation *mezAnimation;
Animation *ballAnimation;

#define numKilnFireAnimations  3
Animation *kilnFireAnimations[ numKilnFireAnimations ];
// update one each timestep
int kilnFireAnimationToUpdate = 0;

// starting y for fires (they shrink downward
int kilnFireStartingY;


#define numKilnSmokeAnimations  3
Animation *kilnSmokeAnimations[ numKilnSmokeAnimations ];
// update one each timestep
int kilnSmokeAnimationToUpdate = 0;

double kilnSmokeStartingAlpha;


Animation *kilnAshPitAnimation;


Animation *prizeOrbAnimation;





char equals( rgbColor inA, rgbColor inB ) {
    return inA.r == inB.r
        && inA.g == inB.g
        && inA.b == inB.b;
    }




Uint32 *preRenderedMap;
int mapWidth, mapHeight, mapPixels;

// true where transparent
// false where opaque
char *mapTransparency;




// precompute the result of multiplying an alpha value of 0..255 by a color
// value of 0..255
unsigned char alphaBlendMap[256][256];




void initWorld() {
    
    // precompute alpha map
    for( int a=0; a<256; a++ ) {
        for( int c=0; c<256; c++ ) {
            alphaBlendMap[a][c] = (unsigned char)( (a / 255.0) * c );
            }
        }
    
    
    prizeList.deleteAll();
    
    animationList.deleteAll();

    // orb front-most
    Animation prizeOrb( 0, 0,
                        5, 5, false, false, 
                        prizeOrbContainer );
                
    prizeOrb.mAdditiveBlend = true;
    prizeOrb.mTransparency = 1.0;
    prizeOrb.mHide = true;
                        
    animationList.push_back( prizeOrb );

    // get pointer to animation in vector
    prizeOrbAnimation = animationList.getElement( animationList.size() - 1 );



    // kiln smoke in front of everything
    Animation kilnSmoke( 7 * tileW + 2, getMapHeight() - tileH - 40 - 10,
                         5, 21, false, false, kilnSmokeContainer );
    
    kilnSmoke.mFrameNumber = 0;
    
    kilnSmokeStartingAlpha = 0.25;
    kilnSmoke.mTransparency = kilnSmokeStartingAlpha;
    
    kilnSmoke.mAdditiveBlend = false;
    
    for( int k=0; k<numKilnSmokeAnimations; k++ ) {
        
        animationList.push_back( kilnSmoke );
    
        // get pointer to animation in vector
        kilnSmokeAnimations[k] 
            = animationList.getElement( animationList.size() - 1 );
        }

    

    
    Animation character(  0, 0, 8, 8, false, false, spriteContainer );
    
    
    animationList.push_back( character );
    
    // get pointer to animation in vector
    spriteAnimation = animationList.getElement( animationList.size() - 1 );



    Animation mez(  tileW + 3/*tileW + 2*/, getMapHeight() - tileH - 4, 
                    8, 8, false, false, mezContainer );
    mez.mFrameNumber = 0;
    
    
    animationList.push_back( mez );
    
    // get pointer to animation in vector
    mezAnimation = animationList.getElement( animationList.size() - 1 );



    Animation ball( 0, 0, 
                    8, 8, false, false, ballContainer );
    ball.mFrameNumber = 0;
    
    
    animationList.push_back( ball );
    
    // get pointer to animation in vector
    ballAnimation = animationList.getElement( animationList.size() - 1 );

    returnBallToMez();    



    // ash pit in front of fire (fire shrinks down behind it
    Animation kilnAshPit( 7 * tileW + 2, getMapHeight() - tileH / 2 - 1,
                          15, 11, false, false, kilnAshPitContainer );
    
    kilnAshPit.mFrameNumber = 0;
    
    animationList.push_back( kilnAshPit );

    kilnAshPitAnimation = animationList.getElement( animationList.size() - 1 );



    // kiln fire in front of prizes
    kilnFireStartingY = getMapHeight() - tileH - 6;
    
    Animation kilnFire( 7 * tileW + 2, kilnFireStartingY,
                    11, 11, false, false, kilnFireContainer );
    
    kilnFire.mFrameNumber = 0;
    kilnFire.mTransparency = 0.65;
    kilnFire.mAdditiveBlend = true;
    
    for( int k=0; k<numKilnFireAnimations; k++ ) {
        
        animationList.push_back( kilnFire );
    
        // get pointer to animation in vector
        kilnFireAnimations[k] 
            = animationList.getElement( animationList.size() - 1 );
        }
    


    // populate prizes
    int mapTileH = getMapHeight() / tileH;
    int mapTileW = getMapWidth() / tileW;
    
    for( int y=0; y<mapTileH; y++ ) {
        for( int x=0; x<mapTileW; x++ ) {
            
            if( isPrizeStartingSpot( x, y ) ) {
                
                int prizeW = 11;
                int prizeH = 11;

                // a score in front of each prize
                Animation prizeScore( x * tileW + 2, y * tileH + 2,
                                      3, 4, false, false, 
                                      numeralContainer );
                
                prizeScore.mAdditiveBlend = false;
                prizeScore.mTransparency = 0.60;
                prizeScore.mHide = true;
                        
                animationList.push_back( prizeScore );

                Animation *scoreAnimation = 
                    animationList.getElement( animationList.size() - 1 );


                Animation prize( x * tileW + prizeW/2, 
                                 y * tileH + prizeH/2, 
                                 11, 11, false, false, prizeContainer );
                
                if( prize.mY < mapHeight - 2 * tileH ) {
                    // thawed
                    prize.mFrameNumber = 0;
                    }
                else {
                    // frozen
                    prize.mFrameNumber = 1;
                    }
                
                animationList.push_back( prize );
                Animation *a = 
                    animationList.getElement( animationList.size() - 1 );
                
                PrizeStatus p( a );
                
                p.mStartY = prize.mY;
                
                p.mScoreAnimation = scoreAnimation;
                
                scoreAnimation->mFrameNumber = p.mScore;
                

                prizeList.push_back( p );
                }
            
            }
        }


    // kiln behind prizes
    Animation kiln( 7 * tileW + 2, getMapHeight() - tileH - 20,
                    15, 41, false, false, kilnContainer );
    
    kiln.mFrameNumber = 0;
    
    animationList.push_back( kiln );

    
    }








char isSpriteTransparent( GraphicContainer *inContainer, int inSpriteIndex ) {
    
    // take transparent color from corner
    return 
        inContainer->mRed[ inSpriteIndex ] == 
        inContainer->mRed[ 0 ]
        &&
        inContainer->mGreen[ inSpriteIndex ] == 
        inContainer->mGreen[ 0 ]
        &&
        inContainer->mBlue[ inSpriteIndex ] == 
        inContainer->mBlue[ 0 ];
    }





#include <math.h>





void sampleFromWorld( int inX, int inY, int inWidth, int inHeight,
                      Uint32 *outDestination ) {
    // first draw from pre-rendered map

    // for now, assume x = 0 and inWidth = mapWidth

    // used to select y window on map
    int skipPixels = inY * mapWidth;
    int numPixels = inWidth * inHeight;

    
    // blend between two tile sets
    
    // skip first tile set (emotion 0 maps to tileset 1)
    // tileset 0 is kept as a base tileset for working convenience

    int tileSetA = (int)( playerEmotion * (numTileSets - 2) ) + 1;
    int tileSetB = tileSetA + 1;
    
    if( tileSetB >= numTileSets ) {
        tileSetB = tileSetA;
        }
    
    double bTileWeight = ( playerEmotion * (numTileSets - 2) + 1) - tileSetA;
    

    
    // extra offset to index tile sets
    Uint32 *mapPixelsAfterSkipA = 
        &( preRenderedMap[ tileSetA * mapPixels + skipPixels ] );
    Uint32 *mapPixelsAfterSkipB = 
        &( preRenderedMap[ tileSetB * mapPixels + skipPixels ] );

    unsigned char bTileAlpha = (unsigned char)( bTileWeight * 255 );
    unsigned char aTileAlpha = 255 - bTileAlpha;
    
    unsigned char *aTileAlphaMap = alphaBlendMap[ aTileAlpha ];
    unsigned char *bTileAlphaMap = alphaBlendMap[ bTileAlpha ];




    // draw background to fill in transparent spots
    char *transparencyValuesAfterSkip = &( mapTransparency[ skipPixels ] );


    // do same style of alpha blending for the background pixels

    // blend between two backgroundImages
    
    int backgroundSetA = (int)( playerEmotion * (numBackgrounds - 1) );
    int backgroundSetB = backgroundSetA + 1;
    
    if( backgroundSetB >= numBackgrounds ) {
        backgroundSetB = backgroundSetA;
        }
    
    double bBackgroundWeight = 
        ( playerEmotion * (numBackgrounds - 1) ) - backgroundSetA;
    
    // offset between backgrounds
    int backgroundSetOffset = backgroundW * ( backgroundH + 1 );
    
    Uint32 *backgroundPixels = backgroundContainer->mCompositePixels;
    

    // apply offset to index background sets
    Uint32 *backgroundPixelsAfterSkipA = 
        &( backgroundPixels[ backgroundSetA * backgroundSetOffset ] );
    Uint32 *backgroundPixelsAfterSkipB = 
        &( backgroundPixels[ backgroundSetB * backgroundSetOffset] );
    
    unsigned char bBackgroundAlpha = 
        (unsigned char)( bBackgroundWeight * 255 );
    unsigned char aBackgroundAlpha = 255 - bBackgroundAlpha;
    
    unsigned char *aBackgroundAlphaMap = alphaBlendMap[ aBackgroundAlpha ];
    unsigned char *bBackgroundAlphaMap = alphaBlendMap[ bBackgroundAlpha ];



    int i;
    
    for( i=0; i<numPixels; i++ ) {
        if( transparencyValuesAfterSkip[i] ) {
            // transparent

            // sample background

            // Optimization:
            // use struct/union data type
            Pixel32 aPixel, bPixel, blendPixel;
            aPixel.argbValue = backgroundPixelsAfterSkipA[i];
            bPixel.argbValue = backgroundPixelsAfterSkipB[i];

            // Optimization:
            // (found with profiler and suggested by a Gamasutra article on 
            //  fast alpha blending)
            // Look up result of alpha multiplications in a table
                        
            blendPixel.channels.r = aBackgroundAlphaMap[ aPixel.channels.r ] +
                bBackgroundAlphaMap[ bPixel.channels.r ];
            blendPixel.channels.g = aBackgroundAlphaMap[ aPixel.channels.g ] +
                bBackgroundAlphaMap[ bPixel.channels.g ];
            blendPixel.channels.b = aBackgroundAlphaMap[ aPixel.channels.b ] +
                bBackgroundAlphaMap[ bPixel.channels.b ];

            outDestination[i] = blendPixel.argbValue;
            }
        else {
            // not transparent, sample from tiles

            // Optimization:
            // use struct/union data type
            Pixel32 aPixel, bPixel, blendPixel;
            aPixel.argbValue = mapPixelsAfterSkipA[i];
            bPixel.argbValue = mapPixelsAfterSkipB[i];

            // Optimization:
            // (found with profiler and suggested by a Gamasutra article on 
            //  fast alpha blending)
            // Look up result of alpha multiplications in a table
                        
            blendPixel.channels.r = aTileAlphaMap[ aPixel.channels.r ] +
                bTileAlphaMap[ bPixel.channels.r ];
            blendPixel.channels.g = aTileAlphaMap[ aPixel.channels.g ] +
                bTileAlphaMap[ bPixel.channels.g ];
            blendPixel.channels.b = aTileAlphaMap[ aPixel.channels.b ] +
                bTileAlphaMap[ bPixel.channels.b ];

            outDestination[i] = blendPixel.argbValue;
            }
        
        }
    

    // draw sprites on top

    // printf( "Drawing %d animations\n", animationList.size() );
    
    // draw them in reverse order so that oldest sprites are drawn on top
    for( i=animationList.size() - 1; i>=0; i-- ) {

        Animation a = *( animationList.getElement( i ) );
        
        int animW = a.mFrameW;
        int animH = a.mFrameH;
        
        // anim centered at mX and mY
        int animStartX = a.mX - animW / 2;
        int animEndX = a.mX + animW - 1;

        int animStartY = a.mY - animH / 2;
        int animEndY = a.mY + animH - 1;
        

        // is anim visible and on screen?
        if( ! a.mHide
            &&
            ( animStartX >= inX && animStartX <= inX + inWidth
              ||
              animEndX >= inX && animEndX <= inX + inWidth
              ||
              animStartX <= inX && animEndX >= inX + inWidth
              )
            &&
            ( animStartY >= inY && animStartY <= inY + inHeight
              ||
              animEndY >= inY && animEndY <= inY + inHeight
              ||
              animStartY <= inY && animEndY >= inY + inHeight
              ) ) {
            
            // cannot access mOnScreen in a, because it's a copy
            animationList.getElement( i )->mOnScreen = true;
            
            // draw pixels
            for( int y=0; y< a.mFrameH; y++ ) {
                int screenY = animStartY + y;
                if( screenY >= inY && screenY < inY + inHeight ) {
                    // y coordinate on screen
                    
                    for( int x=0; x< a.mFrameW; x++ ) {
                        int screenX = animStartX + x;
                        if( screenX >= inX && screenX < inX + inHeight ) {
                            // x coordinate on screen
                

                            int animIndex = y * a.mImageW + x;
        
                            // skip to appropriate anim page
                            animIndex += (int)a.mPageNumber * (animW + 1);
            
            
                            // skip to appropriate anim frame
                            animIndex += 
                                a.mFrameNumber * 
                                a.mImageW * 
                                ( animH + 1 );
            
                            // page to blend with
                            int animBlendIndex = animIndex + animW + 1;
                            
                            double blendWeight = a.mPageNumber - 
                                (int)a.mPageNumber;
            
        
                            if( !isSpriteTransparent( a.mGraphics, 
                                                      animIndex ) ) {
            
                                // in non-transparent part
                                rgbColor pixelColor;
                                
                                if( blendWeight != 0 ) {
                                    pixelColor.r = 
                                        ( 1 - blendWeight ) * 
                                        a.mGraphics->mRed[ animIndex ] 
                                        +
                                        blendWeight * 
                                        a.mGraphics->mRed[ animBlendIndex ];
                                    pixelColor.g = 
                                        ( 1 - blendWeight ) * 
                                        a.mGraphics->mGreen[ animIndex ] 
                                        +
                                        blendWeight * 
                                        a.mGraphics->mGreen[ animBlendIndex ];
                                    pixelColor.b = 
                                        ( 1 - blendWeight ) * 
                                        a.mGraphics->mBlue[ animIndex ] 
                                        +
                                        blendWeight * 
                                        a.mGraphics->mBlue[ animBlendIndex ];
                                    }
                                else {
                                    // no blend
                                    pixelColor.r = 
                                        a.mGraphics->mRed[ animIndex ];
                                    pixelColor.g = 
                                        a.mGraphics->mGreen[ animIndex ];
                                    pixelColor.b = 
                                        a.mGraphics->mBlue[ animIndex ];
                                    }

                                char transparent = false;
                                
                                if( a.mTransparency < 1 ) {
                                    pixelColor.r *= a.mTransparency;
                                    pixelColor.g *= a.mTransparency;
                                    pixelColor.b *= a.mTransparency;
                                    
                                    transparent = true;
                                    }
                                
                                unsigned char r = (unsigned char) pixelColor.r;
                                unsigned char g = (unsigned char) pixelColor.g;
                                unsigned char b = (unsigned char) pixelColor.b;

                                
                                if( transparent || a.mAdditiveBlend ) {
                                    // mix with underlying pixel color

                                    Uint32 oldPixel = 
                                        outDestination[ 
                                            ( screenY - inY ) * inWidth 
                                            + ( screenX - inX ) ];
                                    
                                    unsigned char oldR = 
                                        ( oldPixel >> 16 ) & 0xFF;
                                    unsigned char oldG = 
                                        ( oldPixel >> 8 ) & 0xFF;
                                    unsigned char oldB = 
                                        oldPixel & 0xFF;

                                    double t = a.mTransparency;
                                    
                                    double invT = 1-t;
                                    

                                    double rFloat;
                                    double gFloat;
                                    double bFloat;
                                        
                                    if( !a.mAdditiveBlend ) {
                                        
                                        rFloat = t * r + invT * oldR;
                                        gFloat = t * g + invT * oldG;
                                        bFloat = t * g + invT * oldB;
                                        }
                                    else {
                                        rFloat = t * r + oldR;
                                        gFloat = t * g + oldG;
                                        bFloat = t * b + oldB;
                                        
                                        if( rFloat > 255 ) {
                                            rFloat = 255;
                                            }
                                        if( gFloat > 255 ) {
                                            gFloat = 255;
                                            }
                                        if( bFloat > 255 ) {
                                            bFloat = 255;
                                            }
                                        }
                                    
                                    r = (unsigned char)rFloat;
                                    g = (unsigned char)gFloat;
                                    b = (unsigned char)bFloat;
                                    }
                                
                                // set pixel
                                outDestination[ 
                                    ( screenY - inY ) * inWidth 
                                    + ( screenX - inX ) ]
                                    
                                    = r << 16 | g << 8 | b;
                                
                                }
                            }
                        }
                    }
                }
            }  // end check for animation on screen
        else {
            // cannot access mOnScreen in a, because it's a copy
            animationList.getElement( i )->mOnScreen = false;
            }
        }  // end loop over animations
           
    }



// gets tile index in tile set for a given map position
int getTileIndex( int inMapX, int inMapY ) {
    
    int tileIndex;
    
    char blocked = isBlocked( inMapX, inMapY );
    
    if( !blocked ) {
        // empty tile
        tileIndex = 0;
        }
    else {
        
        int neighborsBlockedBinary = 0;
        
        if( isBlocked( inMapX, inMapY - tileH ) ) {
            // top
            neighborsBlockedBinary = neighborsBlockedBinary | 1;
            }
        if( isBlocked( inMapX + tileW, inMapY ) ) {
            // right
            neighborsBlockedBinary = neighborsBlockedBinary | 1 << 1;
            }
        if( isBlocked( inMapX, inMapY + tileH ) ) {
            // bottom
            neighborsBlockedBinary = neighborsBlockedBinary | 1 << 2;
            }
        if( isBlocked( inMapX - tileW, inMapY ) ) {
            // left
            neighborsBlockedBinary = neighborsBlockedBinary | 1 << 3;
            }
    
        // skip empty tile, treat as tile index
        neighborsBlockedBinary += 1;
        
        tileIndex = neighborsBlockedBinary;
        }

    return tileIndex;
    }




int getTileWidth() {
    return tileW;
    }



int getTileHeight() {
    return tileH;
    }



void destroyWorld() {
    /*
    printf( "%d hits, %d misses, %f hit ratio\n", 
            hitCount, missCount, hitCount / (double)( hitCount + missCount ) );
    */
    }



void stepAnimations() {
    
    
    for( int i=0; i<animationList.size(); i++ ) {
        Animation *a = animationList.getElement( i );
        if( a->mAutoStep ) {
            a->mTotalStepCount ++;
            if( a->mTotalStepCount % a->mStepsPerFrame == 0 ) {
                
                if( a->mFrameNumber < a->mNumFrames - 1 ) {
                    a->mFrameNumber ++;
                    }
                else if( a->mRemoveAtEnd ) {
                    // remove it
                    animationList.deleteElement( i );
                    // back up in list for next loop iteration
                    i--;
                    }
                }
            }
        
        }
    }



void startHeartAnimation( int inX, int inY ) {

    Animation a( inX, inY, 8, 8, true, true, heartContainer );
    
    animationList.push_back( a );
    }







void setPlayerPosition( int inX, int inY ) {

    char moving = false;

    if( inX != spriteAnimation->mX ) {
        moving = true;
        }

    
    spriteAnimation->mX = inX;
    // player position centered at sprite's feet
    int newSpriteY = inY - spriteAnimation->mFrameH / 2 + 1;
    

    if( newSpriteY != spriteAnimation->mY ) {
        moving = true;
        }
    
    spriteAnimation->mY = newSpriteY;
    }



void setPlayerSpriteFrame( int inFrame ) {
    spriteAnimation->mFrameNumber = inFrame;    
    }



void loadWorldGraphics() {
    tileContainer = new GraphicContainer( "tileSet.tga" );
    
    tileImageW = tileContainer->mW;
    tileImageH = tileContainer->mH;
    
    // 1-pixel border between tilesets
    numTileSets = (tileImageW + 1) / (tileW + 1);

    printf( "%d tileSets\n", numTileSets );




    spriteContainer = new GraphicContainer( "characterSprite.tga" );

    mezContainer = new GraphicContainer( "mezSprite.tga" );
    ballContainer = new GraphicContainer( "ball.tga" );

    prizeContainer = new GraphicContainer( "prize.tga" );
    prizeOrbContainer = new GraphicContainer( "prizeOrb.tga" );
    
    numeralContainer = new GraphicContainer( "numeralsBlack.tga" );



    heartContainer = new GraphicContainer( "heart.tga" );
    madContainer = new GraphicContainer( "mad.tga" );

    kilnContainer = new GraphicContainer( "kiln.tga" );
    kilnFireContainer = new GraphicContainer( "kilnFire.tga" );
    kilnSmokeContainer = new GraphicContainer( "kilnSmoke.tga" );
    kilnAshPitContainer = new GraphicContainer( "kilnAshPit.tga" );

    mapWidth = getMapWidth();
    mapHeight = getMapHeight();
    mapPixels = mapWidth * mapHeight;
    

    backgroundContainer = new GraphicContainer( "background.tga" );
    backgroundImageW = backgroundContainer->mW;
    backgroundImageH = backgroundContainer->mH;

    // 1-pixel border between backgrounds
    numBackgrounds = ( backgroundImageH + 1 ) / ( backgroundH + 1 );
    

    
    preRenderedMap = new Uint32[ numTileSets * mapHeight * mapWidth ];
    mapTransparency = new char[ mapHeight * mapWidth ];
    
    // transparency defined by upper left corner of tile sheet
    Uint32 transColor = tileContainer->mCompositePixels[0];
    
    
    int t;

    int mapTileHeight = mapHeight / tileH;
    int mapTileWidth = mapWidth / tileW;
    
    // make a pre-rendered tile map for each tile set
    for( t=0; t<numTileSets; t++ ) {
        int tileSet = t;
        
        // for each block in map
        for( int tY=0; tY<mapTileHeight; tY++ ) {
            
            for( int tX=0; tX<mapTileWidth; tX++ ) {
            
                // map upper corner into world units
                int startX = tX * tileW;
                int startY = tY * tileH;
                
                int tileIndex = getTileIndex( startX, startY );
                                
                // offset to top left corner of tile in tile image
                int tileImageOffset = 
                    tileIndex * ( tileH + 1 ) * tileImageW
                    + tileSet * (tileW + 1);
                
                int endY = startY + tileH;

                
                // copy rows of tile pixels into pre-rendered map
                for( int y=startY; y<endY; y++ ) {
                    int tileRowY = y % tileH;
                    
                    int tileRowStartImageIndex =  tileImageOffset + 
                            tileRowY * tileImageW;

                    int mapRowStartIndex = y * mapWidth + startX;
                    
                    memcpy( &( preRenderedMap[ t * mapPixels + 
                                               mapRowStartIndex ] ),
                            &( tileContainer->mCompositePixels[ 
                                   tileRowStartImageIndex ] ),
                            tileW * 4 );                    
                    }

                }
            }
        }   
    
    // go back over first pre-rendered map to get map transparency
    t = 0;
    
    for( int y=0; y<mapHeight; y++ ) {
        for( int x=0; x<mapWidth; x++ ) {

            int mapIndex = y * mapWidth + x;

            Uint32 c = preRenderedMap[ t * mapPixels + 
                                       mapIndex ];

            char transparent = ( c == transColor );
            
            mapTransparency[ mapIndex ] = transparent;
            }
        }


    // test output
    // writeTGAFromRGBA( preRenderedMap, mapWidth, mapHeight * numTileSets,
    //                   "testPreRendMap.tga" );
    
    }



void destroyWorldGraphics() {
    delete tileContainer;
    delete backgroundContainer;
    
    delete spriteContainer;

    delete mezContainer;
    delete ballContainer;
    
    delete prizeContainer;
    delete prizeOrbContainer;
    
    delete numeralContainer;
    

    delete heartContainer;
    delete madContainer;

    delete kilnContainer;
    delete kilnFireContainer;
    delete kilnSmokeContainer;
    delete kilnAshPitContainer;
    
    delete [] preRenderedMap;
    delete [] mapTransparency;
    }



char isMezOnScreen() {
    // if either the ball or Mez on screen, count as mez on screen
    // (handles case when ball in flight above Mez)
    return ballAnimation->mOnScreen || mezAnimation->mOnScreen;
    }



void hideMez() {
    mezAnimation->mHide = true;
    }



char isMezHidden() {
    return mezAnimation->mHide;
    }



void getMezPosition( int *outX, int *outY ) {
    *outX = mezAnimation->mX;
    *outY = mezAnimation->mY;
    }

char mezHasBallFlag = true;

// steps to wait before throwing ball again
int mezWaitingCount = 0;


char mezHasBall() {
    return mezHasBallFlag;
    }


double ballDX, ballDY;


void throwBall() {
    if( mezWaitingCount == 0 ) {
        
        mezHasBallFlag = false;
    
        ballDX = 1;
        ballDY = randSource.getRandomBoundedDouble( -4.5, -2 );
        
        // raise hands
        mezAnimation->mFrameNumber = 1;
        }
    }



void getBallPosition( int *outX, int *outY ) {
    *outX = ballAnimation->mX;
    *outY = ballAnimation->mY;
    }



void getBallVelocity( double *outDX, double *outDY ) {
    *outDX = ballDX;
    *outDY = ballDY;
    }


// is ball moving back toward Mez?
char ballHeadingToMez = false;



void returnBallToMez() {
    mezHasBallFlag = true;
    ballHeadingToMez = false;
    
    
    ballAnimation->mX = mezAnimation->mX + 4;
    ballAnimation->mY = mezAnimation->mY + 2;
    
    ballDX = 0;
    ballDY = 0;
    
    mezAnimation->mFrameNumber = 0;
    }



char stepBall() {
    if( mezWaitingCount > 0 ) {
        // freeze ball while mez waits
        mezWaitingCount --;
        /*
        if( mezWaitingCount == 0 &&
            ! mezHasBallFlag ) {
            // mez waiting to throw, and wait count just expired
            // (throw starts on next frame)
            
            // raise hands
            mezAnimation->mFrameNumber = 1;
            }
        */
            
        return false;
        }
    

    if( mezHasBallFlag ) {
        // do nothing, he's holding the ball
        return false;
        }
    

    ballAnimation->mX += (int)( ballDX );
    ballAnimation->mY += (int)( ballDY );
    
    // apply gravity
    ballDY += 0.2;
    
    char returnValue = false;
    

    if( isBlocked( ballAnimation->mX, ballAnimation->mY )
        || isBlockedByPrize( ballAnimation->mX, ballAnimation->mY ) ) {
        if( ballHeadingToMez ) {
            // Mez catches ball thrown by parent
            startHeartAnimation( mezAnimation->mX, mezAnimation->mY - 6 );
            
            returnValue = true;
            }
        else {
            // parent missed ball
            
            Animation a( mezAnimation->mX, mezAnimation->mY - 6, 
                         8, 8, true, true, madContainer );
    
            animationList.push_back( a );

            // wait before throwing again
            mezWaitingCount = 20;
            }
        
        returnBallToMez();
        }
    else if( !ballHeadingToMez ) {
        // check if ball hits player        

        int distanceFromPlayer = (int) sqrt( pow( ballAnimation->mX - 
                                                  spriteAnimation->mX, 2 ) +
                                             pow( ballAnimation->mY - 
                                                  spriteAnimation->mY, 2 ) );
        if( distanceFromPlayer <= 4 ) {
            // send ball back other way (back to Mez)
            ballDX *= -1;
            ballDY *= -1;
            ballHeadingToMez = true;
            }
        }
    // ball heading back to mez will return to mez when it hits the ground


    return returnValue;
    }



PrizeStatus *getPrizeAt( int inX, int inY ) {
    
    for( int i=0; i<prizeList.size(); i++ ) {
        PrizeStatus *status = prizeList.getElement( i );
        
        Animation *prize = status->mAnimation;
        
        if( fabs( inX - prize->mX ) < prize->mFrameW / 2 + 1 &&
            fabs( inY - prize->mY ) < prize->mFrameH / 2 + 1 ) {
            return status;
            }
        }
    return NULL;
    }



char isPrize( int inX, int inY ) {
    
    if( getPrizeAt( inX, inY ) != NULL ) {
        return true;
        }
    else {
        return false;
        }
    }



char isBlockedByPrize( int inX, int inY ) {
    
    PrizeStatus *status = getPrizeAt( inX, inY );
    
    if( status != NULL ) {
        if( status->mFrozen && 
            ! status->mAnimation->mHide ) {
            
            return true;
            }
        }
    return false;
    }





void touchPrize( int inX, int inY ) {
    PrizeStatus *status = getPrizeAt( inX, inY );
    
    if( status->mTouched == false ) {
        
        status->mTouched = true;
        status->mFalling = true;
        // start with some base velocity
        status->mDY = 1;

        // renew mania
        playerEmotionSmoothTransitionTarget = 1.0;
        
        // accellerated descent toward depression
        deltaPlayerEmotion *= 2;
        
        // end upswing
        upswingDeltaPlayerEmotion = 0;
        }
    }



// gets whether there is a falling prize somewhere along the x span of inPrize
// in y direction inDY
// used by getPrizeFallingBelow
// Checks x span from right to left, so returns rightmost prize first
char getPrizeFallingY( PrizeStatus *inPrize, int inDY ) {
    Animation *anim = inPrize->mAnimation;
    
    int prizeStartX = anim->mX - anim->mFrameW/2;
    int prizeEndX = anim->mX + anim->mFrameW/2;
            
    int y = anim->mY + inDY;
                
    for( int x=prizeEndX; x>=prizeStartX; x-- ) {
        PrizeStatus *otherPrize = getPrizeAt( x, y );
        
        if( otherPrize != NULL ) {
            if( otherPrize->mFalling ) {
                return true;
                }
            }
        }

    return false;
    }


    
// gets whether there is a prize somewhere along the x span of inPrize
// in y direction inDY
// used by getPrizeAbove and getPrizeBelow
// Checks x span from right to left, so returns rightmost prize first
PrizeStatus *getPrizeTouchingY( PrizeStatus *inPrize, int inDY ) {
    Animation *anim = inPrize->mAnimation;
    
    int prizeStartX = anim->mX - anim->mFrameW/2;
    int prizeEndX = anim->mX + anim->mFrameW/2;
       
    // hack:
    // check one more units to right if we're looking above
    if( inDY < 0 ) {
        prizeEndX += 1;
        }
    
     
    int y = anim->mY + inDY;
                
    for( int x=prizeEndX; x>=prizeStartX; x-- ) {
                
        if( isBlockedByPrize( x, y ) ) {
            return getPrizeAt( x, y );
            }
        }
    return NULL;
    }


// gets whether there is a prize somewhere along the y span of inPrize
// in x direction inDX
// used by getPrizeRight
PrizeStatus *getPrizeTouchingX( PrizeStatus *inPrize, int inDX ) {
    Animation *anim = inPrize->mAnimation;
    
    int prizeStartY = anim->mY - anim->mFrameH/2;
    int prizeEndY = anim->mY + anim->mFrameH/2;
            
    int x = anim->mX + inDX;
                
    for( int y=prizeStartY; y<=prizeEndY; y++ ) {
                
        if( isBlockedByPrize( x, y ) ) {
            return getPrizeAt( x, y );
            }
        }
    return NULL;
    }


    
PrizeStatus *getPrizeAbove( PrizeStatus *inPrize ) {
    return getPrizeTouchingY( inPrize, 
                              - inPrize->mAnimation->mFrameH/2 - 1 );
    }

PrizeStatus *getPrizeBelow( PrizeStatus *inPrize ) {
    return getPrizeTouchingY( inPrize, 
                              inPrize->mAnimation->mFrameH/2 + 1 );
    }

PrizeStatus *getPrizeRight( PrizeStatus *inPrize ) {
    return getPrizeTouchingX( inPrize, 
                              inPrize->mAnimation->mFrameW/2 + 1 );
    }

PrizeStatus *getPrizeLeft( PrizeStatus *inPrize ) {
    return getPrizeTouchingX( inPrize, 
                              - inPrize->mAnimation->mFrameW/2 - 1 );
    }

char getPrizeFallingBelow( PrizeStatus *inPrize ) {
    return getPrizeFallingY( inPrize, 
                             inPrize->mAnimation->mFrameH/2 + 1 );
    }



char isPushable( PrizeStatus *inStatus ) {
    if( inStatus != NULL 
        && inStatus->mTouched 
        && !inStatus->mFalling
        && !inStatus->mMelting
        && !inStatus->mSlidingIntoFire ) {
        
        PrizeStatus *prizeAbove = getPrizeAbove( inStatus );
        
        if( prizeAbove != NULL ) {
            // one above us, but if we're not supporting it, we should
            // keep sliding until we're up against it's support
            
            // see what's supporting it (the right-most prize under it)
            PrizeStatus *support = getPrizeBelow( prizeAbove );
            
            if( support == inStatus || support == NULL ) {
                // we're the main support
                
                // or there's no support (falling past us)

                // we're sandwitched, we can't move
                return false;
                }
            }
        
        PrizeStatus *prizeRight = getPrizeRight( inStatus );
        
        if( prizeRight == NULL ) {
            // unblocked to right
            return true;
            }
        else {
            return isPushable( prizeRight );
            }
        }
    else {
        // not on solid ground yet
        return false;
        }
    }



double pushPrize( PrizeStatus *inStatus, double inForce ) {
    if( isPushable( inStatus ) ) {
                

        PrizeStatus *prizeRight = getPrizeRight( inStatus );
            
        double move = 0;
        
        if( prizeRight == NULL ) {
            // unblocked to right
            
            inStatus->mPushAccumulation += inForce;
            
            move = inStatus->mPushAccumulation;
            }
        else {
            // push this train (recursive)

            // our force decays as the train gets longer
            // 0.45 makes it slower to push a train of 2 prizes than
            // to push them singly, one after the other
            // The speed for a train of n prizes is 0.45^(n-1)
            
            //  # prizes  |  Decay Factor Train | Effective Decay separate
            //  1         |  1                  | 1
            //  2         |  0.45               | 0.5
            //  3         |  0.20               | 0.333...
            //  4         |  0.09               | 0.25
            //  5         |  0.041              | 0.20

            // thus, we have a good trade-off between fetching a bunch
            // of prizes on one jumping trip and then pushing the resulting
            // train, vs. taking multiple jumping trips and pushing
            // the prizes one at a time
            move = pushPrize( prizeRight, inForce * 0.45 );
            }
            
            
        if( move >= 1 ) {
            // cap move at max of 1 unit per push
            move = 1;
            
            inStatus->mAnimation->mX += (int)move;
            
            // reset accumulator
            inStatus->mPushAccumulation = 0;

            
            /*
            if( inStatus->mAnimation->mX == kilnFireAnimations[0]->mX
                && 
                inStatus->mAnimation->mY == kilnFireAnimations[0]->mY ) {
                
                // in fire
                inStatus->mMelting = true;
                }
            */
            }
            
        return move;
        }
    else {
        return 0;
        }
    
    }



double pushPrize( int inX, int inY, double inForce ) {
    PrizeStatus *status = getPrizeAt( inX, inY );
    
    return pushPrize( status, inForce );
    }


extern int score;
extern double timeLeft;
extern double totalTime;


void stepPrizes() {
    // first, cycle fire animations
    for( int k=0; k<numKilnFireAnimations; k++ ) {
        
        if( k == kilnFireAnimationToUpdate ) {
            
            int oldFrameNumber = kilnFireAnimations[k]->mFrameNumber;
    
            while( kilnFireAnimations[k]->mFrameNumber == oldFrameNumber ) {
                
                kilnFireAnimations[k]->mFrameNumber = 
                    randSource.getRandomBoundedInt( 
                        0, 
                        kilnFireAnimations[k]->mNumFrames - 1 );
                }
            }
        }
    
    kilnFireAnimationToUpdate ++;
    if( kilnFireAnimationToUpdate >= numKilnFireAnimations ) {
        kilnFireAnimationToUpdate = 0;
        }


    // next, cycle smoke animations
    for( int k=0; k<numKilnSmokeAnimations; k++ ) {
        
        if( k == kilnSmokeAnimationToUpdate ) {
            
            int oldFrameNumber = kilnSmokeAnimations[k]->mFrameNumber;
    
            while( kilnSmokeAnimations[k]->mFrameNumber == oldFrameNumber ) {
                
                kilnSmokeAnimations[k]->mFrameNumber = 
                    randSource.getRandomBoundedInt( 
                        0, 
                        kilnSmokeAnimations[k]->mNumFrames - 1 );
                }
            }
        }
    
    kilnSmokeAnimationToUpdate ++;
    if( kilnSmokeAnimationToUpdate >= numKilnSmokeAnimations ) {
        kilnSmokeAnimationToUpdate = 0;
        }

    
    // cycle through ash pit pages based on time left
    kilnAshPitAnimation->mPageNumber = 
        ( 1 - timeLeft / totalTime ) *
        ( kilnAshPitAnimation->mNumPages - 1 );
    
    // shrink fire down and fade smoke during last 1/2 of time
    if( timeLeft <= 0.5 * totalTime ) {
        
        // decreases as timeLeft -> 0
        double fadeFactor = timeLeft / ( 0.5 * totalTime );
        
        // increases as timeLeft -> 0
        double shrinkFactor = 1 - fadeFactor;
        
        int maxOffset = tileH - 2;
        
        int yOffset = 
            (int)( shrinkFactor * maxOffset );

        double newSmokeAlpha = fadeFactor * kilnSmokeStartingAlpha;

        int k;
        
        for( k=0; k<numKilnFireAnimations; k++ ) {
            
            kilnFireAnimations[k]->mY = kilnFireStartingY + yOffset;
            }

        for( k=0; k<numKilnSmokeAnimations; k++ ) {
            
            kilnSmokeAnimations[k]->mTransparency = newSmokeAlpha;
            }
        }



    // now step prizes
    for( int i=0; i<prizeList.size(); i++ ) {
        PrizeStatus *status = prizeList.getElement( i );
        Animation *prize = status->mAnimation;
            
        if( status->mFalling ) {

            
            prize->mY += (int)( status->mDY );
            
            // apply gravity
            status->mDY += 0.2;

            // cap it
            if( status->mDY > 5 ) {
                status->mDY = 5;
                }
            

            // change animation frame between 0 and 4 at beginning
            // of fall

            // don't do this if we've already finished animation
            // otherwise, we may play animation again if block starts falling
            // again (after hitting bottom) w/in 50 y units from mStartY
            if( prize->mFrameNumber < 4 ) {
                
                prize->mFrameNumber = 
                    (int)( 4 * (prize->mY - status->mStartY) / 50 );
            
                if( prize->mFrameNumber > 4 ) {
                    prize->mFrameNumber = 4;
                    }
                }
            
            

            if( getPrizeFallingBelow( status ) ) {
                // another prize is already falling, and
                // it is ahead of us
                
                // slow our fall back to base velocity to give
                // other prize a chance to speed up and get out
                // of the way
                status->mDY = 1;
                }
            
            
            PrizeStatus *hitPrize = getPrizeBelow( status );
            
            if( hitPrize != NULL ) {
                printf( "Hit prize.\n" );
                
                // stop falling
                status->mFalling = false;
                status->mDY = 0;
                status->mFrozen = true;
                
                // back off
                
                int otherY = hitPrize->mAnimation->mY;
                    
                prize->mY = otherY - prize->mFrameH;
                }
            
            if( status->mFalling &&
                prize->mY > mapHeight - tileH - prize->mFrameH/2 - 1 ) {
                // hit bottom 

                // stop falling
                status->mFalling = false;
                status->mDY = 0;
                status->mFrozen = true;

                prize->mY = mapHeight - tileH - prize->mFrameH/2 - 1;
                }
            }
        else if( status->mTouched ) {
            // check if still supported from underneath
            char supported = false;
            
            if( prize->mY >= mapHeight - tileH - prize->mFrameH/2 - 1 ) {
                // on bottom
                supported = true;
                }
            else if( getPrizeBelow( status ) != NULL ) {
                // on another prize
                supported = true;
                }
            

            if( !supported ) {
                // fall again
                status->mFalling = true;
                // start with some base velocity
                status->mDY = 1;

                // back to unpushable
                prize->mFrameNumber = 4;
                }
            else if( getPrizeLeft( status ) == NULL
                && isPushable( status ) ) {
                // show arrow
                prize->mFrameNumber = 5;
                }
            else if( !status->mMelting ) {
                // hide arrow
                prize->mFrameNumber = 4;
                }

            
            }

        if( status->mFrozen && ! status->mMelting ) {
            status->mScoreAnimation->mHide = false;
            
            status->mScoreAnimation->mX = prize->mX - 2;
            status->mScoreAnimation->mY = prize->mY - 2;
        
            status->mScoreTickTimer ++;
            
            if( status->mScoreTickTimer >= 75 ) {
                status->mScoreTickTimer = 0;
                status->mScore --;
                
                if( status->mScore < 1 ) {
                    status->mScore = 1;
                    }
                }
            status->mScoreAnimation->mFrameNumber = status->mScore;
            }
        
            
        if( status->mSlidingIntoFire ) {
            prize->mX += 2;
            status->mScoreAnimation->mX += 2;
            
            if( prize->mX >= kilnFireAnimations[0]->mX ) {
                // fully in fire, stop sliding, start melting
                prize->mX = kilnFireAnimations[0]->mX;
                
                status->mSlidingIntoFire = false;
                
                // only melt prizes if time still left (fire still burning)
                if( timeLeft > 0 ) {
                    status->mMelting = true;
                    }
                
                status->mScoreAnimation->mHide = true;
                }
            }
        
                
        if( status->mMelting ) {
            // change animation frame between 4 and 0
            
            status->mMeltTimer ++;
            
            if( status->mMeltTimer == 5 ) {
                
                prize->mFrameNumber --;
                
                if( prize->mFrameNumber < 0 ) {
                    
                    prize->mFrameNumber = 0;
                    
                    if( ! status->mOrbSent ) {
                                                                   
                        // start an orb at chimney top
                        prizeOrbAnimation->mX = prize->mX;
                        prizeOrbAnimation->mY = prize->mY - 36;
                        prizeOrbAnimation->mTransparency = 1.0;
                        prizeOrbAnimation->mHide = false;

                        
     
                        // hide prize
                        prize->mHide = true;

                        // move hidden prize out of the way so that it doesn't 
                        // interfere with collision detection
                        prize->mX += tileW;
                        
                        

                        status->mOrbSent = true;

                        // show score rising with orb
                        status->mScoreAnimation->mHide = false;
                        status->mScoreAnimation->mX = prizeOrbAnimation->mX;
                        status->mScoreAnimation->mY = prizeOrbAnimation->mY;
                        }
                    
                    
                    }

                // reset for next frame
                status->mMeltTimer = 0;
                }
            
            if( status->mOrbSent ) {
                // move orb upward
                prizeOrbAnimation->mY -= 2;
                prizeOrbAnimation->mTransparency -= 0.05;
                
                // move score along with it, but don't fade it
                status->mScoreAnimation->mY -= 2;
                
                if( prizeOrbAnimation->mTransparency <= 0 ) {
                    // done dealing with this prize
                    prizeOrbAnimation->mTransparency = 0;
                    prizeOrbAnimation->mHide = true;
                    status->mScoreAnimation->mHide = true;
                    
                    
                    score += status->mScore;
                    

                    prizeList.deleteElement( i );
                    // back up in loop over prizeList
                    i--;
                    }
                }
            
            }
        else {
            // not melting yet
        
            if( kilnFireAnimations[0]->mX - status->mAnimation->mX < 11
                &&
                status->mAnimation->mY == kilnFireStartingY ) {
                
                // touching fire, start sliding in
                status->mSlidingIntoFire = true;
                }
            }
        
            
        
        }
        
    }





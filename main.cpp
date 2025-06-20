#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <SFML/Graphics.hpp>

#include "buttonRect.h"
#include "dropList.h"
#include "digitDisplay.h"

using std::vector;

struct fieldSpace// rows x cols of these make the field
{
    char cnt;// dual use: 'M' if mine or neighboring mine count
    char st;// state: 3*covered: '-', '?' = clearable, 'f' = flagged + 1 revealed state = 'r'
    sf::RectangleShape rct;
    fieldSpace(): cnt('0'), st('-') { setTxtRect(); }
    bool isMine()const { return cnt == 'M'; }
    bool isClearable()const { return (st == '-' || st == '?'); }
    void init( sf::Vector2f pos, bool mined, char Count='0' );
    int getCount()const;
    void show( sf::RenderWindow& rRW )const;
    void setTxtRect( bool mo = false );
};

const int dr[] = { -1, -1, -1,  1, 1, 1,  0, 0 };// 8 shifts to neighbors
const int dc[] = { -1,  0,  1, -1, 0, 1, -1, 1 };// used in functions

int rwWidth=100, rwHeight=100;
size_t currLevel = 2;
bool plusGame = true;
bool gameOn = true;
int ucSpcCnt = 0;//plusGame ? (rows-2)*(cols-2)-mines : rows*cols-mines;

sf::Clock secTimer;
//sf::Time oneSec;

sf::Font font;

int gridTop=50, gridLt=10, gridBotHgt=10;
int rows=0, cols=0, mines=0;
vector< vector<fieldSpace> > grid;
vector<std::string> levelFnames;

// buttons and lists
sf::RectangleShape R_clr[4];
std::vector<sf::Text> titleVec;// button titles
dropList dL1;
buttonRect reset;

// for digit displays
sf::Texture digitTxt;
sf::IntRect R_dgt[10];
bool initDigitRects(  sf::IntRect* R, sf::Texture& rTxt, const char* fName );// from texture
digitDisplay timeDisp, mineDisp;

// game grid
std::vector<sf::RectangleShape> refTileVec;// reference tile rects
sf::Texture tileTxt;
bool initTileRects( const char* fName );// loads tileTXT, inits refTileVec
bool getGameConfig( const char* fName );// gridTop, Lt, botHgt, levelFnames, plusGame
bool loadLevel( size_t lvlNum, bool makePlus );// open lvl file. Get rows,cols,mines, init grid
int refIdx( char st, char cnt, bool mo = false );

int clearSpace( int r, int c );// Recursive. returns # of spaces cleared
bool applyChord( int r, int c, int& numCleared );// returns true if mine hit

bool handleEvent( sf::Event rEvent, sf::RenderWindow& rRW );
bool handleMseLtDn( sf::RenderWindow& rRW, int hitRow, int hitCol, bool& ltButtDn, bool& rtButtDn );
bool handleMseRtDn( sf::RenderWindow& rRW, int hitRow, int hitCol, bool& ltButtDn, bool& rtButtDn );
bool handleMseOver( int msx, int msy );

void INITrects( sf::RectangleShape* R, float W, float H );// prepare 4 template Rects from sf::Colors
bool INITrects(  sf::RectangleShape* R, sf::Texture& buttTxt, const char* fName );// prepare 4 template Rects from buttTxt

// helpers
//bool hitRect( sf::RectangleShape R );
bool isInField( int r, int c, bool omitPerim = false );// validates position
void makeEndState();// reveals mines and mis-marked flags. Call on game loss (handleEvent)

void fieldSpace::init( sf::Vector2f pos, bool mined, char Count )
{
    st = '-';
    cnt = mined ? 'M' : Count;
    rct = refTileVec[0];
    rct.setPosition(pos);
}

int fieldSpace::getCount()const
{
    if( cnt == 'M' ) return -1;// I rest my case
    return (int)(cnt-'0');
}

void fieldSpace::show( sf::RenderWindow& rRW )const { rRW.draw(rct); }
void fieldSpace::setTxtRect( bool mo ) { rct.setTextureRect( refTileVec[refIdx(st,cnt,mo)].getTextureRect() ); }

int main()
{

    // Create a graphical text to display
    if (!font.loadFromFile("arial.ttf")) return EXIT_FAILURE;

    INITrects( R_clr, 60, 25 );
    if( !initTileRects("images/tilesData.txt") ) return 2;

    // drop list
    sf::Text hmTitle("Levels", font, 10);
    hmTitle.setColor( sf::Color(0,0,0) );
    if( !getGameConfig( "gameData/config.txt" ) ) return 3;// gridTop, Lt, botHgt, levelFnames

    dL1.init( hmTitle, titleVec, R_clr, 10.0f, 10.0f, 2 );
    reset.init( R_clr, 150.0f, 10.0f, sf::Text("RESET",font,10) );
    reset.title.setColor( sf::Color(0,0,0) );

    // digit displays
    if( !initDigitRects(  R_dgt, digitTxt, "images/digits_config.txt" ) ) return 2;
    if( !timeDisp.init( "images/dgtDisplayTime_config.txt", digitTxt, R_dgt, font ) ) return 3;
    if( !mineDisp.init( "images/dgtDisplayMine_config.txt", digitTxt, R_dgt, font ) ) return 4;

    if( !loadLevel( currLevel, plusGame ) ) return 4;

    // now ready for a window
    sf::RenderWindow RW( sf::VideoMode(rwWidth, rwHeight), "Minesweeper", sf::Style::Titlebar | sf::Style::Close );
    RW.setVerticalSyncEnabled(true); // call it once, after creating the window

    secTimer.restart();

    while (RW.isOpen())
    {
        sf::Event event;
        while (RW.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || !handleEvent(event,RW) )
                RW.close();
        }

        if( gameOn && secTimer.getElapsedTime() >= sf::seconds(1.0f) )
            { secTimer.restart(); if(timeDisp.getN() < 999) timeDisp += 1; }

        RW.clear();

        for( const auto& Row : grid )
            for( const auto& fs : Row )
                fs.show(RW);

        reset.draw(RW);
        mineDisp.draw(RW);
        timeDisp.draw(RW);
        dL1.draw(RW);


        if( rEvent.key.code == sf::Keyboard::C )// press key c to capture screen shot
        {
            sf::Image captImg = RW.capture();
            captImg.saveToFile( "screenCapt.png" );
        }

        RW.display();
    }

    return 0;
}

bool getGameConfig( const char* fName )// gridTop, Lt, botHgt, levelFnames
{
    std::ifstream inFile( fName );
    if( !inFile ) return false;

    int pg = 0;
    inFile >> gridLt >> gridTop >> gridBotHgt >> pg;
    plusGame = (pg == 1);
    inFile.ignore(256,'\n');
    std::string buttTitle, lvlFname;
    while( inFile >> buttTitle >> lvlFname )
    {
        levelFnames.push_back( lvlFname );
        titleVec.push_back( sf::Text(buttTitle.c_str(), font, 10) );
        titleVec.back().setColor( sf::Color(0,0,0) );
    }

    return true;
}


bool loadLevel( size_t lvlNum, bool makePlus )// open lvl file. Get rows,cols,mines, init grid
{
//    grid.clear();

    if( lvlNum >= levelFnames.size() ) return false;
    std::ifstream inFile( levelFnames[lvlNum].c_str() );
    if( !inFile ) return false;
    sf::Vector2f tlSz = refTileVec[0].getSize();

    inFile >> rows >> cols >> mines;

    std::vector<bool> mineDist( rows*cols, false );
    for( int i=0; i<mines && i<(int)mineDist.size(); ++i ) mineDist[i] = true;
    std::random_shuffle( mineDist.begin(), mineDist.end() );

    if( makePlus ) { rows += 2; cols += 2; }
    ucSpcCnt = rows*cols-mines;

    // find window dimensions
    int rwWmin = 270;
    rwWidth = 2*gridLt + cols*refTileVec[0].getTextureRect().width;
    if( rwWidth < rwWmin )
    {
        rwWidth = rwWmin;
        std::cout << "\n gridLt old = " << gridLt;
        gridLt = ( rwWmin - cols*refTileVec[0].getTextureRect().width )/2;
        std::cout << " new = " << gridLt;
    }

    // fill grid and init the fs
    grid.resize(rows);
    for( int r=0; r<rows; ++r )
    {
        grid[r].resize(cols);
        for( int c=0; c<cols; ++c )
            grid[r][c].init( sf::Vector2f( gridLt+c*tlSz.x, gridTop+r*tlSz.y ), false );
    }

    // init the mined spaces
    int mineCnt = 0;
    if( makePlus )
    {
       for( int r=1; r<rows-1; ++r )
            for( int c=1; c<cols-1; ++c )
                if( mineDist[(r-1)*(cols-2) + (c-1)] ){ grid[r][c].cnt = 'M'; ++mineCnt; }
    }
    else
    {
        for( int r=0; r<rows; ++r )
            for( int c=0; c<cols; ++c )
                if( mineDist[r*cols+c] ) { grid[r][c].cnt = 'M'; ++mineCnt; }
    }

    std::cout << "mineCnt = " << mineCnt << '\n';

    // complete init and find cnt values
    for( int r=0; r<rows; ++r )
        for( int c=0; c<cols; ++c )
        {
        //      char mineCnt = '0';// find # of neighboring mines
            if( grid[r][c].cnt == '0' )
                for(int i=0; i<8; ++i)// visit the 8 spaces around it
                    if( isInField( r+dr[i], c+dc[i] ) )
                        if( grid[ r+dr[i] ][ c+dc[i] ].cnt == 'M' ) ++grid[r][c].cnt;
        }

    if( makePlus )// clear perimeter spaces. Call clearspace() so any auto clears will occur
    {
        // top and bottom
        for(int c=0; c<cols; ++c) { ucSpcCnt -= clearSpace(0,c); ucSpcCnt -= clearSpace(rows-1,c); }
        // left and right sides
        for(int r=1; r<rows-1; ++r) { ucSpcCnt -= clearSpace(r,0); ucSpcCnt -= clearSpace(r,cols-1); }

        for( auto& Row : grid )
            for( auto& fs : Row )
                if( fs.st == 'r' && fs.cnt == '0' )
                    fs.rct.setTextureRect( refTileVec[19].getTextureRect() );
    }

    // find window dimensions
 /*   int rwWmin = 270;
    rwWidth = 2*gridLt + cols*refTileVec[0].getTextureRect().width;
    if( rwWidth < rwWmin )
    {
        rwWidth = rwWmin;
        std::cout << "\n gridLt old = " << gridLt;
        gridLt = ( rwWmin - cols*refTileVec[0].getTextureRect().width )/2;
        std::cout << " new = " << gridLt;
    }   */
    rwHeight = gridTop + gridBotHgt + rows*refTileVec[0].getTextureRect().height;

    std::cout << "\n rwWidth = " << rwWidth << "rwHeight = " << rwHeight << " ucSpcCnt = " << ucSpcCnt << '\n';

    dL1.setPosition( sf::Vector2f( (rwWidth/2) - 30.0f , 5.0f ) );
    reset.setPosition( sf::Vector2f( (rwWidth/2) - 30.0f , 35.0f ) );
    mineDisp = mines;
    timeDisp.setPosition( sf::Vector2f( rwWidth - timeDisp.bkgdRect.getSize().x - 20.0f , 5.0f ) );

    return true;
}

bool initTileRects( const char* fName )
{
    std::ifstream inFile( fName );
    std::string imgFname;
    if( !inFile ) return false;
    inFile >> imgFname;
    if( !tileTxt.loadFromFile( imgFname.c_str() ) ) return false;
    int W, H, numTiles;
    inFile >> W >> H >> numTiles;
    refTileVec.resize(numTiles);

    for( int i=0; i<numTiles; ++i )
    {
        refTileVec[i].setSize( sf::Vector2f(W,H) );
        refTileVec[i].setPosition( 50.0f+17.0f*i, 180.0f );
        int Lt, Tp;
        inFile >> Lt >> Tp;
        refTileVec[i].setTexture( &tileTxt );
        refTileVec[i].setTextureRect( sf::IntRect(Lt,Tp,W,H) );
    }

    return true;
}

int refIdx( char st, char cnt, bool mo )
{
    switch( st )
    {
    case '-':
        if( mo ) return 5;
        else return 0;
    case '?':
        if( mo ) return 7;
        else return 2;
    case 'f':// normal flag
        if( mo ) return 6;
        else return 1;
    case 'F':// yellow flags at loss
        return 3;
    case 'r':
        if( cnt == 'M' ) return 8;// red mine
        else if( cnt == 'm' ) return 4;// grey mines at loss
        else return 9+(int)(cnt-'0');
    }

    return 0;
}

void INITrects( sf::RectangleShape* R, float W, float H )
{
    for( size_t i=0; i<4; ++i )
    {
        R[i].setSize( sf::Vector2f(W,H) );
        R[i].setPosition( 50.0f, 80.0f );
        R[i].setOutlineThickness( -3.0f );
    }

    // unsel
    R[0].setFillColor( sf::Color(0, 255, 0) );// green
    R[0].setOutlineColor( sf::Color(0,0,255) );// blue
    // unsel - mo
    R[1].setFillColor( sf::Color(0, 255, 0) );// green
    R[1].setOutlineColor( sf::Color(255,242,0) );// yellow

    // sel
    R[2].setFillColor( sf::Color(255, 0, 0) );// red
    R[2].setOutlineColor( sf::Color(0,0,255) );// blue
    // sel - mo
    R[3].setFillColor( sf::Color(255, 0, 0) );// red
    R[3].setOutlineColor( sf::Color(255,242,0) );// yellow
}

bool INITrects( sf::RectangleShape* R, sf::Texture& buttTxt, const char* fName )
{
    std::ifstream inFile( fName );
    std::string imgFname;
    if( !inFile ) return false;
    inFile >> imgFname;
    if( !buttTxt.loadFromFile( imgFname.c_str() ) ) return false;
    int W, H;
    inFile >> W >> H;
    for( size_t i=0; i<4; ++i )
    {
        R[i].setSize( sf::Vector2f(W,H) );
        R[i].setPosition( 50.0f, 80.0f );
        int Lt, Tp;
        inFile >> Lt >> Tp;
        R[i].setTexture( &buttTxt );
        R[i].setTextureRect( sf::IntRect(Lt,Tp,W,H) );
    }

    return true;
}

bool initDigitRects(  sf::IntRect* R, sf::Texture& rTxt, const char* fName )// from texture
{
    std::ifstream inFile( fName );
    std::string imgFname;
    if( !inFile ) return false;
    inFile >> imgFname;
    if( !rTxt.loadFromFile( imgFname.c_str() ) ) return false;
    int W, H, numDigits;
    inFile >> W >> H >> numDigits;
    for( int i=0; i<numDigits; ++i )
    {
        R[i].width = W;
        R[i].height = H;
        inFile >> R[i].left >> R[i].top;
    }

    return true;
}

bool handleEvent( sf::Event rEvent, sf::RenderWindow& rRW )
{
    static int msxi=0, msyi=0;
    static bool ltButtDn = false, rtButtDn = false;

    if( rEvent.type == sf::Event::KeyPressed )
    {
        if( rEvent.key.code == sf::Keyboard::Escape ) return false;
    }
    else if(rEvent.type == sf::Event::MouseButtonReleased)
    {
        if(rEvent.mouseButton.button == sf::Mouse::Right) rtButtDn = false;
        else if(rEvent.mouseButton.button == sf::Mouse::Left)
        {
            if( reset.hit() && reset.sel ) reset.sel = false;
            ltButtDn = false;
        }
    }
    else if(rEvent.type == sf::Event::MouseButtonPressed)
    {
        button::mseX = msxi = rEvent.mouseButton.x;
        button::mseY = msyi = rEvent.mouseButton.y;

        int hitRow=rows, hitCol=cols;
        sf::IntRect tlRect = refTileVec[0].getTextureRect();
        if( msxi > gridLt && msyi > gridTop )
        {
            hitCol = (msxi-gridLt)/tlRect.width; if( hitCol > cols ) hitCol = cols;
            hitRow = (msyi-gridTop)/tlRect.height; if( hitRow > rows ) hitRow = rows;
        }

        if(rEvent.mouseButton.button == sf::Mouse::Right)
        {
        //    rtButtDn = true;
            if( !handleMseRtDn( rRW, hitRow, hitCol, ltButtDn, rtButtDn ) )
            {
                gameOn = false;
                makeEndState();
            }
            rtButtDn = true;
            return true;
        }
        else if(rEvent.mouseButton.button == sf::Mouse::Left)
        {

            if( !handleMseLtDn( rRW, hitRow, hitCol, ltButtDn, rtButtDn ) )
            {
                gameOn = false;
                makeEndState();
            }
            ltButtDn = true;
            return true;
        }
    }
    else if(rEvent.type == sf::Event::MouseMoved)
    {
        button::mseX = msxi = rEvent.mouseMove.x;
        button::mseY = msyi = rEvent.mouseMove.y;

        handleMseOver( msxi, msyi );// return value use?

    }// end if mouse moved

    return true;
}// end of handleEvent()

bool handleMseLtDn( sf::RenderWindow& rRW, int hitRow, int hitCol, bool& ltButtDn, bool& rtButtDn )
{
    // this 1st
    size_t sn = dL1.Nbutts;
    if( dL1.hitLeft( sn ) && sn < dL1.Nbutts )
    {
        if( !loadLevel(sn, plusGame) ) { std::cout << "Could not load level " << sn << '\n'; return false; }
        if( sn != currLevel )
        {
            rRW.create(sf::VideoMode(rwWidth, rwHeight), "Minesweeper", sf::Style::Titlebar | sf::Style::Close );
            rRW.setVerticalSyncEnabled(true); // call it once, after creating the window
        }
    //    mineCnt = mines;// mines is global
        mineDisp = mines;
        currLevel = sn;// currLevel is global
        gameOn = true;
        ltButtDn = true;
        secTimer.restart();
        timeDisp = 0;
        return true;// new
    }

    if( reset.hitLeft() )
    {
        if( !loadLevel(currLevel, plusGame) ) { std::cout << "Could not load level " << currLevel << '\n'; return false; }
   //     mineCnt = mines;
        mineDisp = mines;
        gameOn = true;
        ltButtDn = true;
        secTimer.restart();
        timeDisp = 0;
        return true;
    }

    // clear field spaces
    if( gameOn && (hitRow < rows && hitCol < cols) )
    {
        int spClrd = 0;

        if( grid[hitRow][hitCol].isClearable() )
        {
            spClrd = clearSpace( hitRow, hitCol );
            if( grid[hitRow][hitCol].isMine() )
            {
                std::cout << "Mine Hit! You lose!\n";
                gameOn = false;
                makeEndState();
            }
        }
        else if( rtButtDn && !ltButtDn )
        {
            if( applyChord( hitRow, hitCol, spClrd ) )
            {
                 std::cout << "Mine Hit due to wrong flag placement!\nYou lose!\n";
                gameOn = false;
                makeEndState();
            }
        }

        if( spClrd > 0 )
        {
            ucSpcCnt -= spClrd;
            std::cout << "Lt: cleared " << spClrd << " spaces. ucSpCnt = " << ucSpcCnt << " mineCnt = " << mineDisp.getN() << '\n';
            if( ucSpcCnt <= 0 )// WIN!!
            {
                std::cout << "WIN!!\n";
                gameOn = false;
                makeEndState();
            }
        }
    }
    return true;
}

bool handleMseRtDn( sf::RenderWindow& rRW, int hitRow, int hitCol, bool& ltButtDn, bool& rtButtDn )
{
    // cycle -f?
    char st = grid[hitRow][hitCol].st;
    if( hitRow < rows && hitCol < cols )
    {
        if( st == 'r' )// revealed
        {
            if( ltButtDn && !rtButtDn )// applyChord( hitRow, hitCol );
            {
                int spClrd = 0;
                if( applyChord( hitRow, hitCol, spClrd ) )
                {
                     std::cout << "Mine Hit due to wrong flag placement!\nYou lose!\n";
                    gameOn = false;
                    makeEndState();
                }
                else if( spClrd > 0 )
                {
                    ucSpcCnt -= spClrd;
                    std::cout << "Rt: cleared " << spClrd << " spaces. ucSpCnt = " << ucSpcCnt << " mineCnt = " << mineDisp.getN() << '\n';
                    if( ucSpcCnt <= 0 )// WIN!!
                    {
                        std::cout << "WIN!!\n";
                        gameOn = false;
                        makeEndState();
                    }
                }
            }
        }
        else// not revealed
        {
            if( st == '-' ){ grid[hitRow][hitCol].st = 'f';/* --mineCnt;*/ mineDisp -= 1; }
            else if( st == 'f' ){ grid[hitRow][hitCol].st = '?'; /*++mineCnt;*/ mineDisp += 1; }
            else grid[hitRow][hitCol].st = '-';
            grid[hitRow][hitCol].setTxtRect();
        }
    }
    return gameOn;
}

bool handleMseOver( int msx, int msy )
{
    dL1.MseOver();// level select drop list
    if( reset.MseOver() ) return true;

    if( !gameOn ) return true;

    // mseover field spaces
    int moRow=rows, moCol=cols;
    static int moRowLast=rows, moColLast=cols;
    sf::IntRect tlRect = refTileVec[0].getTextureRect();

    if( msx > gridLt && msy > gridTop )
    {
        moCol = (msx-gridLt)/tlRect.width; if( moCol > cols ) moCol = cols;
        moRow = (msy-gridTop)/tlRect.height; if( moRow > rows ) moRow = rows;
    }

    if( (moRow !=moRowLast || moCol!=moColLast) )
    {
        // out with old
        if( moRowLast < rows && moColLast < cols && grid[moRowLast][moColLast].st != 'r' )
            grid[moRowLast][moColLast].setTxtRect();

        // in with new if not revealed
        if( moRow < rows && moCol < cols && grid[moRow][moCol].st != 'r' )
        {
            grid[moRow][moCol].setTxtRect(true);
            moRowLast = moRow;
            moColLast = moCol;
        }
        else// revealed. no mo to undo next time
        {
            moRowLast = rows;
            moColLast = cols;
        }
    }

    return true;
}

// Recursive. returns # of spaces cleared
int clearSpace( int r, int c )
{
    // base case: space already revealed or it's flagged ( '-' and '?' are clearable )
    if( !grid[r][c].isClearable() ) return 0;

    grid[r][c].st = 'r';// clear the space
    grid[r][c].setTxtRect();
    int numCleared = 1;
    // was a space with zero mines around it hit?
    if( grid[r][c].cnt == '0' )// clear the 8 spaces around this space
    {
        for(int i=0; i<8; ++i)// visit the 8 spaces around it
            if( isInField( r+dr[i], c+dc[i] ) )
                 numCleared += clearSpace( r+dr[i], c+dc[i] );// and so on as required
    }
    return numCleared;
}

// returns true if mine is cleared
bool applyChord( int r, int c, int& numCleared )
{
    if( grid[r][c].st != 'r' ) return false;// must chord on revealed space
    if( grid[r][c].cnt == 'M' ) return false;// cannot chord on a mine

    char flagCount = '0';
    for(int i=0; i<8; ++i)
        if( isInField( r+dr[i], c+dc[i] ) && grid[r+dr[i]][c+dc[i]].st == 'f' )
            ++flagCount;

    bool mineHit = false;
    numCleared = 0;
    if( flagCount == grid[r][c].cnt )// chord may be applied
    {
        for(int i=0; i<8; ++i)
            if( isInField( r+dr[i], c+dc[i] ) )
            {
                if( grid[r+dr[i]][c+dc[i]].isClearable() && grid[r+dr[i]][c+dc[i]].isMine() )
                    mineHit = true;
                numCleared += clearSpace( r+dr[i], c+dc[i] );
            }
    }

    return mineHit;// hit no mine
}

bool isInField( int r, int c, bool omitPerim )// validate position
{
    if( omitPerim )
    {
        if( r < 1 || r >= rows-1 ) return false;
        if( c < 1 || c >= cols-1 ) return false;
    }
    else
    {
        if( r < 0 || r >= rows ) return false;
        if( c < 0 || c >= cols ) return false;
    }

    return true;
}

void makeEndState()// reveals mines and mis-marked flags. Call on game loss (handleEvent)
{
    for( auto& Row : grid )
            for( auto& fs : Row )
            {
                if( fs.isMine() )
                {
                    if( fs.isClearable() )
                    {
                        fs.st = 'r';// clear it
                        fs.cnt = 'm';// grey mine
                        fs.setTxtRect();
                    }
                }
                else if( fs.st == 'f' )// mark as yellow flag
                {
                    fs.st = 'F';// yellow flag
                    fs.setTxtRect();
                }
            }
    return;
}

/*
/ returns # of spaces cleared
int applyChord( int r, int c )
{
    if( grid[r][c].st != 'r' ) return 0;// must chord on revealed space
    if( grid[r][c].cnt == 'M' ) return 0;// cannot chord on a mine

    char flagCount = '0';
    for(int i=0; i<8; ++i)
        if( isInField( r+dr[i], c+dc[i] ) && grid[r+dr[i]][c+dc[i]].st == 'f' )
            ++flagCount;

    int numCleared = 0;
    if( flagCount == grid[r][c].cnt )// chord may be applied
        for(int i=0; i<8; ++i)
            if( isInField( r+dr[i], c+dc[i] ) )
                numCleared += clearSpace( r+dr[i], c+dc[i] );

    return numCleared;
}


bool loadLevel( size_t lvlNum, bool makePlus )// open lvl file. Get rows,cols,mines, init grid
{
    if( lvlNum >= levelFnames.size() ) return false;
    std::ifstream inFile( levelFnames[lvlNum].c_str() );
    if( !inFile ) return false;
    sf::Vector2f tlSz = refTileVec[0].getSize();

    inFile >> rows >> cols >> mines;

    std::vector<bool> mineDist( rows*cols, false );
    for( int i=0; i<mines && i<(int)mineDist.size(); ++i ) mineDist[i] = true;
    std::random_shuffle( mineDist.begin(), mineDist.end() );

    if( makePlus ) { rows += 2; cols += 2; }
    ucSpcCnt = rows*cols-mines;

    grid.resize(rows);
    for( int r=0; r<rows; ++r )
    {
        grid[r].resize(cols);
        for( int c=0; c<cols; ++c )
        {
            if( isInField(r,c,makePlus) && mineDist[r*cols+c] ) { grid[r][c].init( sf::Vector2f( gridLt+c*tlSz.x, gridTop+r*tlSz.y ), true ); continue; }

            char mineCnt = '0';// find # of neighboring mines
                for(int i=0; i<8; ++i)// visit the 8 spaces around it
                    if( isInField( r+dr[i], c+dc[i], makePlus ) )
                        if( mineDist[ (r+dr[i])*cols + c+dc[i] ] ) ++mineCnt;

            grid[r][c].init( sf::Vector2f( gridLt+c*tlSz.x, gridTop+r*tlSz.y ), false, mineCnt );
        }
    }

    if( makePlus )// clear perimeter spaces. Call clearspace() so any auto clears will occur
    {
        // top and bottom
        for(int c=0; c<cols; ++c) { ucSpcCnt -= clearSpace(0,c); ucSpcCnt -= clearSpace(rows-1,c); }
        // left and right sides
        for(int r=1; r<rows-1; ++r) { ucSpcCnt -= clearSpace(r,0); ucSpcCnt -= clearSpace(r,cols-1); }

        for( auto& Row : grid )
            for( auto& fs : Row )
                if( fs.st == 'r' && fs.cnt == '0' )
                    fs.rct.setTextureRect( refTileVec[19].getTextureRect() );
    }

    rwWidth = 2*gridLt + cols*refTileVec[0].getTextureRect().width;
    rwHeight = gridTop + gridBotHgt + rows*refTileVec[0].getTextureRect().height;

    return true;
}

bool hitRect( sf::RectangleShape R )
{
    sf::Vector2f pos = R.getPosition();
    sf::Vector2f sz = R.getSize();
    if( mseX < pos.x || mseX > pos.x+sz.x ) return false;
    if( mseY < pos.y || mseY > pos.y+sz.y ) return false;
    return true;
}

*/



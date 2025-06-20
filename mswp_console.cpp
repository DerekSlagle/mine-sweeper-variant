#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
using std::cout;

struct fieldSpace// rows x cols of these make the field
{
    char cnt;// dual use: 'M' if mine or neighboring mine count
    char st;// state: 3*covered: '-', '?' = clearable, 'f' = flagged + 1 revealed state = 'r'
    void show( std::ostream& os, bool showAll = false );
    void init( bool mined, char Count='0' )
    { st = '-'; cnt = mined ? 'M' : Count; }
};

void fieldSpace::show( std::ostream& os, bool showAll )
{
    if( showAll || st == 'r' )
    {
        if( cnt == '0' ) os << ' ';// don't show the 0's
        else os << cnt;
    }
    else os << st;
}

// core game data
int rows=0, cols=0, mines=0;
fieldSpace**  ppField = nullptr;

// positioning
const int X0 = 5, Y0 = 2;// ul corner of display
int Xf = X0+3, Yf = Y0;

const int y[] = { -1, -1, -1,  1, 1, 1,  0, 0 };// 8 shifts to neighbors
const int x[] = { -1,  0,  1, -1, 0, 1, -1, 1 };// used in functions

bool createField( std::ifstream& is );// regular minesweeper
bool createField( std::ifstream& is, bool makePlus );
bool createField_plus( std::ifstream& is );// marked perimeter at start
void destroyField();// dynamic memory cleanup
int clearSpace( int r, int c );// Returns # of spaces cleared
int applyChord( int r, int c );// Returns # of spaces cleared
bool isInField( int r, int c );// validates position

int getPlay( int& r, int& c );
int queryKey(char key, bool& keyDnLast  );// returns -1, 0, 1 = key down, same or up this call
void gotoxy( int column, int line );// Windows dependent. Moves cursor to x, y
void displayField( std::ostream& os, bool showAll = false );
void displayPlayResult( std::ostream& os, int r, int c, int spacesCleared );

int main()
{
    char gameVer;
    cout << "regular or plus game (r/p)? "; std::cin >> gameVer;

    std::ifstream fin("sample_game_mod.txt");// input file
    if( !fin ) return 1;// no input
    if( !createField( fin , gameVer == 'p' ) ) return 3;
    displayField( cout, true );

    int r = 8, c = 15;
    while( true )
    {
        int spacesCleared = getPlay(r,c);
        if( spacesCleared == -1 ) break;// quit
        displayField( cout );
        displayPlayResult( cout, r, c, spacesCleared );
    }

    destroyField();// cleanup
    return 0;
}// end main()

// returns # spaces cleared or -1 to indicate quit
int getPlay( int& r, int& c )
{
    gotoxy( Xf+c, Yf+r );

    while( true )// until 'c' or 'm' hit
    {
        int rs = r, cs = c;// at start of loop
        int mm = 1;

        static bool Q_dnLast=false;
        if( queryKey(VK_ESCAPE, Q_dnLast) == -1 ) return -1;// Quit Game

        if( (GetAsyncKeyState('T') & 0x8000) )  mm = 5;// move multiplier (Turbo)

        // get play position
        static bool W_dnLast=false, S_dnLast=false, A_dnLast=false, D_dnLast=false;
        if( queryKey(VK_UP, W_dnLast) == -1 ) { if( isInField( r-mm,   c ) ) r -= mm; }// up
        if( queryKey(VK_DOWN, S_dnLast) == -1 ) { if( isInField( r+mm,   c ) ) r += mm; }// down
        if( queryKey(VK_LEFT, A_dnLast) == -1 ) { if( isInField( r  , c-mm ) ) c -= mm;; }// left
        if( queryKey(VK_RIGHT, D_dnLast) == -1 ) { if( isInField( r  , c+mm ) ) c += mm; }// right

        // handle play action
        static bool C_dnLast=false;
        if( queryKey('C', C_dnLast) == -1 )// clear or chord
        {
            if( ppField[r][c].st == 'r' ) return applyChord(r,c);// only action on a revealed space
            return clearSpace(r,c);// clear the space
        }

        static bool M_dnLast=false;
        if( queryKey('M', M_dnLast) == -1 )// cycle mark
        {
            if( ppField[r][c].st == 'r' ) return 0;
            // cycle: - f ? - f ? ...
            if( ppField[r][c].st == '-' ) ppField[r][c].st = 'f';
            else if( ppField[r][c].st == 'f' ) ppField[r][c].st = '?';
            else ppField[r][c].st = '-';

            return 0;
        }

        // for next iteration
        if( r != rs || c != cs ) gotoxy( Xf+c, Yf+r );
    }
}

int queryKey(char key, bool& keyDnLast )// returns -1, 0, 1 = key down, same or up this call
{
    bool keyDnNow = (GetAsyncKeyState(key) & 0x8000);// true when key down
    if( keyDnNow == keyDnLast ) return 0;
    keyDnLast = keyDnNow;// update for next call
    return keyDnNow ? -1:1;
}

void gotoxy( int column, int line )
{
    COORD coord;
    coord.X = column;
    coord.Y = line;
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), coord );
}

void displayField( std::ostream& os, bool showAll )
{
    gotoxy(0,Y0);
    for( int r=0; r<rows; ++r)
    {
        os << "        ";
        for( int c=0; c<cols; ++c) ppField[r][c].show( os, showAll );
        os << '\n';
    }
    gotoxy( 10+X0, rows+Yf );
    cout << "                                           ";
    gotoxy( 10+X0, rows+Yf );
}

void displayPlayResult( std::ostream& os, int r, int c, int spacesCleared )
{
    if( spacesCleared == 1 )
    {
        if( ppField[r][c].cnt == 'M' ) os << "MINE - YOU LOSE\n";
        else os << "NO MINE - " << ppField[r][c].cnt << " SURROUNDING IT\n";// one space cleared
    }
    else if( spacesCleared > 1 ) os << "NO MINE - " <<  spacesCleared << " SQUARES REVEALED\n";// fill clear triggered
    else if( ppField[r][c].st != 'r' ) os << "Mark changed to: " << ppField[r][c].st << '\n';// 0 cleared = '-', 'f' or '?' marks cycled
    else os << "invalid play\n";
}

bool createField( std::ifstream& is, bool makePlus )
{
    if( is >> rows >> cols )// stream good?
    {
        if( makePlus ) { rows += 2; cols += 2; }// the plus
        mines = 0;// read in the mine distribution
        bool* mineDist = new bool[rows*cols];
        for( int i=0; i < rows*cols; ++i )
        {                 //   top              bottom            left            right
            if( makePlus && ( i < cols || i >= cols*(rows-1) || i%cols == 0 || i%cols == cols-1 ) )
            { mineDist[i] = false; continue; }

            char chIn;
            if( !(is >> chIn) ) { cout << "cf_bail!"; delete [] mineDist; return false; }// input failure
            if( chIn == 'X' ) { mineDist[i] = true; ++mines; }
            else mineDist[i] = false;
        }

        // build field
        ppField = new fieldSpace*[rows];
        for(int r=0; r<rows; ++r)
        {
            ppField[r] = new fieldSpace[cols];
            for(int c=0; c<cols; ++c)
            {
                if( mineDist[r*cols+c] ) { ppField[r][c].init( true ); continue; }// MINE - noted. next space!

                char mineCnt = '0';// find # of neighboring mines
                for(int i=0; i<8; ++i)// visit the 8 spaces around it
                    if( isInField( r+y[i], c+x[i] ) )
                        if( mineDist[ (r+y[i])*cols + c+x[i] ] ) ++mineCnt;

                ppField[r][c].init( false, mineCnt );// NOT a mine
            }
        }

        if( makePlus )// clear perimeter spaces. Call clearspace() so any auto clears will occur
        {
            // top and bottom
            for(int c=0; c<cols; ++c) { clearSpace(0,c); clearSpace(rows-1,c); }
            // left and right sides
            for(int r=1; r<rows-1; ++r) { clearSpace(r,0); clearSpace(r,cols-1); }
        }

        delete [] mineDist;
        return true;
    }// end if

    return false;
}// end of createField_plus

void destroyField()
{
    if( ppField )
    {
        for(int r=0; r<rows; ++r) delete [] ppField[r];
        delete [] ppField;
    }
    ppField = nullptr;// safed off?
}

bool isInField( int r, int c )// validate position
{
    if( r < 0 || r >= rows ) return false;
    if( c < 0 || c >= cols ) return false;
    return true;
}

// Recursive. returns # of spaces cleared
int clearSpace( int r, int c )
{
    // base case: space already revealed or it's flagged ( '-' and '?' are clearable )
    if( ppField[r][c].st == 'r' || ppField[r][c].st == 'f' ) return 0;

    ppField[r][c].st = 'r';// clear the space
    int numCleared = 1;
    // was a space with zero mines around it hit?
    if( ppField[r][c].cnt == '0' )// clear the 8 spaces around this space
    {
        for(int i=0; i<8; ++i)// visit the 8 spaces around it
            if( isInField( r+y[i], c+x[i] ) )
                 numCleared += clearSpace( r+y[i], c+x[i] );// and so on as required
    }
    return numCleared;
}

// returns # of spaces cleared
int applyChord( int r, int c )
{
    if( ppField[r][c].st != 'r' ) return 0;// must chord on revealed space
    if( ppField[r][c].cnt == 'M' ) return 0;// cannot chord on a mine

    char flagCount = '0';
    for(int i=0; i<8; ++i)
        if( isInField( r+y[i], c+x[i] ) && ppField[r+y[i]][c+x[i]].st == 'f' )
            ++flagCount;

    int numCleared = 0;
    if( flagCount == ppField[r][c].cnt )// chord may be applied
        for(int i=0; i<8; ++i)
            if( isInField( r+y[i], c+x[i] ) )
                numCleared += clearSpace( r+y[i], c+x[i] );

    return numCleared;
}

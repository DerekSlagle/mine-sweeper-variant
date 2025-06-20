#include "digitDisplay.h"

digitDisplay::digitDisplay() : pIR(nullptr), N(0) {}

digitDisplay::digitDisplay( size_t numDigits, const sf::Texture& rTxt, sf::IntRect* pIrct, const sf::Text& Label, float X, float Y, int n )
    : label(Label), dgtVec(numDigits), pIR(pIrct), N(n)
{
    for( size_t i=0; i<dgtVec.size(); ++i )
    {
        dgtVec[i].setTexture(&rTxt);
        dgtVec[i].setTextureRect( *pIR );// 0
        dgtVec[i].setPosition( sf::Vector2f( 20.0f + X+pIR->width*i, Y + 5.0f ) );
        dgtVec[i].setSize( sf::Vector2f(pIR->width,pIR->height) );
    }

    if( n != 0 ) setN(N);

    bkgdRect.setSize( sf::Vector2f( 40.0f + pIR->width*numDigits, pIR->height + 25.0f ) );
    bkgdRect.setFillColor( sf::Color(0,128,0) );// forest green
    bkgdRect.setPosition( sf::Vector2f( X, Y ) );
    bkgdRect.setOutlineColor( sf::Color(0,0,255) );
    bkgdRect.setOutlineThickness(2.0f);

    negSign.setSize( sf::Vector2f(12.0f,4.0f) );
    negSign.setFillColor( sf::Color(255,242,0) );// yellow
    negSign.setPosition( sf::Vector2f( X+5.0f, Y+15.0f ) );

    label.setPosition( sf::Vector2f( X+20.0f, Y+5+pIR->height ) );
}

void digitDisplay::init( size_t numDigits, const sf::Texture& rTxt, sf::IntRect* pIrct, const sf::Text& Label, float X, float Y, int n )
{
    dgtVec.resize(numDigits);
    pIR = pIrct;
    N = n;
    for( size_t i=0; i<dgtVec.size(); ++i )
    {
        dgtVec[i].setTexture(&rTxt);
        dgtVec[i].setTextureRect( *pIR );// 0
        dgtVec[i].setPosition( sf::Vector2f( 20.0f + X+pIR->width*i, Y + 5.0f ) );
        dgtVec[i].setSize( sf::Vector2f(pIR->width,pIR->height) );
    }

    if( n != 0 ) setN(N);

    bkgdRect.setSize( sf::Vector2f( 40.0f + pIR->width*numDigits, pIR->height + 25.0f ) );
    bkgdRect.setFillColor( sf::Color(0,128,0) );// forest green
    bkgdRect.setPosition( sf::Vector2f( X, Y ) );
    bkgdRect.setOutlineColor( sf::Color(0,0,255) );
    bkgdRect.setOutlineThickness(2.0f);

    negSign.setSize( sf::Vector2f(12.0f,4.0f) );
    negSign.setFillColor( sf::Color(255,242,0) );// yellow
    negSign.setPosition( sf::Vector2f( X+5.0f, Y+15.0f ) );

    label = Label;
    label.setPosition( sf::Vector2f( X+20.0f, Y+5+pIR->height ) );
}

bool digitDisplay::init( const char* fileName, const sf::Texture& rTxt, sf::IntRect* pIrct, const sf::Font& font, float X, float Y, int n )
{
    std::ifstream inFile( fileName );
    if( !inFile ) return false;

    pIR = pIrct;

    char useGiven = 'n';
    inFile >> useGiven;
    float x, y;// position
    if( useGiven == 'y' ) { x = X; y = Y; }
    else { inFile >> x >> y; }

    inFile >> useGiven;// N assigned
    if( useGiven == 'y' ) { N = n; }
    else { inFile >> N; }

    // digits
    size_t numDigits = 0;
    inFile >> numDigits;
    dgtVec.resize(numDigits);
    float ofstX, ofstY;
    inFile >> ofstX >> ofstY;
    for( size_t i=0; i<dgtVec.size(); ++i )
    {
        dgtVec[i].setTexture(&rTxt);
        dgtVec[i].setTextureRect( *pIR );// 0
        dgtVec[i].setPosition( sf::Vector2f( ofstX + x + pIR->width*i, y + ofstY ) );
        dgtVec[i].setSize( sf::Vector2f(pIR->width,pIR->height) );
    }

    // bkgdRect
    float endX, endY;// bkgdRect size
    inFile >> endX >> endY;
    bkgdRect.setSize( sf::Vector2f( ofstX+endX+numDigits*pIR->width, ofstY+endY+pIR->height ) );
    bkgdRect.setPosition( sf::Vector2f(x,y) );
    size_t red, grn, blue;
    inFile >> red >> grn >> blue;  bkgdRect.setFillColor( sf::Color(red,grn,blue) );
    inFile >> red >> grn >> blue;  bkgdRect.setOutlineColor( sf::Color(red,grn,blue) );
    float f1, f2; inFile >> f1;// outline thickness
    bkgdRect.setOutlineThickness(f1);

    // negSign
    inFile >> f1 >> f2;           negSign.setSize( sf::Vector2f(f1,f2) );
    inFile >> red >> grn >> blue; negSign.setFillColor( sf::Color(red,grn,blue) );
    inFile >> ofstX >> ofstY;     negSign.setPosition( sf::Vector2f( x+ofstX, y+ofstY ) );

    // label
    label.setFont(font);
    size_t fontSz; inFile >> fontSz; label.setCharacterSize(fontSz);
    inFile >> red >> grn >> blue; label.setColor( sf::Color(red,grn,blue) );
    inFile >> ofstX >> ofstY;  label.setPosition( sf::Vector2f( x+ofstX, y+ofstY ) );
    inFile.ignore(256,'\n');
    std::string Label; getline( inFile, Label ); label.setString( Label.c_str() );// last input!!

    if( N != 0 ) setN(N);

    return true;
}

digitDisplay::~digitDisplay()
{
    //dtor
}

void digitDisplay::setN( int n )
{
    if( !pIR ) return;

    N = n;
    if( n < 0 ) n *= -1;
    size_t Ndgt = dgtVec.size();
    for( size_t i=1; i<=Ndgt; ++i )
    {
        dgtVec[Ndgt-i].setTextureRect( pIR[n%10] );
        n /= 10;
    }
}

void digitDisplay::draw( sf::RenderWindow& rRW )const
{
    rRW.draw(bkgdRect);
    rRW.draw(label);
    if( N < 0 ) rRW.draw(negSign);
    for( const sf::RectangleShape& dgt : dgtVec )
        rRW.draw(dgt);
}

void digitDisplay::setPosition( sf::Vector2f Pos )
{
    float ofstX = label.getPosition().x - bkgdRect.getPosition().x;// to label
    float ofstY = label.getPosition().y - bkgdRect.getPosition().y;// to label
    // label = sf:Text
    label.setPosition( sf::Vector2f(Pos.x+ofstX, Pos.y+ofstY) );

    // bkgdRect, negSign, digitVec[] = RectangleShape
    bkgdRect.setPosition( Pos );
    negSign.setPosition( sf::Vector2f( Pos.x+5.0f, Pos.y+15.0f ) );
    for( size_t i=0; i<dgtVec.size(); ++i )
        dgtVec[i].setPosition( sf::Vector2f( 20.0f + Pos.x+pIR->width*i, Pos.y + 5.0f ) );
}

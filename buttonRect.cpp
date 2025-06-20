#include "buttonRect.h"

buttonRect::buttonRect(): sz(10.0f,10.0f)
{
    //ctor
}

buttonRect::~buttonRect()
{
    //dtor
}

bool buttonRect::hit()const
{
    if( mseX < pos.x || mseX > pos.x+sz.x ) return false;
    if( mseY < pos.y || mseY > pos.y+sz.y ) return false;
    return true;
}

void buttonRect::draw( sf::RenderWindow& rRW )const
{
    size_t idx = 0;
    if( sel ) idx = 2;
    if( mseOver ) ++idx;

    rRW.draw( R[idx] );
    rRW.draw(title);
}

void buttonRect::init( const sf::RectangleShape* pR, float x, float y, const sf::Text& Title )
{

    pos.x = x;
    pos.y = y;
    sz = pR->getSize();

    title = Title;
    sf::FloatRect titleSz = title.getLocalBounds();
    sf::Vector2f dPos;
    dPos.x = (sz.x - titleSz.width)/2.0f;
    dPos.y = (sz.y - titleSz.height)/2.0f;
    title.setPosition(pos+dPos);

    for( size_t i=0; i<4; ++i )
    {
        R[i] = pR[i];
        R[i].setPosition(pos);
    }
}

void buttonRect::setPosition( sf::Vector2f Pos )
{
    pos = Pos;
    sf::Vector2f dPos;
    sf::FloatRect titleSz = title.getLocalBounds();
    dPos.x = (sz.x - titleSz.width)/2.0f;
    dPos.y = (sz.y - titleSz.height)/2.0f;
    title.setPosition(pos+dPos);

    for( size_t i=0; i<4; ++i )
        R[i].setPosition(pos);
}


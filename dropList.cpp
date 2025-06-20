#include "dropList.h"

dropList::dropList(){}

dropList::dropList( const sf::Text& homeTitle, const std::vector<sf::Text>& titleVec, const sf::RectangleShape* pR, float x, float y, size_t Persist )
{
    init( homeTitle, titleVec, pR, x, y, Persist );
}

dropList::~dropList()
{
    //dtor
}

void dropList::init( const sf::Text& homeTitle, const std::vector<sf::Text>& titleVec, const sf::RectangleShape* pR, float x, float y, size_t Persist )
{
    persist = Persist;
    homeButt.init( pR, x, y, homeTitle );
    Nbutts = titleVec.size();
    selNum = Nbutts;
    listArea.left = x;
    listArea.width = homeButt.sz.x;
    listArea.top = y + homeButt.sz.y;
    listArea.height = Nbutts*homeButt.sz.y;

    buttVec.resize(Nbutts);
    for( size_t i=0; i<Nbutts; ++i )
        buttVec[i].init( pR, x, y+(i+1)*homeButt.sz.y, titleVec[i] );
}

void dropList::draw( sf::RenderWindow& rRW )const
{
    homeButt.draw(rRW);
    if( homeButt.sel )
        for( const buttonRect& b : buttVec ) b.draw(rRW);
}

bool dropList::hitLeft( size_t& SelNum )
{
    if( homeButt.hitLeft() )
    {
        if( homeButt.sel && selNum < Nbutts )// list opening
        {
            buttVec[selNum].sel = false;
            buttVec[selNum].mseOver = false;
            selNum = SelNum = Nbutts;
        }
        return false;
    }
    else if( homeButt.sel && listHit() )
    {
        for( size_t i=0; i<Nbutts; ++i )
        {
            if( buttVec[i].hit() )
            {
                if( selNum < Nbutts ) buttVec[selNum].sel = false;
                selNum = SelNum = i;
                 if( persist < 3 ) homeButt.sel = false;// select = close list
                return true;
            }
        }
    }

    // miss = close list
    if( persist < 2 ){ homeButt.sel = false; }
    return false;
}

bool dropList::listHit()const
{
    if( button::mseX < listArea.left ) return false;
    if( button::mseX > listArea.left + listArea.width ) return false;
    if( button::mseY < listArea.top ) return false;
    if( button::mseY > listArea.top + listArea.height ) return false;
    return true;
}

void dropList::MseOver()
{
    if( homeButt.MseOver() ) return;
    if( homeButt.sel )// list open
        for( auto& b : buttVec ) b.MseOver();
}

void dropList::setPosition( sf::Vector2f Pos )
{
    listArea.left = Pos.x;
 //   listArea.width = homeButt.sz.x;
    listArea.top = Pos.y + homeButt.sz.y;
//    listArea.height = Nbutts*homeButt.sz.y;

    homeButt.setPosition( Pos );
    for( size_t i=0; i<Nbutts; ++i )
        buttVec[i].setPosition( sf::Vector2f( Pos.x, Pos.y+(i+1)*homeButt.sz.y) );
}

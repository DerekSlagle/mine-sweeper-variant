#include "button.h"

float button::mseX = 0.0f, button::mseY = 0.0f;

button::button(): mseOver(false), sel(false), pos(0.0f,0.0f) {}

button::~button()
{
    //dtor
}

bool button::MseOver()// assigns mseOver
{
    if(  hit() ) mseOver = true;
    else mseOver = false;

    return mseOver;
}

bool button::hitLeft()// alters sel
{
    if( hit() )
    {
        sel = !sel;// toggle
        return true;
    }
    return false;
}

void button::setPosition( sf::Vector2f Pos )
{
    pos = Pos;
}

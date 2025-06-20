#ifndef BUTTONRECT_H
#define BUTTONRECT_H

#include "button.h"


class buttonRect : public button
{
    public:
        sf::Vector2f sz;
        sf::RectangleShape R[4];
        sf::Text title;

        buttonRect();
        virtual ~buttonRect();

        virtual bool hit()const;
        virtual void draw( sf::RenderWindow& rRW )const;
        virtual void setPosition( sf::Vector2f Pos );

        void init( const sf::RectangleShape* pR, float x, float y, const sf::Text& Title = sf::Text() );

    protected:

    private:
};

#endif // BUTTONRECT_H

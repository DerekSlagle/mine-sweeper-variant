#ifndef DROPLIST_H
#define DROPLIST_H

#include<vector>
#include<iostream>
#include "buttonRect.h"

class dropList
{
    public:
        buttonRect homeButt;
        std::vector<buttonRect> buttVec;
        size_t Nbutts;// = buttVec.size()
        size_t selNum;// 0 to Nbutts-1
        size_t persist;// 1=close on list miss, 2=close on list select, 3=close on homeButt only
        sf::FloatRect listArea;

        dropList();
        dropList( const sf::Text& homeTitle, const std::vector<sf::Text>& titleVec, const sf::RectangleShape* pR, float x, float y, size_t Persist=1 );
        ~dropList();

        void init( const sf::Text& homeTitle, const std::vector<sf::Text>& titleVec, const sf::RectangleShape* pR, float x, float y, size_t Persist=1 );
        void draw( sf::RenderWindow& rRW )const;
        bool hitLeft( size_t& SelNum );
        void MseOver();
        bool listHit()const;
        void setPosition( sf::Vector2f Pos );

    protected:

    private:
};

#endif // DROPLIST_H

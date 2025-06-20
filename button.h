#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>

class button
{
    public:
        static float mseX, mseY;
        bool mseOver, sel;
        sf::Vector2f pos;

        button();
        virtual ~button();

        virtual bool hit()const = 0;
        virtual void draw( sf::RenderWindow& rRW )const = 0;
        virtual void setPosition( sf::Vector2f Pos );
        bool MseOver();// assigns mseOver
        bool hitLeft();// alters sel

    protected:

    private:
};

#endif // BUTTON_H

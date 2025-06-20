#ifndef DIGITDISPLAY_H
#define DIGITDISPLAY_H

#include <fstream>
#include <string>
#include<vector>
#include <SFML/Graphics.hpp>

class digitDisplay
{
    public:
        sf::RectangleShape negSign;
        sf::RectangleShape bkgdRect;
        sf::Text label;
        std::vector<sf::RectangleShape> dgtVec;
        sf::IntRect* pIR;
        int N;// # to display
        digitDisplay( size_t numDigits, const sf::Texture& rTxt, sf::IntRect* pIrct, const sf::Text& Label, float X, float Y, int n=0 );
        digitDisplay();
        void init( size_t numDigits, const sf::Texture& rTxt, sf::IntRect* pIrct, const sf::Text& Label, float X, float Y, int n=0 );
        bool init( const char* fileName, const sf::Texture& rTxt, sf::IntRect* pIrct, const sf::Font& font, float X=0.0f, float Y=0.0f, int n=0 );
        void setN( int n );
        int getN()const { return N; }
        void draw( sf::RenderWindow& rRW )const;
        void setPosition( sf::Vector2f Pos );
        digitDisplay& operator+=(int inc ) { setN(N+inc); return *this; }
        digitDisplay& operator-=(int inc ) { return (*this += -inc); }
        digitDisplay& operator=(int inc ) { setN(inc); return *this; }
        ~digitDisplay();

    protected:

    private:
};

#endif // DIGITDISPLAY_H

#pragma once
#pragma warning(disable:4996)

#include <string>
#include <SFML/Graphics.hpp>
#include <queue>

#define HEIGHT 600
#define WIDTH 800

class PaintWindow
{
public:
    PaintWindow();

    std::string GetDateAndTime() const;

private:
    sf::RenderTexture renderTexture;
    sf::Sprite sprite;
    sf::Vector2<int> mousePos;
    sf::Shape* shape;
    sf::RenderWindow* window;
    std::queue<sf::Vector2<int>> mouseLocations;
    bool MouseDown = false;

    void DrawBetweenPoints(sf::Vector2<int> a, sf::Vector2<int> b);

    static float Lerp(float a, float b, float t);
};

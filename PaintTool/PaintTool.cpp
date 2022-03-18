#pragma warning(disable:4996)

#include <SFML/Graphics.hpp>
#include <time.h>
#include <chrono>
#include <ctime>  


#define HEIGHT 600
#define WIDTH 800



int main()
{
    sf::Texture renderTexture;
    sf::Sprite sprite;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Paint Tool v0.1");
    sf::CircleShape shape(5.f);
    sf::Vector2<int> mousePos;
    bool MouseDown = false;
    
    shape.setFillColor(sf::Color::Green);
    renderTexture.create(WIDTH, HEIGHT);
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed)
                MouseDown = true;

            if (event.type == sf::Event::MouseButtonReleased)
                MouseDown = false;

            if (event.type == sf::Event::KeyPressed)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                {
                    std::time_t t = std::time(0);
                    std::tm* tm = std::localtime(&t);
                    std::string year = std::to_string(tm->tm_year + 1900);
                    std::string month = std::to_string(tm->tm_mon + 1);
                    std::string day = std::to_string(tm->tm_mday);
                    std::string hour = std::to_string(tm->tm_hour);
                    std::string minute = std::to_string(tm->tm_min);
                    std::string second = std::to_string(tm->tm_sec);
                    std::string TimeStamp = year + "-" += month + "-" += day + "_" += hour + "-" += minute + "-" += second;
                    renderTexture.copyToImage().saveToFile("Saved/" + TimeStamp + ".png");
                }
            }
        }
        mousePos = sf::Mouse::getPosition(window);

        if (MouseDown)
        {
            shape.setPosition(mousePos.x - 5.f, mousePos.y - 5.f);
            window.draw(shape);
            renderTexture.update(window);
        }
        
        window.clear(sf::Color::White);
        sprite.setTexture(renderTexture);
        window.draw(sprite);
        window.display();
    }

    return 0;
}
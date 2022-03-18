#include "PaintWindow.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <ctime>

#include <time.h>



PaintWindow::PaintWindow()
{
    shape = new sf::CircleShape(freedrawSize);
    shape->setFillColor(sf::Color(211, 124, 170, 255));
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Paint Tool v0.1");
    

    renderTexture.create(WIDTH, HEIGHT);

    window->setFramerateLimit(60);
    
    
    while (window->isOpen())
    {
        
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window->close();
            }

            if (event.type == sf::Event::MouseButtonPressed)
                MouseDown = true;

            if (event.type == sf::Event::MouseButtonReleased)
            {
                MouseDown = false;
                while (!mouseLocations.empty())
                    mouseLocations.pop();
            }

            if (event.type == sf::Event::MouseWheelMoved)
            {
                freedrawSize += event.mouseWheel.delta;
                if (freedrawSize < 1) freedrawSize = 1;
                dynamic_cast<sf::CircleShape*>(shape)->setRadius(freedrawSize);
                
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                {

                    renderTexture.getTexture().copyToImage().saveToFile("Saved/" + GetDateAndTime() + ".png");
                }
            }
        }
        
        mousePos = sf::Mouse::getPosition(*window);
        
        
        if (MouseDown)
        {
            
            
            renderTexture.draw(*shape);
            mouseLocations.push(mousePos);
            if (mouseLocations.size() >= 2)
            {
                sf::Vector2<int> p1 = mouseLocations.front();
                mouseLocations.pop();
                sf::Vector2<int> p2 = mouseLocations.front();
                DrawBetweenPoints(p2, p1);
            }
            renderTexture.display();
        }
        
        window->clear(sf::Color::White);
        sprite.setTexture(renderTexture.getTexture());

        shape->setPosition(static_cast<float>(mousePos.x) - freedrawSize, static_cast<float>(mousePos.y) - freedrawSize);
        window->draw(*shape);
        
        window->draw(sprite);
        window->display();
    }
}

std::string PaintWindow::GetDateAndTime() const
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

    return TimeStamp;
}

void PaintWindow::DrawBetweenPoints(sf::Vector2<int> a, sf::Vector2<int> b)
{
    //Get distance between two points
    int distance = static_cast<int>(sqrt(pow(a.x-freedrawSize - b.x-freedrawSize, 2) + pow(a.y-freedrawSize - b.y-freedrawSize, 2)));

    //Draw a dot at every pixel between the two sampled mouse points
    for (float i = 0; i < distance; i += freedrawSize*0.1)
    {
        float alpha = i / distance;
        shape->setPosition(Lerp(a.x-freedrawSize, b.x-freedrawSize, alpha), Lerp(a.y-freedrawSize, b.y-freedrawSize, alpha));
        renderTexture.draw(*shape);
    }
}

float PaintWindow::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

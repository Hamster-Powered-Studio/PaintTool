#include "PaintWindow.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <ctime>

#include <time.h>
#include <Windows.h>
#include <sstream>
#include <string>
#include "imgui_internal.h"


PaintWindow::PaintWindow()
{
    shape = new sf::CircleShape(freedrawSize);
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8.0f;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Paint Tool v0.1");
    window->setActive();

    
    
    ImGui::SFML::Init(*window);
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    float* opacity = new float;
    *opacity = 1.f;

    NewLayer()->GetRenderTexture()->clear(sf::Color::White);
    

    window->setFramerateLimit(60);
    
    sf::Clock deltaClock;
    while (window->isOpen())
    {
        
        
        sf::Event event;
        while (window->pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
            {
                window->close();
            }
            

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (io.WantCaptureMouse) break;
                MouseDown = true;
            }
                

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
                    SaveImage();
                }
            }

            if (event.type == sf::Event::Resized)
            {
                // update the view to the new size of the window
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window->setView(sf::View(visibleArea));
            }
        }


        
        if (!io.WantCaptureMouse && sf::Mouse::getPosition(*window) != mousePos)
        {
            mouseMoved = true;
            mousePos = sf::Mouse::getPosition(*window);
        }
        else mouseMoved = false;
        
        
        if (MouseDown && mouseMoved)
        {
            DrawToLayer(Layers[currentLayer]);
        }
        
        ImGui::SFML::Update(*window, deltaClock.restart());
        
        
        ImGui::Begin("Colour Picker");
        ImGui::ColorPicker3("Colour", circleColor);
        ImGui::SliderFloat("Opacity", opacity, 0.f, 1.f);
        ImGui::End();

        ImGui::Begin("Debug Window");
        ImGui::Text("Mouse Position:");
        ImGui::Text("X: %i   Y: %i", mousePos.x, mousePos.y);
        
        
        std::string Current = "Layer ";
        Current.append(std::to_string(currentLayer));
        
        
        if (ImGui::BeginCombo("Layers", Current.c_str()))
        {
            for (int n = 0; n < Layers.size(); n++)
            {
                std::string Option = "Layer ";
                Option.append(std::to_string(n));
                bool is_selected = (Layers[currentLayer] == Layers[n]);
                
                if (ImGui::Selectable(Option.c_str(), is_selected))
                    currentLayer = n;
                
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        
        
        if (ImGui::Button("New Layer"))
        {
            NewLayer();
        }

        ImGui::Text("Layer Count: %i", Layers.size());
        ImGui::End();

        shape->setFillColor(sf::Color(
        (int)(circleColor[0] * 255),
        (int)(circleColor[1] * 255),
        (int)(circleColor[2] * 255),
         *opacity * 255));

        
        window->clear(sf::Color(200, 200, 200, 255));

        shape->setPosition(static_cast<float>(mousePos.x) - freedrawSize, static_cast<float>(mousePos.y) - freedrawSize);
        

        for (RenderLayer* rt : Layers)
        {
            rt->UpdateTexture();
            window->draw(*rt);
        }

        window->draw(*shape);

        ImGui::SFML::Render(*window);
        window->display();
    }
    ImGui::SFML::Shutdown();
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
        Layers[currentLayer]->GetRenderTexture()->draw(*shape);
    }
}

RenderLayer* PaintWindow::NewLayer()
{
    RenderLayer* layer = new RenderLayer(800, 600);
    Layers.push_back(layer);
    currentLayer = Layers.size() - 1;
    return layer;
}


float PaintWindow::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}


void PaintWindow::DrawToLayer(RenderLayer* layer)
{
    layer->GetRenderTexture()->draw(*shape);
    mouseLocations.push(mousePos);
    
    if (mouseLocations.size() >= 2)
    {
        sf::Vector2<int> p1 = mouseLocations.front();
        mouseLocations.pop();
        sf::Vector2<int> p2 = mouseLocations.front();
        DrawBetweenPoints(p2, p1);
    }
    
    layer->GetRenderTexture()->display();
}

bool PaintWindow::SaveImage()
{
    sf::RenderTexture image;
    image.create(800, 600);
    for (auto layer : Layers)
    {
        image.draw(*layer);
    }
    return image.getTexture().copyToImage().saveToFile("Saved/" + GetDateAndTime() + ".png");
    
}

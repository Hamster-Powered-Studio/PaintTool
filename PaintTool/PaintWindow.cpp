#include "PaintWindow.h"
#include <cmath>
#include <cassert>
#include <ctime>
#include <time.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include <sstream>

namespace fs = std::filesystem;

PaintWindow::PaintWindow()
{
    shape = new sf::CircleShape(freedrawSize);
    window = new sf::RenderWindow(sf::VideoMode(Width, Height), "Paint Tool v0.1");
    
    window->setActive();
    
    ImGui::SFML::Init(*window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGuiIO testio = ImGui::GetIO();
    
    io = &ImGui::GetIO();
    
    NewLayer(Width, Width)->GetRenderTexture()->clear(sf::Color::White);
    window->setFramerateLimit(60);

    WindowLoop();
    
    ImGui::SFML::Shutdown();
}

PaintWindow::~PaintWindow()
{
    delete shape;
    delete window;

    for (RenderLayer* rt : Layers)
    {
        delete rt;
    }
}

void PaintWindow::WindowLoop()
{
    while (window->isOpen())
    {
        
        
        sf::Event event{};
        while (window->pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
            {
                window->close();
            }
            

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (io->WantCaptureMouse) break;
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
                freedrawSize += static_cast<float>(event.mouseWheel.delta);
                if (freedrawSize < 1) freedrawSize = 1;
                dynamic_cast<sf::CircleShape*>(shape)->setRadius(freedrawSize);
                
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                {
                    SaveImage();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                {
                    AtlasTextures(AtlasDimensions[0], AtlasDimensions[1], 823, 1180);
                }
            }

            if (event.type == sf::Event::Resized)
            {
                // update the view to the new size of the window
                sf::FloatRect visibleArea(0, 0, (float)(event.size.width), (float)(event.size.height));
                window->setView(sf::View(visibleArea));
            }
        }


        
        if (!io->WantCaptureMouse && sf::Mouse::getPosition(*window) != mousePos)
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
        
        UI_Atlas();
        UI_ColorPicker();
        UI_Tools();
        UI_Debug();
        
        
        shape->setFillColor(sf::Color(
        static_cast<sf::Uint8>(circleColor[0] * 255),
        static_cast<sf::Uint8>(circleColor[1] * 255),
        static_cast<sf::Uint8>(circleColor[2] * 255),
         static_cast<sf::Uint8>(opacity * 255)));

        
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
    for (float i = 0; i < static_cast<float>(distance); i += freedrawSize * 0.1)
    {
        float alpha = i / distance;
        shape->setPosition(Lerp(a.x-freedrawSize, b.x-freedrawSize, alpha), Lerp(a.y-freedrawSize, b.y-freedrawSize, alpha));
        Layers[currentLayer]->GetRenderTexture()->draw(*shape);
    }
}

void PaintWindow::AtlasTextures(int rows, int columns, int imageWidth, int imageHeight)
{
    std::filesystem::path in = AtlasInput;
    std::string out = AtlasOutput;
    
    if (!fs::exists(in))
    {
        fs::create_directory(AtlasInput);
    }
    if (!fs::exists(out))
    {
        fs::create_directory(AtlasOutput);
    }
    
    sf::Image TestImage;
    int AtlasWidth = imageWidth * columns;
    int AtlasHeight = imageHeight * rows;
    
    sf::RenderTexture* Atlas = new sf::RenderTexture;
    Atlas->create(AtlasWidth * AtlasResolutionScale, AtlasHeight * AtlasResolutionScale);
    int counter = 0;
    for (const auto & entry : fs::directory_iterator(in))
    {
        

        sf::Image Image;
        if (!Image.loadFromFile(entry.path().string())) break;
        
        sf::Vector2u size = Image.getSize();
        sf::Texture Tex;
        Tex.create(size.x, size.y);
        Tex.update(Image);
        sf::Sprite sprite(Tex);
        sprite.setScale(sprite.getScale() * AtlasResolutionScale);
        
        unsigned int imgWidth = size.x;
        unsigned int imgHeight = size.y;
        float posX = counter % columns * imgWidth * AtlasResolutionScale;
        float posY = floor(counter / columns * imgHeight * AtlasResolutionScale);
        
        sprite.setPosition(posX, posY);
        Atlas->draw(sprite);
        
        counter++;
    }

    Atlas->display();
    Atlas->getTexture().copyToImage().saveToFile(out + "/" + GetDateAndTime() + "_ATLAS.png");
}

RenderLayer* PaintWindow::NewLayer(int width, int height)
{
    RenderLayer* layer = new RenderLayer(width, height);
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
    image.create(Width, Height);
    for (auto layer : Layers)
    {
        image.draw(*layer);
    }
    image.display();

    if (!fs::exists("Saved"))
    {
        fs::create_directory("Saved");
    }
    
    return image.getTexture().copyToImage().saveToFile("Saved/" + GetDateAndTime() + ".png");
    
}

void PaintWindow::UI_Debug()
{
    ImGui::Begin("Debug Window");
    ImGui::Text("Mouse Position:");
    ImGui::Text("X: %i   Y: %i", mousePos.x, mousePos.y);
        
        
    std::string Current = "Layer ";
    Current.append(std::to_string(currentLayer));
        
        
    if (ImGui::BeginCombo("Layers", Current.c_str()))
    {
        for (unsigned int n = 0; n < Layers.size(); n++)
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
        NewLayer(Width, Height);
    }

    ImGui::Text("Layer Count: %i", Layers.size());
    ImGui::End();
}

void PaintWindow::UI_ColorPicker()
{
    ImGui::Begin("Colour Picker");
    ImGui::ColorPicker3("Colour", circleColor);
    ImGui::SliderFloat("Opacity", &opacity, 0.f, 1.f);
    ImGui::End();
}

void PaintWindow::UI_Atlas()
{

        ImGui::Begin("Texture Atlas");
            ImGui::PushItemWidth(100);
                ImGui::Text("Path to image folder");
                ImGui::InputText("  ", AtlasInput, IM_ARRAYSIZE(AtlasInput));
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                ImGui::Text("Atlas Dimensions");
                ImGui::DragInt2("Rows * Columns", AtlasDimensions, 1, 1, 20);
                    ImGui::Spacing();
                ImGui::Text("Tile Dimensions");
                ImGui::DragInt2("x * y", TileDimensions, 1, 1, 4096);
                    ImGui::Spacing();
                    ImGui::Spacing();
                ImGui::Text("Resolution Scale");
                ImGui::DragFloat("    ", &AtlasResolutionScale, 0.05, 0.1, 2);
                    ImGui::Spacing();
                    ImGui::Spacing();
                ImGui::Text("Atlas Output Folder");
                ImGui::InputText(" ", AtlasOutput, IM_ARRAYSIZE(AtlasOutput));
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                if (ImGui::Button("Generate Atlas")) AtlasTextures(AtlasDimensions[0], AtlasDimensions[1], TileDimensions[0], TileDimensions[1]) ;
            ImGui::PopItemWidth();
        ImGui::End();
}

void PaintWindow::UI_Tools()
{
    ImGui::Begin("Tools");

    bool update = false;
    if (CurrentTool == Tools::EFREEDRAW)
    {
        update = true;
        ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ActiveButtonColour);
    }
    if(ImGui::Button("Draw")) SetTool(Tools::EFREEDRAW);
    if (update) ImGui::PopStyleColor();

    update = false;
    if (CurrentTool == Tools::ELINE)
    {
        update = true;
        ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ActiveButtonColour);
    }
    ImGui::SameLine();
    if(ImGui::Button("Line")) SetTool(Tools::ELINE);
    if (update) ImGui::PopStyleColor();

    update = false;
    if (CurrentTool == Tools::ECIRCLE)
    {
        update = true;
        ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ActiveButtonColour);
    }
    if(ImGui::Button("Circle")) SetTool(Tools::ECIRCLE);
    if (update) ImGui::PopStyleColor();

    update = false;
    if (CurrentTool == Tools::ERECTANGLE)
    {
        update = true;
        ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ActiveButtonColour);
    }
    ImGui::SameLine();
    if(ImGui::Button("Rect")) SetTool(Tools::ERECTANGLE);
    if (update) ImGui::PopStyleColor();
    ImGui::End();
}
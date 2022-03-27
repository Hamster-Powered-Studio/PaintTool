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

    view.setCenter(Width/2, Height/2);
    view.setSize(Width, Height);
    
    window->setActive();
    
    
    ImGui::SFML::Init(*window);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGuiIO testio = ImGui::GetIO();
    
    io = &ImGui::GetIO();
    
    RenderLayer* layer = NewLayer(Width,  Height);
    layer->GetRenderTexture()->clear(sf::Color::White);
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
        colour = sf::Color(
               static_cast<sf::Uint8>(circleColor[0] * 255),
              static_cast<sf::Uint8>(circleColor[1] * 255),
              static_cast<sf::Uint8>(circleColor[2] * 255),
              static_cast<sf::Uint8>(opacity * 255));

        window->setView(window->getDefaultView());
        window->setView(view);
        
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
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    if (io->WantCaptureMouse) break;
                    MouseDown = true;
                }
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
                {
                    panning = true;
                }
                
            }
                

            if (event.type == sf::Event::MouseButtonReleased)
            {
                MouseDown = false;
                while (!lineMouseLocations.empty())
                    lineMouseLocations.pop();
                mouseDoOnce = false;
                panning = false;

                switch (CurrentTool)
                {
                    
                case Tools::EFREEDRAW: break;
                case Tools::ECIRCLE: Layers[currentLayer]->GetRenderTexture()->draw(circle); break;
                case Tools::ERECTANGLE:
                    {
                        //rectangle.setPosition(rectangle.getPosition().x, rectangle.getPosition().y);
                        Layers[currentLayer]->GetRenderTexture()->draw(rectangle);
                        break;
                    }
                case Tools::ELINE: if (!actionMouseLocations.empty())DrawBetweenPoints(actionMouseLocations.front(), actionMouseLocations.back()); break;
                default: ;
                }
                actionMouseLocations.clear();
            }

            if (event.type == sf::Event::MouseWheelMoved && !io->WantCaptureMouse)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
                {
                    int scale = event.mouseWheel.delta * -20;
                    float aratio = (float)window->getSize().y / (float)window->getSize().x ;
                    view.setSize(( (float)view.getSize().x +  scale), ((float)view.getSize().y + aratio * scale));

                }
                else
                {
                    freedrawSize += static_cast<float>(event.mouseWheel.delta);
                    if (freedrawSize < 1) freedrawSize = 1;
                }
                    

                
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
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                view.reset(visibleArea);
            }
        }

        
        
        if (!io->WantCaptureMouse && sf::Mouse::getPosition(*window) != mousePos)
        {
            mouseMoved = true;
            sf::Vector2i pixelPos = sf::Mouse::getPosition(*window);
            mousePos = (sf::Vector2i)window->mapPixelToCoords(pixelPos, view);
        }
        else mouseMoved = false;
        

        if (MouseDown)
        {
            actionMouseLocations.push_back(mousePos);
            sf::Vector2<int> scale = (actionMouseLocations.back() - actionMouseLocations.front()) ;
            switch (CurrentTool)
            {
            case Tools::EFREEDRAW:
                {
                    DrawToLayer(Layers[currentLayer]);
                    break;
                }
            case Tools::ELINE:
                {
                    /*
                    float scaleFactor = Length(scale);
                    line.setSize(sf::Vector2f(10, freedrawSize));
                    line.setFillColor(colour);
                    float angle = (Angle( actionMouseLocations.back() - actionMouseLocations.front(), actionMouseLocations.back())) * (180/3.141);
                    line.setRotation(angle);
                    std::cout << angle << std::endl;
                    line.setPosition(actionMouseLocations.front().x, actionMouseLocations.front().y);
                    */
                   // lines(sf::LinesStrip, 2)
                   // line.Line
                    shape->setPosition(sf::Vector2f(actionMouseLocations.front().x - freedrawSize, actionMouseLocations.front().y - freedrawSize));
                    line[0].position = sf::Vector2f(actionMouseLocations.front());
                    line[1].position = sf::Vector2f(actionMouseLocations.back());
                    line[0].color = colour;
                    line[1].color = colour;
                    break;
                }
            case Tools::ERECTANGLE:
                {
                    rectangle.setSize(static_cast<sf::Vector2f>(scale));
                    fillShape ? rectangle.setFillColor(colour) :rectangle.setFillColor(sf::Color::Transparent);
                    rectangle.setOutlineColor(colour);
                    fillShape ? rectangle.setOutlineThickness((0)) : rectangle.setOutlineThickness((freedrawSize * 2));
                    //rectangle.setOutlineThickness(1);
                    rectangle.setPosition(actionMouseLocations.front().x, actionMouseLocations.front().y);
                    break;
                }
            case Tools::ECIRCLE:
                {
                    circle.setRadius(1);
                    circle.setScale(scale.x * 0.5, scale.y * 0.5);
                    float scaleFactor = (sqrt(circle.getScale().x * circle.getScale().x + circle.getScale().y * circle.getScale().y));
                    fillShape ? circle.setFillColor(colour) : circle.setFillColor(sf::Color::Transparent);
                    circle.setOutlineColor(colour);
                    fillShape ? circle.setOutlineThickness(0) : circle.setOutlineThickness((freedrawSize * 2) / scaleFactor);
                    circle.setPointCount(16 + (scaleFactor * 0.2));
                    
                    circle.setPosition(actionMouseLocations.front().x, actionMouseLocations.front().y);
                    
                    break;
                }
            }
            mouseDoOnce = true;
        }
        if (panning)
        {
            actionMouseLocations.push_back(mousePos);
            sf::Vector2i deltaMove = actionMouseLocations.front() - actionMouseLocations.back();
            view.move(deltaMove.x, deltaMove.y);
        }
        
        ImGui::SFML::Update(*window, deltaClock.restart());
        
        UI_Atlas();
        UI_ColorPicker();
        UI_Tools();
        UI_Debug();
        UI_Instructions();
        
        
        shape->setFillColor(colour);
        dynamic_cast<sf::CircleShape*>(shape)->setRadius(freedrawSize);
        
        window->clear(sf::Color(200, 200, 200, 255));
        //window->setView(window->getDefaultView());
        if ((CurrentTool == Tools::ELINE && !MouseDown) || CurrentTool != Tools::ELINE) shape->setPosition(static_cast<float>(mousePos.x) - freedrawSize, static_cast<float>(mousePos.y) - freedrawSize);
        
        
        
        for (RenderLayer* rt : Layers)
        {
            rt->UpdateTexture();
            rt->GetRenderTexture()->display();
            if (rt->visible) window->draw(*rt);
        }
        
        
        switch (CurrentTool)
        {
        case Tools::EFREEDRAW: {window->draw(*shape); break;}
        case Tools::ECIRCLE: {if (MouseDown) window->draw(circle); break;}
        case Tools::ERECTANGLE: { if (MouseDown) window->draw(rectangle); break;}
        case Tools::ELINE: {if (MouseDown) window->draw(line); window->draw(*shape); break;}
        default: ;
        }
        
        ImGui::SFML::Render(*window);
        window->display();
    }
}

float PaintWindow::Dot(sf::Vector2<int> a, sf::Vector2<int> b)
{
    return (a.x * b.x) + (a.y * b.y);
}

float PaintWindow::Angle(sf::Vector2<int> a, sf::Vector2<int> b)
{
   // return atan2(Det(b, a), Dot(a, b));
    return acos(Dot(a, b) / (Length(a) * Length(b)));
}

float PaintWindow::Length(sf::Vector2<int> a)
{
    return sqrt((a.x * a.x) + (a.y * a.y));
}

float PaintWindow::Det(sf::Vector2<int> a, sf::Vector2<int> b)
{
    return (a.x * b.y) - (a.y * b.x);
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
    lineMouseLocations.push(mousePos);
    
    if (lineMouseLocations.size() >= 2)
    {
        sf::Vector2<int> p1 = lineMouseLocations.front();
        lineMouseLocations.pop();
        sf::Vector2<int> p2 = lineMouseLocations.front();
        DrawBetweenPoints(p2, p1);
    }
    
    //layer->GetRenderTexture()->display();
}

bool PaintWindow::SaveImage()
{
    sf::RenderTexture image;
    image.create(Width, Height);
    for (auto layer : Layers)
    {
        if (layer->visible) image.draw(*layer);
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
    ImGui::Begin("Info");
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

    ImGui::Checkbox("Visible", &Layers[currentLayer]->visible);
    
    if (ImGui::Button("New Layer"))
    {
        NewLayer(Width, Height);
    }
    

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

    ImGui::Checkbox("Fill Shape", &fillShape);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    ImGui::SliderFloat("Thickness", &freedrawSize, 1, 60);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    
    if (ImGui::Button("Save Image"))
    {
        SaveImage();
    }
    ImGui::End();
}

void PaintWindow::UI_Instructions()
{
    ImGui::Begin("Instructions");
    ImGui::Text("Scroll to change Thickness");
    ImGui::Text("Alt + Scroll to zoom");
    ImGui::Text("Middle mouse to pan canvas");
    ImGui::End();
}
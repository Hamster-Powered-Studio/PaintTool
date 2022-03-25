﻿#pragma once
#pragma warning(disable:4996)

#include <string>
#include <SFML/Graphics.hpp>
#include <queue>

#include "imgui.h"
#include "imgui-SFML.h"
#include "RenderLayer.h"

class PaintWindow
{
public:
    PaintWindow();
    ~PaintWindow();

    std::string GetDateAndTime() const;
    std::vector<RenderLayer*> Layers;

    enum class Tools
    {
        EFREEDRAW,
        ECIRCLE,
        ERECTANGLE,
        ELINE
    };

    void SetTool(Tools tool) { CurrentTool = tool; };
    
private:
    
    sf::RenderTexture renderTexture;
    sf::Sprite sprite;
    sf::Vector2<int> mousePos;
    sf::Shape* shape;
    sf::RenderWindow* window;
    std::queue<sf::Vector2<int>> mouseLocations;
    sf::Clock deltaClock;
    ImGuiIO* io;
    Tools CurrentTool = Tools::EFREEDRAW;
    ImVec4 ActiveButtonColour = ImColor(27, 54, 74);

    int Width = 1600;
    int Height = 900;
    
    unsigned int currentLayer = 0;
    
    void DrawToLayer(RenderLayer* layer);

    bool SaveImage();
    
    float freedrawSize = 5.0f;
    float opacity = 1.f;
    bool MouseDown = false;
    float circleColor[3] = {0.f, 0.f, 0.f};
    bool mouseMoved = false;
    void DrawBetweenPoints(sf::Vector2<int> a, sf::Vector2<int> b);
    int AtlasDimensions[2] = {5, 5};
    int TileDimensions[2] = {823, 1180};
    char AtlasInput[128] = "Saved/In";
    char AtlasOutput[128] = "Saved/Out";
    float AtlasResolutionScale = 1.f;

    void AtlasTextures(int rows, int columns, int imageWidth, int imageHeight);
    void UI_Debug();
    void UI_Tools();
    void UI_ColorPicker();
    void UI_Atlas();
    void WindowLoop();
    
    RenderLayer* NewLayer(int width, int height);
    
    static float Lerp(float a, float b, float t);
};



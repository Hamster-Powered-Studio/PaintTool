#pragma once
#pragma warning(disable:4996)

#include <string>
#include <SFML/Graphics.hpp>
#include <queue>

#include "imgui.h"
#include "imgui-SFML.h"
#include "RenderLayer.h"

#define HEIGHT 600
#define WIDTH 800

class PaintWindow
{
public:
    PaintWindow();

    std::string GetDateAndTime() const;
    std::vector<RenderLayer*> Layers;

private:
    sf::RenderTexture renderTexture;
    sf::Sprite sprite;
    sf::Vector2<int> mousePos;
    sf::Shape* shape;
    sf::RenderWindow* window;
    std::queue<sf::Vector2<int>> mouseLocations;

    unsigned int currentLayer = 0;
    
    void DrawToLayer(RenderLayer* layer);

    bool SaveImage();
    
    float freedrawSize = 5.0f;
    bool MouseDown = false;
    float circleColor[3] = {0.f, 0.f, 0.f};
    bool mouseMoved = false;
    void DrawBetweenPoints(sf::Vector2<int> a, sf::Vector2<int> b);
    int AtlasDimensions[2] = {5, 5};
    int TileDimensions[2] = {823, 1180};
    char AtlasInput[128] = "Saved/In";
    char AtlasOutput[128] = "Saved/Out";
    float AtlasResolutionScale = 1.f;

    sf::RenderTexture* AtlasTextures(int rows, int columns, int imageWidth, int imageHeight);
    
    RenderLayer* NewLayer(int width, int height);
    
    static float Lerp(float a, float b, float t);
};



#pragma once
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

class RenderLayer : public sf::Sprite
{
public:
    RenderLayer();
    RenderLayer(float width, float height);

    sf::RenderTexture* RT = new sf::RenderTexture;

    sf::RenderTexture* GetRenderTexture() {return RT;}

    void UpdateTexture();
    
};

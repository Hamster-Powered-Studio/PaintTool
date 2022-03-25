#include "RenderLayer.h"

RenderLayer::RenderLayer()
{
    RT->create(800, 600);
    //RT->clear(sf::Color::White);
}

RenderLayer::RenderLayer(float width, float height)
{
    RT->create(width, height);
    //RT->clear(sf::Color::White);
}



void RenderLayer::UpdateTexture()
{
    setTexture(GetRenderTexture()->getTexture());
}

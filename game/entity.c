#include "entity.h"
#include <stdlib.h>

Entity *createEntity(float x, float y, float width, float height, Sprite *sprite, int layer)
{
    Entity *entity = malloc(sizeof(Entity));
    entity->x = x;
    entity->y = y;
    entity->width = width;
    entity->height = height;
    entity->sprite = sprite;
    entity->visible = true;
    entity->layer = layer;
    entity->hitbox.x = x;
    entity->hitbox.y = y;
    entity->hitbox.width = width;
    entity->hitbox.height = height;
    return entity;
}

void updateEntity(Entity *entity)
{
    if (entity->sprite)
    {
        updateSprite(entity->sprite);
    }
}

void renderEntity(Entity *entity, SDL_Renderer *renderer)
{
    if (entity->visible && entity->sprite)
    {
        renderSprite(entity->sprite, renderer, (int)entity->x, (int)entity->y);
    }
}

void setHitbox(Entity *entity, float x, float y, float width, float height)
{
    entity->hitbox.x = x;
    entity->hitbox.y = y;
    entity->hitbox.width = width;
    entity->hitbox.height = height;
}

Hitbox getHitbox(Entity *entity)
{
    return entity->hitbox;
}

void drawHitbox(Entity *entity, SDL_Renderer *renderer)
{
    // rectangle rouge autour de l'entité
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Alpha à 255 au lieu de 128
    SDL_Rect rect = {(int)entity->hitbox.x, (int)entity->hitbox.y, (int)entity->hitbox.width, (int)entity->hitbox.height};
    SDL_RenderDrawRect(renderer, &rect);
}
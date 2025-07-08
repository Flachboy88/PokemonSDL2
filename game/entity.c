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

// Modified to use camera
void renderEntity(Entity *entity, SDL_Renderer *renderer, Camera *camera)
{
    if (entity->visible && entity->sprite)
    {
        SDL_Rect screen_rect = getScreenRect(camera, entity->x, entity->y, entity->width, entity->height);
        renderSprite(entity->sprite, renderer, screen_rect.x, screen_rect.y);
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

void drawHitbox(Entity *entity, SDL_Renderer *renderer, Camera *camera)
{
    // rectangle rouge autour de l'entité
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Alpha à 255 au lieu de 128
    SDL_Rect screen_rect = getScreenRect(camera, entity->hitbox.x, entity->hitbox.y, entity->hitbox.width, entity->hitbox.height);
    SDL_RenderDrawRect(renderer, &screen_rect);
}
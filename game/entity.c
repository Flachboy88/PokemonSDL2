#include "entity.h"
#include <stdlib.h>

Entity* createEntity(float x, float y, float width, float height, Sprite *sprite, int layer) {
    Entity *entity = malloc(sizeof(Entity));
    entity->x = x;
    entity->y = y;
    entity->width = width;
    entity->height = height;
    entity->sprite = sprite;
    entity->visible = true;
    entity->layer = layer;
    return entity;
}

void updateEntity(Entity *entity) {
    if (entity->sprite) {
        updateSprite(entity->sprite);
    }
}

void renderEntity(Entity *entity, SDL_Renderer *renderer) {
    if (entity->visible && entity->sprite) {
        renderSprite(entity->sprite, renderer, (int)entity->x, (int)entity->y);
    }
}
#ifndef ENTITY_H
#define ENTITY_H

#include "../framework/sprite.h"
#include <stdbool.h>

typedef struct
{
    float x, y;          // Position
    float width, height; // Taille
    Sprite *sprite;      // Sprite de l'entité
    bool visible;        // Visibilité
    int layer;           // Layer de rendu (pour tri)
} Entity;

Entity *createEntity(float x, float y, float width, float height, Sprite *sprite, int layer);
void updateEntity(Entity *entity);
void renderEntity(Entity *entity, SDL_Renderer *renderer);

#endif
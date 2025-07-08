#ifndef ENTITY_H
#define ENTITY_H

#include "../framework/sprite.h"
#include "../systems/camera.h"
#include <stdbool.h>

// Hitbox
typedef struct
{
    float x, y;
    float width, height;
} Hitbox;

typedef struct
{
    Hitbox hitbox;       // Hitbox de l'entité
    float x, y;          // Position
    float width, height; // Taille
    Sprite *sprite;      // Sprite de l'entité
    bool visible;        // Visibilité
    int layer;           // Layer de rendu (pour tri)
} Entity;

Entity *createEntity(float x, float y, float width, float height, Sprite *sprite, int layer);
void updateEntity(Entity *entity);
void renderEntity(Entity *entity, SDL_Renderer *renderer, Camera *camera);
Hitbox getHitbox(Entity *entity);
void setHitbox(Entity *entity, float x, float y, float width, float height);
void drawHitbox(Entity *entity, SDL_Renderer *renderer, Camera *camera);

#endif
#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>
#include "../systems/inputs.h"

typedef struct
{
    Entity entity;         // Hérite d'Entity
    float speed;           // Vitesse de déplacement
    bool moving;           // En mouvement
    SDL_RendererFlip flip; // Direction (flip horizontal)
    const char *currentAnimationName;
    int lastDirection;

    float targetX, targetY; // Position cible (multiple de 16)
    bool hasTarget;
} Player;

Player *InitPlayer(float x, float y, SDL_Renderer *renderer);
void updatePlayer(Player *player, float deltaTime);
void updatePlayerWithInput(Player *player, Input *input, float deltaTime);
void renderPlayer(Player *player, SDL_Renderer *renderer);
void freePlayer(Player *player);
char *derniereDir(Player *player);
void deplacement(Player *player, int dir, float deltaTime);
const char *walkAnimFromDir(int dir);

#endif
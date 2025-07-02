#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    Entity entity;          // Hérite d'Entity
    float speed;            // Vitesse de déplacement
    int health;             // Points de vie
    bool moving;            // En mouvement
    SDL_RendererFlip flip;  // Direction (flip horizontal)
} Player;

Player* createPlayer(float x, float y, SDL_Renderer *renderer);
void updatePlayer(Player *player, float deltaTime);
void renderPlayer(Player *player, SDL_Renderer *renderer);
void freePlayer(Player *player);

#endif
#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>
#include "../systems/inputs.h"
#include "../framework/map.h"

typedef enum
{
    WALK_MOD,
    RUN_MOD,
    BIKE_MOD
} PlayerMode;

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

    PlayerMode mode;

    Sprite *walkSprite;
    Sprite *runSprite;
    Sprite *bikeSprite;
    Sprite *currentSprite;

} Player;

Player *InitPlayer(float x, float y, SDL_Renderer *renderer);
void updatePlayer(Player *player, float deltaTime);
void updatePlayerWithInput(Player *player, Input *input, float deltaTime, Map *map);
void renderPlayer(Player *player, SDL_Renderer *renderer);
void freePlayer(Player *player);
char *derniereDir(Player *player);
void deplacement(Player *player, int dir, float deltaTime, Map *map);
const char *walkAnimFromDir(int dir);
bool checkCollisionWithMap(Player *player, float newX, float newY, Map *map);
bool pointInPolygon(Point point, Point *polygon, int count);
bool rectangleIntersectsPolygon(Hitbox rect, Point *polygon, int count);
void setPlayerMode(Player *player, PlayerMode mode);
PlayerMode getPlayerMode(Player *player);
void setPlayerSpeedMode(Player *player);
void setPlayerSprite(Player *player);
void initWalkSprite(Player *player, SDL_Renderer *renderer);
void initBikeSprite(Player *player, SDL_Renderer *renderer);
const char *getWalkAnimFromDir(Player *player, int dir);

#endif
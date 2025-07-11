#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>
#include "../systems/inputs.h"
#include "../framework/map.h"
#include "../systems/camera.h"

typedef enum
{
    WALK_MOD,
    RUN_MOD,
    BIKE_MOD
} PlayerMode;

typedef struct
{
    const char *idle;
    const char *walk;
} AnimationSet;

typedef struct
{
    AnimationSet walk_anims[4]; // [gauche, droite, haut, bas]
    AnimationSet bike_anims[4];
    AnimationSet run_anims[4]; // pour plus tard
} PlayerAnimations;

// Structure pour gérer les actions du joueur
typedef struct
{
    bool move_left;
    bool move_right;
    bool move_up;
    bool move_down;
    bool toggle_bike;
    bool toggle_run;
} PlayerActions;

typedef struct
{
    Entity entity;
    float speed;
    bool moving;
    SDL_RendererFlip flip;

    PlayerAnimations animations;
    int direction; // 0=gauche, 1=droite, 2=haut, 3=bas

    float targetX, targetY;
    bool hasTarget;
    bool wasMovingLastFrame;

    PlayerMode mode;

    Sprite *walkSprite;
    Sprite *runSprite;
    Sprite *bikeSprite;
    Sprite *currentSprite;

} Player;

Player *InitPlayer(float x, float y, SDL_Renderer *renderer);
void updatePlayer(Player *player, float deltaTime);
void updatePlayerWithInput(Player *player, Input *input, float deltaTime, Map *map);
void renderPlayer(Player *player, SDL_Renderer *renderer, Camera *camera); // Added Camera* parameter
void freePlayer(Player *player);
bool checkCollisionWithMap(Player *player, float newX, float newY, Map *map);
bool pointInPolygon(Point point, Point *polygon, int count);
bool rectangleIntersectsPolygon(Hitbox rect, Point *polygon, int count);
void setPlayerSprite(Player *player);
void processPlayerInput(Player *player, Input *input, float deltaTime, Map *map);
void updatePlayerMovement(Player *player, float deltaTime, Map *map);
void updatePlayerAnimation(Player *player);
PlayerActions inputToActions(Input *input);
void startMovement(Player *player, int direction, Map *map);

#endif
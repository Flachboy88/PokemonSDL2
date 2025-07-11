#ifndef PNJ_H
#define PNJ_H

#include "entity.h"
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>
#include "../systems/camera.h"

typedef struct
{
    const char *idle;
    const char *walk;
} PNJAnimationSet;

typedef struct
{
    PNJAnimationSet anims[4]; // [gauche, droite, haut, bas]
} PNJAnimations;

typedef struct
{
    Entity entity;
    float speed;
    bool moving;
    SDL_RendererFlip flip;

    PNJAnimations animations;
    int direction; // 0=gauche, 1=droite, 2=haut, 3=bas

    float targetX, targetY;
    bool hasTarget;

    float default_x_spawn;
    float default_y_spawn;
    int default_dir;
    bool aEteInit;

    Sprite *sprite;
} PNJ;

// Fonctions principales
PNJ *createPNJ(float x, float y, const char *spritePath, SDL_Renderer *renderer);
void updatePNJ(PNJ *pnj, float deltaTime);
void renderPNJ(PNJ *pnj, SDL_Renderer *renderer, Camera *camera);
void freePNJ(PNJ *pnj);

// Fonctions de déplacement
void moveLeft(PNJ *pnj, float distance);
void moveRight(PNJ *pnj, float distance);
void moveUp(PNJ *pnj, float distance);
void moveDown(PNJ *pnj, float distance);
void moveTo(PNJ *pnj, float x, float y);

// Fonctions d'animation
void addPNJAnimation(PNJ *pnj, const char *name, int startFrame, int endFrame, int frameTime, bool loop);
void playPNJAnimation(PNJ *pnj, const char *animName);
void setPNJDirection(PNJ *pnj, int direction);

// Fonctions internes
void updatePNJMovement(PNJ *pnj, float deltaTime);
void updatePNJAnimation(PNJ *pnj);

#endif
#include "pnj.h"
#include <stdlib.h>
#include <stdio.h>

PNJ *createPNJ(float x, float y, const char *spritePath, SDL_Renderer *renderer)
{
    PNJ *pnj = malloc(sizeof(PNJ));
    if (!pnj)
        return NULL;

    pnj->sprite = createSpriteWithColumns(spritePath, 4, 5, 25, 32, renderer);
    if (!pnj->sprite)
    {
        free(pnj);
        return NULL;
    }

    // Setup entity
    pnj->entity.x = x;
    pnj->entity.y = y;
    pnj->entity.width = pnj->sprite->frame_width;
    pnj->entity.height = pnj->sprite->frame_height;
    pnj->entity.sprite = pnj->sprite;
    pnj->entity.visible = true;
    pnj->entity.layer = 1;
    pnj->aEteInit = false;

    // Setup hitbox
    pnj->entity.hitbox.x = x;
    pnj->entity.hitbox.y = y;
    pnj->entity.hitbox.width = pnj->entity.width;
    pnj->entity.hitbox.height = pnj->entity.height;

    pnj->direction = 2; // bas par défaut
    pnj->speed = 30.0f;
    pnj->moving = false;
    pnj->hasTarget = false;
    pnj->flip = SDL_FLIP_NONE;

    pnj->animations.anims[0] = (PNJAnimationSet){"idle_left", "walk_left"};
    pnj->animations.anims[1] = (PNJAnimationSet){"idle_right", "walk_right"};
    pnj->animations.anims[2] = (PNJAnimationSet){"idle_up", "walk_up"};
    pnj->animations.anims[3] = (PNJAnimationSet){"idle_down", "walk_down"};

    addPNJAnimation(pnj, "idle_left", 4, 4, 0, false);
    addPNJAnimation(pnj, "idle_right", 8, 8, 0, false);
    addPNJAnimation(pnj, "idle_up", 12, 12, 0, false);
    addPNJAnimation(pnj, "idle_down", 0, 0, 0, false);

    addPNJAnimation(pnj, "walk_left", 4, 7, 150, true);
    addPNJAnimation(pnj, "walk_right", 8, 11, 150, true);
    addPNJAnimation(pnj, "walk_up", 12, 15, 150, true);
    addPNJAnimation(pnj, "walk_down", 0, 3, 150, true);

    return pnj;
}

void updatePNJ(PNJ *pnj, float deltaTime)
{
    if (!pnj)
        return;

    updatePNJMovement(pnj, deltaTime);
    updatePNJAnimation(pnj);
    updateEntity(&pnj->entity);
}

void renderPNJ(PNJ *pnj, SDL_Renderer *renderer, Camera *camera)
{
    if (pnj && pnj->entity.visible && pnj->entity.sprite)
    {
        SDL_Rect screen_rect = getScreenRect(camera, pnj->entity.x, pnj->entity.y, pnj->entity.width, pnj->entity.height);
        renderSprite(pnj->entity.sprite, renderer, screen_rect.x, screen_rect.y);
    }
    if (pnj)
    {
        // drawHitbox(&pnj->entity, renderer, camera);
    }
}

void freePNJ(PNJ *pnj)
{
    if (pnj)
    {
        if (pnj->sprite)
        {
            freeSprite(pnj->sprite);
            pnj->sprite = NULL;
        }
        pnj->entity.sprite = NULL;
        free(pnj);
    }
}

void moveLeft(PNJ *pnj, float distance)
{
    if (!pnj)
        return;
    pnj->targetX = pnj->entity.x - distance;
    pnj->targetY = pnj->entity.y;
    pnj->hasTarget = true;
    pnj->moving = true;
    pnj->direction = 0;
}

void moveRight(PNJ *pnj, float distance)
{
    if (!pnj)
        return;
    pnj->targetX = pnj->entity.x + distance;
    pnj->targetY = pnj->entity.y;
    pnj->hasTarget = true;
    pnj->moving = true;
    pnj->direction = 1;
}

void moveUp(PNJ *pnj, float distance)
{
    if (!pnj)
        return;
    pnj->targetX = pnj->entity.x;
    pnj->targetY = pnj->entity.y - distance;
    pnj->hasTarget = true;
    pnj->moving = true;
    pnj->direction = 2;
}

void moveDown(PNJ *pnj, float distance)
{
    if (!pnj)
        return;
    pnj->targetX = pnj->entity.x;
    pnj->targetY = pnj->entity.y + distance;
    pnj->hasTarget = true;
    pnj->moving = true;
    pnj->direction = 3;
}

void moveTo(PNJ *pnj, float x, float y)
{
    if (!pnj)
        return;
    pnj->targetX = x;
    pnj->targetY = y;
    pnj->hasTarget = true;
    pnj->moving = true;

    // Déterminer la direction principale du mouvement
    float dx = x - pnj->entity.x;
    float dy = y - pnj->entity.y;

    if (abs(dx) > abs(dy))
    {
        pnj->direction = (dx > 0) ? 1 : 0; // droite ou gauche
    }
    else
    {
        pnj->direction = (dy > 0) ? 3 : 2; // bas ou haut
    }
}

void addPNJAnimation(PNJ *pnj, const char *name, int startFrame, int endFrame, int frameTime, bool loop)
{
    if (!pnj || !pnj->sprite)
        return;
    addSimpleAnimation(pnj->sprite, name, startFrame, endFrame, frameTime, loop);
}

void playPNJAnimation(PNJ *pnj, const char *animName)
{
    if (!pnj || !pnj->sprite)
        return;
    playAnimation(pnj->sprite, animName);
}

void setPNJDirection(PNJ *pnj, int direction)
{
    if (!pnj || direction < 0 || direction > 3)
        return;
    pnj->direction = direction;
}

void updatePNJMovement(PNJ *pnj, float deltaTime)
{
    if (!pnj->hasTarget)
    {
        pnj->moving = false;
        return;
    }

    float dx = pnj->targetX - pnj->entity.x;
    float dy = pnj->targetY - pnj->entity.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 1.0f)
    {
        float moveDistance = pnj->speed * deltaTime;
        if (moveDistance > distance)
            moveDistance = distance;

        pnj->entity.x += (dx / distance) * moveDistance;
        pnj->entity.y += (dy / distance) * moveDistance;
    }
    else
    {
        pnj->entity.x = pnj->targetX;
        pnj->entity.y = pnj->targetY;
        pnj->hasTarget = false;
        pnj->moving = false;
    }

    // Mise à jour de la hitbox
    pnj->entity.hitbox.x = pnj->entity.x;
    pnj->entity.hitbox.y = pnj->entity.y;
}

void updatePNJAnimation(PNJ *pnj)
{
    if (!pnj)
        return;

    const char *animName = NULL;
    PNJAnimationSet *currentAnimSet = &pnj->animations.anims[pnj->direction];

    // Choisir idle ou walk selon si le PNJ bouge
    animName = pnj->moving ? currentAnimSet->walk : currentAnimSet->idle;

    playAnimation(pnj->sprite, animName);
}
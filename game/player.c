#include "player.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

Player *InitPlayer(float x, float y, SDL_Renderer *renderer)
{
    Player *player = malloc(sizeof(Player));

    Sprite *sprite = createSpriteWithColumns("resources/sprites/player.png", 4, 5, 25, 32, renderer); // 4 cols x 5 rows
    if (!sprite)
    {
        free(player);
        return NULL;
    }

    addSimpleAnimation(sprite, "defaut_bas", 0, 0, 0, false);
    addSimpleAnimation(sprite, "defaut_haut", 12, 12, 0, false);
    addSimpleAnimation(sprite, "defaut_droite", 8, 8, 0, false);
    addSimpleAnimation(sprite, "defaut_gauche", 4, 4, 0, false);
    addSimpleAnimation(sprite, "defaut_bas", 3, 3, 0, false);

    addSimpleAnimation(sprite, "walk_bas", 0, 3, 150, true);
    addSimpleAnimation(sprite, "walk_haut", 12, 15, 150, true);
    addSimpleAnimation(sprite, "walk_droite", 8, 11, 150, true);
    addSimpleAnimation(sprite, "walk_gauche", 4, 7, 150, true);

    player->entity.x = x;
    player->entity.y = y;
    player->entity.width = sprite->frame_width;
    player->entity.height = sprite->frame_height;
    player->entity.sprite = sprite;
    player->entity.visible = true;
    player->entity.layer = 1;
    player->currentAnimationName = "defaut_bas";
    player->lastDirection = 3;

    player->targetX = x;
    player->targetY = y;
    player->hasTarget = false;

    player->speed = 50.0f; // pixels par seconde
    player->moving = false;
    player->flip = SDL_FLIP_NONE;

    playAnimation(sprite, "defaut_bas");

    return player;
}

void updatePlayerWithInput(Player *player, Input *input, float deltaTime)
{
    updateEntity(&player->entity);

    const char *newAnimation = NULL;

    if (player->hasTarget)
    {
        // On ignore les inputs, on continue dans la direction actuelle
        deplacement(player, player->lastDirection, deltaTime);
        player->moving = true;
        newAnimation = walkAnimFromDir(player->lastDirection);
    }
    else
    {
        // On accepte les inputs uniquement si pas de cible
        if (input->left && !player->moving)
        {
            player->lastDirection = 0;
            player->moving = true;
            deplacement(player, 0, deltaTime);
            newAnimation = "walk_gauche";
        }
        else if (input->right && !player->moving)
        {
            player->lastDirection = 1;
            player->moving = true;
            deplacement(player, 1, deltaTime);
            newAnimation = "walk_droite";
        }
        else if (input->up && !player->moving)
        {
            player->lastDirection = 2;
            player->moving = true;
            deplacement(player, 2, deltaTime);
            newAnimation = "walk_haut";
        }
        else if (input->down && !player->moving)
        {
            player->lastDirection = 3;
            player->moving = true;
            deplacement(player, 3, deltaTime);
            newAnimation = "walk_bas";
        }
        else
        {
            player->moving = false;
            newAnimation = derniereDir(player);
        }
    }

    if (strcmp(player->currentAnimationName, newAnimation) != 0)
    {
        playAnimation(player->entity.sprite, newAnimation);
        player->currentAnimationName = newAnimation;
    }
}

void renderPlayer(Player *player, SDL_Renderer *renderer)
{
    if (player->entity.visible && player->entity.sprite)
    {
        renderSpriteFlipped(player->entity.sprite, renderer,
                            (int)player->entity.x, (int)player->entity.y,
                            player->flip);
    }
}

void freePlayer(Player *player)
{
    if (player->entity.sprite)
    {
        freeSprite(player->entity.sprite);
        player->entity.sprite = NULL; // Éviter double free
    }
    free(player);
}

char *derniereDir(Player *player)
{
    if (player->lastDirection == 0)
        return "defaut_gauche";
    else if (player->lastDirection == 1)
        return "defaut_droite";
    else if (player->lastDirection == 2)
        return "defaut_haut";
    else
        return "defaut_bas";
}

void deplacement(Player *player, int dir, float deltaTime)
{
    // Si pas de cible, définir la prochaine cible (multiple de n)
    int n = 16;
    if (!player->hasTarget)
    {
        switch (dir)
        {
        case 0: // gauche
            player->targetX = ((int)player->entity.x / n) * n - n;
            player->targetY = player->entity.y;
            break;
        case 1: // droite
            player->targetX = ((int)player->entity.x / n) * n + n;
            player->targetY = player->entity.y;
            break;
        case 2: // haut
            player->targetX = player->entity.x;
            player->targetY = ((int)player->entity.y / n) * n - n;
            break;
        case 3: // bas
            player->targetX = player->entity.x;
            player->targetY = ((int)player->entity.y / n) * n + n;
            break;
        }
        player->hasTarget = true;
    }

    // Se déplacer vers la cible
    float dx = player->targetX - player->entity.x;
    float dy = player->targetY - player->entity.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 1.0f)
    { // Pas encore arrivé
        float moveDistance = player->speed * deltaTime;
        if (moveDistance > distance)
            moveDistance = distance; // ne pas dépasser la cible

        player->entity.x += (dx / distance) * moveDistance;
        player->entity.y += (dy / distance) * moveDistance;
    }
    else
    {
        // Arrivé à la cible
        player->entity.x = player->targetX;
        player->entity.y = player->targetY;
        player->hasTarget = false;
    }
}

const char *walkAnimFromDir(int dir)
{
    switch (dir)
    {
    case 0:
        return "walk_gauche";
    case 1:
        return "walk_droite";
    case 2:
        return "walk_haut";
    default:
        return "walk_bas";
    }
}

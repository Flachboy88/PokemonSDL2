#include "player.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

const int LARGEUR_HITBOX = 10;
const int HAUTEUR_HITBOX = 15;

void initWalkSprite(Player *player, SDL_Renderer *renderer)
{
    player->walkSprite = createSpriteWithColumns("resources/sprites/player.png", 4, 5, 25, 32, renderer); // 4 cols x 5 rows
    if (!player->walkSprite)
    {
        free(player);
        return NULL;
    }

    addSimpleAnimation(player->walkSprite, "defaut_bas", 0, 0, 0, false);
    addSimpleAnimation(player->walkSprite, "defaut_haut", 12, 12, 0, false);
    addSimpleAnimation(player->walkSprite, "defaut_droite", 8, 8, 0, false);
    addSimpleAnimation(player->walkSprite, "defaut_gauche", 4, 4, 0, false);
    addSimpleAnimation(player->walkSprite, "defaut_bas", 3, 3, 0, false);

    addSimpleAnimation(player->walkSprite, "walk_bas", 0, 3, 150, true);
    addSimpleAnimation(player->walkSprite, "walk_haut", 12, 15, 150, true);
    addSimpleAnimation(player->walkSprite, "walk_droite", 8, 11, 150, true);
    addSimpleAnimation(player->walkSprite, "walk_gauche", 4, 7, 150, true);
}

void initBikeSprite(Player *player, SDL_Renderer *renderer)
{
    player->bikeSprite = createSpriteWithColumns("resources/sprites/player_bike.png", 4, 5, 25, 32, renderer); // 4 cols x 5 rows
    if (!player->bikeSprite)
    {
        free(player);
        return;
    }

    addSimpleAnimation(player->bikeSprite, "bike_bas", 0, 0, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_haut", 12, 12, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_droite", 8, 8, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_gauche", 4, 4, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_bas", 3, 3, 0, false);

    addSimpleAnimation(player->bikeSprite, "bike_walk_bas", 0, 3, 150, true);
    addSimpleAnimation(player->bikeSprite, "bike_walk_haut", 12, 15, 150, true);
    addSimpleAnimation(player->bikeSprite, "bike_walk_droite", 8, 11, 150, true);
    addSimpleAnimation(player->bikeSprite, "bike_walk_gauche", 4, 7, 150, true);
}

Player *InitPlayer(float x, float y, SDL_Renderer *renderer)
{
    Player *player = malloc(sizeof(Player));

    initWalkSprite(player, renderer);
    initBikeSprite(player, renderer);

    player->currentSprite = player->walkSprite;
    player->entity.x = x;
    player->entity.y = y;
    player->entity.width = player->walkSprite->frame_width;
    player->entity.height = player->walkSprite->frame_height;
    player->entity.sprite = player->currentSprite;
    player->entity.visible = true;
    player->entity.layer = 1;

    player->entity.hitbox.x = x + player->walkSprite->frame_width / 2 - LARGEUR_HITBOX / 2;
    player->entity.hitbox.y = y + player->walkSprite->frame_height - HAUTEUR_HITBOX;
    player->entity.hitbox.width = LARGEUR_HITBOX;
    player->entity.hitbox.height = HAUTEUR_HITBOX;

    player->currentAnimationName = "defaut_bas";
    player->lastDirection = 3;

    player->targetX = x;
    player->targetY = y;
    player->hasTarget = false;

    player->mode = WALK_MOD;
    setPlayerSpeedMode(player);
    player->moving = false;

    player->flip = SDL_FLIP_NONE;

    playAnimation(player->currentSprite, "defaut_bas");

    return player;
}

void updatePlayerWithInput(Player *player, Input *input, float deltaTime, Map *map)
{
    updateEntity(&player->entity);

    player->entity.hitbox.x = player->entity.x + player->entity.sprite->frame_width / 2 - LARGEUR_HITBOX / 2;
    player->entity.hitbox.y = player->entity.y + player->entity.sprite->frame_height - HAUTEUR_HITBOX;

    const char *newAnimation = NULL;

    if (input->left && !player->moving)
    {
        player->lastDirection = 0;
        player->moving = true;
        deplacement(player, 0, deltaTime, map);
        newAnimation = "walk_gauche";
    }
    else if (input->right && !player->moving)
    {
        player->lastDirection = 1;
        player->moving = true;
        deplacement(player, 1, deltaTime, map);
        newAnimation = "walk_droite";
    }
    else if (input->up && !player->moving)
    {
        player->lastDirection = 2;
        player->moving = true;
        deplacement(player, 2, deltaTime, map);
        newAnimation = "walk_haut";
    }
    else if (input->down && !player->moving)
    {
        player->lastDirection = 3;
        player->moving = true;
        deplacement(player, 3, deltaTime, map);
        newAnimation = "walk_bas";
    }
    else if (input->space)
    {
        if (player->mode == WALK_MOD)
        {
            setPlayerMode(player, BIKE_MOD);
        }
        else if (player->mode == BIKE_MOD)
        {
            setPlayerMode(player, WALK_MOD);
        }
        newAnimation = derniereDir(player);
    }
    else if (input->r_key)
    {
        if (player->mode == WALK_MOD)
        {
            setPlayerMode(player, RUN_MOD);
        }
        else if (player->mode == RUN_MOD)
        {
            setPlayerMode(player, WALK_MOD);
        }
        newAnimation = derniereDir(player);
    }
    else if (player->hasTarget)
    {
        // Continue le mouvement en cours seulement si on a une cible
        deplacement(player, player->lastDirection, deltaTime, map);
        player->moving = true;
        newAnimation = getWalkAnimFromDir(player, player->lastDirection);
    }
    else
    {
        player->moving = false;
        newAnimation = derniereDir(player);
    }

    if (strcmp(player->currentAnimationName, newAnimation) != 0)
    {
        playAnimation(player->entity.sprite, newAnimation);
        player->currentAnimationName = newAnimation;
    }

    // printf("Player: x=%.2f, y=%.2f, dir=%d\n", player->entity.x, player->entity.y, player->lastDirection);
}

void renderPlayer(Player *player, SDL_Renderer *renderer)
{
    if (player->entity.visible && player->entity.sprite)
    {
        renderSpriteFlipped(player->entity.sprite, renderer,
                            (int)player->entity.x, (int)player->entity.y,
                            player->flip);
    }
    drawHitbox(&player->entity, renderer);
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
    const char *prefix = (player->mode == BIKE_MOD) ? "bike_" : "defaut_";

    if (player->lastDirection == 0)
        return (player->mode == BIKE_MOD) ? "bike_gauche" : "defaut_gauche";
    else if (player->lastDirection == 1)
        return (player->mode == BIKE_MOD) ? "bike_droite" : "defaut_droite";
    else if (player->lastDirection == 2)
        return (player->mode == BIKE_MOD) ? "bike_haut" : "defaut_haut";
    else
        return (player->mode == BIKE_MOD) ? "bike_bas" : "defaut_bas";
}

void deplacement(Player *player, int dir, float deltaTime, Map *map)
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

        float newX = player->entity.x + (dx / distance) * moveDistance;
        float newY = player->entity.y + (dy / distance) * moveDistance;

        // Vérifier les collisions avec la nouvelle position
        if (!checkCollisionWithMap(player, newX, newY, map))
        {
            // Pas de collision, on peut se déplacer
            player->entity.x = newX;
            player->entity.y = newY;
        }
        else
        {
            // Collision détectée, on annule le mouvement et on supprime la cible
            player->hasTarget = false;
            player->moving = false;
        }
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

bool pointInPolygon(Point point, Point *polygon, int count)
{
    bool inside = false;
    int j = count - 1;

    for (int i = 0; i < count; i++)
    {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x))
        {
            inside = !inside;
        }
        j = i;
    }
    return inside;
}

bool rectangleIntersectsPolygon(Hitbox rect, Point *polygon, int count)
{
    // Vérifier si un des coins du rectangle est dans le polygone
    Point corners[4] = {
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height}};

    for (int i = 0; i < 4; i++)
    {
        if (pointInPolygon(corners[i], polygon, count))
        {
            return true;
        }
    }

    // Vérifier si un des points du polygone est dans le rectangle
    for (int i = 0; i < count; i++)
    {
        if (polygon[i].x >= rect.x && polygon[i].x <= rect.x + rect.width &&
            polygon[i].y >= rect.y && polygon[i].y <= rect.y + rect.height)
        {
            return true;
        }
    }

    return false;
}

bool checkCollisionWithMap(Player *player, float newX, float newY, Map *map)
{
    // Créer une hitbox temporaire avec la nouvelle position
    Hitbox tempHitbox;
    tempHitbox.x = newX + player->entity.sprite->frame_width / 2 - LARGEUR_HITBOX / 2;
    tempHitbox.y = newY + player->entity.sprite->frame_height - HAUTEUR_HITBOX;
    tempHitbox.width = LARGEUR_HITBOX;
    tempHitbox.height = HAUTEUR_HITBOX;

    // Vérifier les collisions avec tous les objets de collision de la map
    for (int i = 0; i < map->collision_count; i++)
    {
        if (map->collisions[i].is_polygon)
        {
            // Collision avec un polygone
            if (rectangleIntersectsPolygon(tempHitbox, map->collisions[i].polygon_points, map->collisions[i].polygon_count))
            {
                return true;
            }
        }
        else
        {
            // Collision avec un rectangle (code existant)
            SDL_Rect collisionRect = map->collisions[i].rect;

            if (tempHitbox.x < collisionRect.x + collisionRect.w &&
                tempHitbox.x + tempHitbox.width > collisionRect.x &&
                tempHitbox.y < collisionRect.y + collisionRect.h &&
                tempHitbox.y + tempHitbox.height > collisionRect.y)
            {
                return true;
            }
        }
    }
    return false;
}

void setPlayerMode(Player *player, PlayerMode mode)
{
    if (!player)
        return;

    player->mode = mode;
    setPlayerSpeedMode(player);
    setPlayerSprite(player);

    player->currentAnimationName = derniereDir(player);
    playAnimation(player->currentSprite, player->currentAnimationName);
}

void setPlayerSpeedMode(Player *player)
{
    if (!player)
        return;

    switch (player->mode)
    {
    case WALK_MOD:
        player->speed = 50.0f; // pixels par seconde
        break;
    case RUN_MOD:
        player->speed = 100.0f; // pixels par seconde
        break;
    case BIKE_MOD:
        player->speed = 200.0f; // pixels par seconde
        break;
    }
}

void setPlayerSprite(Player *player)
{
    switch (player->mode)
    {
    case WALK_MOD:
        player->currentSprite = player->walkSprite;
        break;
    case RUN_MOD:
        player->currentSprite = player->runSprite;
        break;
    case BIKE_MOD:
        player->currentSprite = player->bikeSprite;
        break;
    }
    player->entity.sprite = player->currentSprite;
}

const char *getWalkAnimFromDir(Player *player, int dir)
{
    if (player->mode == BIKE_MOD)
    {
        switch (dir)
        {
        case 0:
            return "bike_walk_gauche";
        case 1:
            return "bike_walk_droite";
        case 2:
            return "bike_walk_haut";
        default:
            return "bike_walk_bas";
        }
    }
    else
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
}
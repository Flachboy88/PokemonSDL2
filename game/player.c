#include "player.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

const int LARGEUR_HITBOX = 10;
const int HAUTEUR_HITBOX = 15;

void initPlayerAnimations(Player *player, SDL_Renderer *renderer)
{
    // Init walk sprite
    player->walkSprite = createSpriteWithColumns("resources/sprites/player.png", 4, 5, 25, 32, renderer);

    // Walk animations
    addSimpleAnimation(player->walkSprite, "idle_left", 4, 4, 0, false);
    addSimpleAnimation(player->walkSprite, "idle_right", 8, 8, 0, false);
    addSimpleAnimation(player->walkSprite, "idle_up", 12, 12, 0, false);
    addSimpleAnimation(player->walkSprite, "idle_down", 0, 0, 0, false);

    addSimpleAnimation(player->walkSprite, "walk_left", 4, 7, 150, true);
    addSimpleAnimation(player->walkSprite, "walk_right", 8, 11, 150, true);
    addSimpleAnimation(player->walkSprite, "walk_up", 12, 15, 150, true);
    addSimpleAnimation(player->walkSprite, "walk_down", 0, 3, 150, true);

    // Init bike sprite
    player->bikeSprite = createSpriteWithColumns("resources/sprites/player_bike.png", 4, 5, 25, 32, renderer);

    // Bike animations
    addSimpleAnimation(player->bikeSprite, "bike_idle_left", 4, 4, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_idle_right", 8, 8, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_idle_up", 12, 12, 0, false);
    addSimpleAnimation(player->bikeSprite, "bike_idle_down", 0, 0, 0, false);

    addSimpleAnimation(player->bikeSprite, "bike_walk_left", 4, 7, 150, true);
    addSimpleAnimation(player->bikeSprite, "bike_walk_right", 8, 11, 150, true);
    addSimpleAnimation(player->bikeSprite, "bike_walk_up", 12, 15, 150, true);
    addSimpleAnimation(player->bikeSprite, "bike_walk_down", 0, 3, 150, true);

    // Setup animation mapping
    player->animations.walk_anims[0] = (AnimationSet){"idle_left", "walk_left"};
    player->animations.walk_anims[1] = (AnimationSet){"idle_right", "walk_right"};
    player->animations.walk_anims[2] = (AnimationSet){"idle_up", "walk_up"};
    player->animations.walk_anims[3] = (AnimationSet){"idle_down", "walk_down"};

    player->animations.bike_anims[0] = (AnimationSet){"bike_idle_left", "bike_walk_left"};
    player->animations.bike_anims[1] = (AnimationSet){"bike_idle_right", "bike_walk_right"};
    player->animations.bike_anims[2] = (AnimationSet){"bike_idle_up", "bike_walk_up"};
    player->animations.bike_anims[3] = (AnimationSet){"bike_idle_down", "bike_walk_down"};
}

Player *InitPlayer(float x, float y, SDL_Renderer *renderer)
{
    Player *player = malloc(sizeof(Player));

    initPlayerAnimations(player, renderer);

    // Setup entity
    player->entity.x = x;
    player->entity.y = y;
    player->entity.width = player->walkSprite->frame_width;
    player->entity.height = player->walkSprite->frame_height;
    player->entity.visible = true;
    player->entity.layer = 1;

    // Setup hitbox
    player->entity.hitbox.x = x + player->walkSprite->frame_width / 2 - LARGEUR_HITBOX / 2;
    player->entity.hitbox.y = y + player->walkSprite->frame_height - HAUTEUR_HITBOX;
    player->entity.hitbox.width = LARGEUR_HITBOX;
    player->entity.hitbox.height = HAUTEUR_HITBOX;

    // Setup player state
    player->direction = 3; // bas par défaut
    player->mode = WALK_MOD;
    player->currentSprite = player->walkSprite;
    player->entity.sprite = player->currentSprite;
    player->speed = 50.0f;
    player->moving = false;
    player->hasTarget = false;
    player->flip = SDL_FLIP_NONE;
    player->wasMovingLastFrame = false;

    updatePlayerAnimation(player);

    return player;
}

void startMovement(Player *player, int direction, Map *map)
{
    int n = 16;
    float newTargetX = player->entity.x;
    float newTargetY = player->entity.y;

    switch (direction)
    {
    case 0:
        newTargetX = ((int)player->entity.x / n) * n - n;
        break;
    case 1:
        newTargetX = ((int)player->entity.x / n) * n + n;
        break;
    case 2:
        newTargetY = ((int)player->entity.y / n) * n - n;
        break;
    case 3:
        newTargetY = ((int)player->entity.y / n) * n + n;
        break;
    }

    // Vérifier si on peut se déplacer vers cette position
    if (!checkCollisionWithMap(player, newTargetX, newTargetY, map))
    {
        player->targetX = newTargetX;
        player->targetY = newTargetY;
        player->hasTarget = true;
        player->moving = true;
    }
}

void processPlayerInput(Player *player, Input *input, float deltaTime, Map *map)
{
    PlayerActions actions = inputToActions(input);

    // Gestion des changements de mode
    if (actions.toggle_bike)
    {
        player->mode = (player->mode == BIKE_MOD) ? WALK_MOD : BIKE_MOD;
        setPlayerSprite(player);
    }

    if (actions.toggle_run)
    {
        player->mode = (player->mode == RUN_MOD) ? WALK_MOD : RUN_MOD;
        setPlayerSprite(player);
    }

    // Vérifier si on a des inputs de mouvement
    bool hasMovementInput = actions.move_left || actions.move_right || actions.move_up || actions.move_down;

    if (hasMovementInput)
    {
        // Mouvement libre tant que les touches sont pressées
        float newX = player->entity.x;
        float newY = player->entity.y;
        float moveDistance = player->speed * deltaTime;

        // Prioriser une seule direction à la fois (horizontal d'abord)
        if (actions.move_left)
        {
            player->direction = 0;
            newX -= moveDistance;
        }
        else if (actions.move_right)
        {
            player->direction = 1;
            newX += moveDistance;
        }
        else if (actions.move_up)
        {
            player->direction = 2;
            newY -= moveDistance;
        }
        else if (actions.move_down)
        {
            player->direction = 3;
            newY += moveDistance;
        }

        // Vérifier les collisions séparément pour X et Y
        float tempX = player->entity.x;
        float tempY = player->entity.y;

        // Tester le mouvement horizontal
        if (actions.move_left || actions.move_right)
        {
            if (!checkCollisionWithMap(player, newX, tempY, map))
            {
                tempX = newX;
            }
        }

        // Tester le mouvement vertical
        if (actions.move_up || actions.move_down)
        {
            if (!checkCollisionWithMap(player, tempX, newY, map))
            {
                tempY = newY;
            }
        }

        // Appliquer les nouvelles positions
        player->entity.x = tempX;
        player->entity.y = tempY;

        // TOUJOURS mettre à jour la hitbsox après avoir bougé (ou pas)
        player->entity.hitbox.x = player->entity.x + player->entity.sprite->frame_width / 2 - LARGEUR_HITBOX / 2;
        player->entity.hitbox.y = player->entity.y + player->entity.sprite->frame_height - HAUTEUR_HITBOX;

        player->moving = true;
        player->hasTarget = false; // Annuler tout target existant
    }
    else if (player->wasMovingLastFrame)
    {
        // On vient de relâcher les touches, aligner sur la grille dans la direction du mouvement
        int n = 16;
        float alignedX = player->entity.x;
        float alignedY = player->entity.y;

        // Calculer la prochaine position alignée selon la direction
        switch (player->direction)
        {
        case 0: // gauche
            alignedX = floor(player->entity.x / n) * n;
            break;
        case 1: // droite
            alignedX = ceil(player->entity.x / n) * n;
            break;
        case 2: // haut
            alignedY = floor(player->entity.y / n) * n;
            break;
        case 3: // bas
            alignedY = ceil(player->entity.y / n) * n;
            break;
        }

        // VÉRIFIER QUE LA TARGET N'EST PAS EN COLLISION
        if (!checkCollisionWithMap(player, alignedX, alignedY, map))
        {
            player->targetX = alignedX;
            player->targetY = alignedY;
            player->hasTarget = true;
        }
        else
        {
            // Si la target serait en collision, on reste où on est
            player->hasTarget = false;
            player->moving = false;
        }
    }

    // Mettre à jour l'état précédent
    player->wasMovingLastFrame = hasMovementInput;

    // Mise à jour du mouvement vers la target (si on en a une)
    updatePlayerMovement(player, deltaTime, map);
    updatePlayerAnimation(player);
    updateEntity(&player->entity);
}

PlayerActions inputToActions(Input *input)
{
    PlayerActions actions = {0};
    actions.move_left = input->left;
    actions.move_right = input->right;
    actions.move_up = input->up;
    actions.move_down = input->down;
    actions.toggle_bike = input->space;
    actions.toggle_run = input->r_key;
    return actions;
}

void updatePlayerMovement(Player *player, float deltaTime, Map *map)
{
    if (!player->hasTarget)
    {
        // Si on n'a pas de target et qu'on ne bouge pas, on n'est pas en mouvement
        if (!player->wasMovingLastFrame)
        {
            player->moving = false;
        }
        return;
    }

    float dx = player->targetX - player->entity.x;
    float dy = player->targetY - player->entity.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance > 1.0f)
    {
        float moveDistance = player->speed * deltaTime;
        if (moveDistance > distance)
            moveDistance = distance;

        float newX = player->entity.x + (dx / distance) * moveDistance;
        float newY = player->entity.y + (dy / distance) * moveDistance;

        // VÉRIFIER LA COLLISION AVANT DE BOUGER VERS LA TARGET
        if (!checkCollisionWithMap(player, newX, newY, map))
        {
            player->entity.x = newX;
            player->entity.y = newY;
        }
        else
        {
            // Si on ne peut pas bouger vers la target, l'annuler
            player->hasTarget = false;
            player->moving = false;
        }
    }
    else
    {
        player->entity.x = player->targetX;
        player->entity.y = player->targetY;
        player->hasTarget = false;
        player->moving = false;
    }

    // Mise à jour de la hitbox
    player->entity.hitbox.x = player->entity.x + player->entity.sprite->frame_width / 2 - LARGEUR_HITBOX / 2;
    player->entity.hitbox.y = player->entity.y + player->entity.sprite->frame_height - HAUTEUR_HITBOX;
}

void updatePlayerAnimation(Player *player)
{
    const char *animName = NULL;
    AnimationSet *currentAnimSet = NULL;

    // Choisir le bon set d'animations selon le mode
    switch (player->mode)
    {
    case WALK_MOD:
        currentAnimSet = &player->animations.walk_anims[player->direction];
        break;
    case BIKE_MOD:
        currentAnimSet = &player->animations.bike_anims[player->direction];
        break;
    case RUN_MOD:
        currentAnimSet = &player->animations.run_anims[player->direction];
        break;
    }

    // Choisir idle ou walk
    animName = player->moving ? currentAnimSet->walk : currentAnimSet->idle;

    playAnimation(player->currentSprite, animName);
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

    if (player->direction == 0)
        return (player->mode == BIKE_MOD) ? "bike_gauche" : "defaut_gauche";
    else if (player->direction == 1)
        return (player->mode == BIKE_MOD) ? "bike_droite" : "defaut_droite";
    else if (player->direction == 2)
        return (player->mode == BIKE_MOD) ? "bike_haut" : "defaut_haut";
    else
        return (player->mode == BIKE_MOD) ? "bike_bas" : "defaut_bas";
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

void setPlayerSprite(Player *player)
{
    switch (player->mode)
    {
    case WALK_MOD:
        player->currentSprite = player->walkSprite;
        player->speed = 50.0f;
        break;
    case RUN_MOD:
        player->currentSprite = player->runSprite;
        player->speed = 60.0f;
        break;
    case BIKE_MOD:
        player->currentSprite = player->bikeSprite;
        player->speed = 200.0f;
        break;
    }
    player->entity.sprite = player->currentSprite;
}

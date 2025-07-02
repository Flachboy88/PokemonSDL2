#include "player.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

Player* createPlayer(float x, float y, SDL_Renderer *renderer) {
    Player *player = malloc(sizeof(Player));
    
    // Charger le sprite du player
    Sprite *sprite = createSpriteWithColumns("resources/sprites/player.png", 4, 5,25,32,renderer); // Exemple: 4 colsx5rows grid
    if (!sprite) {
        free(player);
        return NULL;
    }
    
    addSimpleAnimation(sprite, "idle", 0, 3, 200, true);      // Frames 0-3, 200ms par frame, en boucle
    addSimpleAnimation(sprite, "walk", 4, 7, 150, true);      // Frames 4-7, 150ms par frame, en boucle
    addSimpleAnimation(sprite, "attack", 8, 11, 100, false);  // Frames 8-11, 100ms par frame, pas en boucle
    
    // Initialiser l'entité
    player->entity.x = x;
    player->entity.y = y;
    player->entity.width = sprite->frame_width;
    player->entity.height = sprite->frame_height;
    player->entity.sprite = sprite;
    player->entity.visible = true;
    player->entity.layer = 1; 
    
    // Initialiser les propriétés du player
    player->speed = 100.0f; // pixels par seconde
    player->health = 100;
    player->moving = false;
    player->flip = SDL_FLIP_NONE;
    
    // Commencer avec l'animation idle
    playAnimation(sprite, "idle");
    
    return player;
}

void updatePlayer(Player *player, float deltaTime) {
    updateEntity(&player->entity);
    
    // Logique de mouvement
}

void renderPlayer(Player *player, SDL_Renderer *renderer) {
    if (player->entity.visible && player->entity.sprite) {
        renderSpriteFlipped(player->entity.sprite, renderer, 
                          (int)player->entity.x, (int)player->entity.y, 
                          player->flip);
    }
}

void freePlayer(Player *player) {
    if (player->entity.sprite) {
        freeSprite(player->entity.sprite);
        player->entity.sprite = NULL;  // Éviter double free
    }
    free(player);
}
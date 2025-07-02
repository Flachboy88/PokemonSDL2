#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "framework/map.h"
#include "framework/sprite.h"

typedef struct {
    float x, y;             // Position
    float width, height;    // Taille
    Sprite *sprite;         // Sprite de l'entité
    bool visible;           // Visibilité
    int layer;              // Layer de rendu (pour tri)
} Entity;

typedef struct {
    Entity entity;          // Hérite d'Entity
    float speed;            // Vitesse de déplacement
    int health;             // Points de vie
    bool moving;            // En mouvement
    SDL_RendererFlip flip;  // Direction (flip horizontal)
} Player;

Entity* createEntity(float x, float y, float width, float height, Sprite *sprite, int layer) {
    Entity *entity = malloc(sizeof(Entity));
    entity->x = x;
    entity->y = y;
    entity->width = width;
    entity->height = height;
    entity->sprite = sprite;
    entity->visible = true;
    entity->layer = layer;
    return entity;
}

void updateEntity(Entity *entity) {
    if (entity->sprite) {
        updateSprite(entity->sprite);
    }
}

void renderEntity(Entity *entity, SDL_Renderer *renderer) {
    if (entity->visible && entity->sprite) {
        renderSprite(entity->sprite, renderer, (int)entity->x, (int)entity->y);
    }
}

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
    player->entity.layer = 1; // Au-dessus du background
    
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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Jeu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        fprintf(stderr, "Erreur d'initialisation SDL_image : %s\n", IMG_GetError());
        // gérer l'erreur...
    }

    Map *map = loadMap("resources/maps/map1.tmj", renderer);
    if (!map)
    {
        fprintf(stderr, "Erreur chargement map\n");
        return 1;
    }

    printf("map chargee\n");

    Player *player = createPlayer(250, 100, renderer);
    if (!player) {
        fprintf(stderr, "Erreur création player\n");
        freeMap(map);
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running)
    {
        // Handle input
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            // handleEvents(event); ← à faire plus tard
        }

        // update(); ← futur update logique

        // rendering
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        renderMapBeforeLayer(map, renderer, "Entities");

        // Update et render le player
        updatePlayer(player, 16.0f / 1000.0f);
        renderPlayer(player, renderer);

        // Render les layers au-dessus du player
        renderMapAfterLayer(map, renderer, "Entities");
        
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60fps
    }

    freeMap(map);
    freePlayer(player);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

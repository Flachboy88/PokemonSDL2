#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "framework/map.h"
#include "framework/sprite.h"
#include "game/entity.h"
#include "game/player.h"
#include "systems/utils.h"
#include "systems/inputs.h"

Map *loadAndInitMap(const char *name, SDL_Renderer *renderer)
{
    char *full_path = buildFilePath("resources/maps/", name, ".tmx");
    if (!full_path)
        return NULL;

    Map *map = loadMap(full_path, renderer);
    if (!map)
    {
        fprintf(stderr, "Erreur chargement map '%s'\n", full_path);
        free(full_path);
        return NULL;
    }

    float spawn_x, spawn_y;
    if (Map_getPlayerSpawn(map, &spawn_x, &spawn_y))
    {
        map->default_x_spawn = spawn_x;
        map->default_y_spawn = spawn_y;
        // printf("Map chargée : spawn en (%.2f, %.2f)\n", map->default_x_spawn, map->default_y_spawn);
    }
    else
    {
        printf("Map chargée, mais aucun point de spawn du joueur trouvé ('PlayerSpawn' dans le groupe 'PlayerObject'). Utilisation de (0,0).\n");
    }

    Map_initAnimations(map);

    free(full_path);
    return map;
}

bool handleEvents(SDL_Event *event, Input *input)
{
    while (SDL_PollEvent(event))
    {
        if (event->type == SDL_QUIT)
        {
            return false;
        }
    }

    // Gestion des touches en temps réel
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    input->left = keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A];
    input->right = keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D];
    input->up = keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W];
    input->down = keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_S];

    return true;
}

void updateData(Player *player, Input *input, float deltaTime)
{
    updatePlayerWithInput(player, input, deltaTime);
}

void updateGraphics(SDL_Renderer *renderer, Map *map, Player *player, Uint32 currentTime)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Afficher tous les calques du groupe Background et mettre à jour leurs animations
    Map_afficherGroup(renderer, map, "Background", 0, 0, currentTime);

    // Afficher tous les calques du groupe PremierPlan et mettre à jour leurs animations
    Map_afficherGroup(renderer, map, "PremierPlan", 0, 0, currentTime);

    // Afficher le joueur
    renderPlayer(player, renderer);

    // Afficher tous les calques du groupe SecondPlan et mettre à jour leurs animations
    Map_afficherGroup(renderer, map, "SecondPlan", 0, 0, currentTime);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!InitSDL(&window, &renderer, 800, 600))
    {
        fprintf(stderr, "Erreur InitSDL\n");
        return 1;
    }

    Map *map = loadAndInitMap("map3", renderer);
    if (!map)
    {
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player *player = InitPlayer(map->default_x_spawn, map->default_y_spawn, renderer);
    if (!player)
    {
        fprintf(stderr, "Erreur création player\n");
        freeMap(map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;
    Input input = {false, false, false, false};
    Uint32 lastTime = SDL_GetTicks();

    while (running)
    {
        // Calcul du deltaTime
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Gestion des événements
        running = handleEvents(&event, &input);

        // Mise à jour logique
        updateData(player, &input, deltaTime);

        // Rendu graphique (passe currentTime)
        updateGraphics(renderer, map, player, currentTime);

        // SDL_Delay(16); // ~60fps
    }

    freeMap(map);
    freePlayer(player);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
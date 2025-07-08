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
#include "game/pnj.h"
#include "systems/camera.h"

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
    input->space = false;
    input->r_key = false;

    while (SDL_PollEvent(event))
    {
        if (event->type == SDL_QUIT)
        {
            return false;
        }

        if (event->type == SDL_KEYDOWN)
        {
            if (event->key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                input->space = true;
            }
            else if (event->key.keysym.scancode == SDL_SCANCODE_R)
            {
                input->r_key = true;
            }
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

void scriptPnj(PNJ *pnj, float deltaTime)
{
    if (pnj)
    {
        updatePNJ(pnj, deltaTime);
        if (pnj->entity.x < 500)
        {
            moveRight(pnj, 10);
        }
        else
        {
            moveUp(pnj, 10);
        }
    }
}

// Pass camera to updateData
void updateData(Player *player, PNJ *pnj, Input *input, float deltaTime, Map *map, Camera *camera)
{
    processPlayerInput(player, input, deltaTime, map);
    updatePNJ(pnj, deltaTime);
    updateCamera(camera, player->entity.x, player->entity.y);

    for (int i = 0; i < map->pnj_count; i++)
    {
        updatePNJ(map->pnjs[i], deltaTime);
    }
}

// Pass camera to updateGraphics
void updateGraphics(SDL_Renderer *renderer, Map *map, Player *player, PNJ *pnj, Uint32 currentTime, Camera *camera)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    Map_renderGroup(renderer, map, "Background", -camera->view_rect.x, -camera->view_rect.y, currentTime);
    Map_renderGroup(renderer, map, "PremierPlan", -camera->view_rect.x, -camera->view_rect.y, currentTime);

    renderPlayer(player, renderer, camera);
    renderPNJ(pnj, renderer, camera);
    Map_renderPNJs(renderer, map, camera);

    Map_renderGroup(renderer, map, "SecondPlan", -camera->view_rect.x, -camera->view_rect.y, currentTime);
    Map_drawCollisionsInCamera(renderer, map, camera);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    int window_width = 350;
    int window_height = 350;

    if (!InitSDL(&window, &renderer, window_width, window_height))
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

    // Initialize the camera
    Camera *camera = initCamera(0, 0, window_width, window_height, map->tmx_map->width * map->tmx_map->tile_width, map->tmx_map->height * map->tmx_map->tile_height);
    if (!camera)
    {
        fprintf(stderr, "Erreur création camera\n");
        freeMap(map);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    Player *player = InitPlayer(map->default_x_spawn - 12, map->default_y_spawn - 32, renderer); // revoir calcul
    if (!player)
    {
        fprintf(stderr, "Erreur création player\n");
        freeMap(map);
        freeCamera(camera); // Free camera before exiting
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // test pnj
    PNJ *testPNJ = createPNJ(100, 100, "resources/sprites/pnj.png", renderer);
    if (!testPNJ)
    {
        fprintf(stderr, "Erreur création PNJ\n");
    }

    bool running = true;
    SDL_Event event;
    Input input = {false, false, false, false, false, false};
    Uint32 lastTime = SDL_GetTicks();

    while (running)
    {
        // Calcul du deltaTime
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Gestion des événements
        running = handleEvents(&event, &input);

        // Mise à jour logique (pass camera)
        updateData(player, testPNJ, &input, deltaTime, map, camera);

        // Rendu graphique (passe currentTime et camera)
        updateGraphics(renderer, map, player, testPNJ, currentTime, camera);

        // SDL_Delay(16); // ~60fps
    }

    freeMap(map);
    freePlayer(player);
    freeCamera(camera); // Free camera
    if (testPNJ)
    {
        freePNJ(testPNJ);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
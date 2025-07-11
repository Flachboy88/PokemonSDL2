#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

// Inclure les en-têtes nécessaires
#include "../framework/map.h"
#include "player.h"
#include "pnj.h"
#include "../systems/camera.h"
#include "../systems/inputs.h"
#include "../systems/utils.h"

typedef enum
{
    MODE_WORLD,
    MODE_MENU,
    MODE_COMBAT,
    MODE_CINEMATIC

} GameState;

typedef struct Game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int window_width;
    int window_height;

    GameState state;

    Map *current_map;
    Player *player;
    Camera *camera;
    PNJ *testPNJ;
    Input input;
    Uint32 lastTime; // à supprimer plus tard

    bool running;
} Game;

Game *Game_Create(const char *title, int width, int height);
void Game_Free(Game *game);
void Game_HandleEvent(Game *game);
void Game_Update(Game *game);
void Game_Render(Game *game);
void Game_Run(Game *game);

// Fonctions d'initialisation internes
bool Game_InitSDL(Game *game, const char *title, int width, int height);
bool Game_InitMap(Game *game, const char *map_name);
bool Game_InitPlayer(Game *game);
bool Game_InitCamera(Game *game);
bool Game_InitPNJs(Game *game); // Pour initialiser les PNJ, si besoin

#endif // GAME_H

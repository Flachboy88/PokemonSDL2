#ifndef GAME_H // game/
#define GAME_H

#include "player.h"
#include "../framework/map.h"
#include "../systems/camera.h"
#include "../systems/inputs.h"
#include <SDL2/SDL.h>

typedef enum
{
    GAME_STATE_WORLD,
    GAME_STATE_MENU,
    GAME_STATE_BATTLE,
    GAME_STATE_PAUSE,
    GAME_STATE_CUTSCENE,
    GAME_MODE_TRANSITION
} GameState;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    Map *current_map;
    Player *player;

    Camera *camera;

    GameState state; // enum: JEU, MENU, COMBAT, etc

    // Ajouts futurs :
    /*
    Inventory *inventory;
    AudioSystem *audio;
    QuestSystem *quests;
    DialogueManager *dialogues;
    */

} Game;

// Initialize game systems and load initial assets
bool initGame(Game *game, const char *initialMapName, int window_width, int window_height);
// Free all game resources
void freeGame(Game *game);

// Update game logic (player, PNJs, camera, etc.)
void updateGame(Game *game, float deltaTime);
// Render game graphics
void renderGame(Game *game, SDL_Renderer *renderer, uint32_t current_time);
// Switch between different game states
void switchMode(Game *game, GameState state);
// Set the current map
void setMap(Game *game, Map *map);

#endif
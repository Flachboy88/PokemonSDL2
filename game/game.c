#include "game.h"

static Map *Game_LoadAndInitMap(const char *name, SDL_Renderer *renderer);
static bool Game_HandleInputEvents(Game *game, SDL_Event *event);
static void Game_UpdateData(Game *game, float deltaTime);
static void Game_UpdateGraphics(Game *game, Uint32 currentTime);

bool Game_InitSDL(Game *game, const char *title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }

    game->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!game->window)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer)
    {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(game->window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    game->window_width = width;
    game->window_height = height;

    return true;
}

bool Game_InitMap(Game *game, const char *map_name)
{
    game->current_map = Game_LoadAndInitMap(map_name, game->renderer);
    if (!game->current_map)
    {
        fprintf(stderr, "Failed to load and initialize map '%s'\n", map_name);
        return false;
    }
    return true;
}

static Map *Game_LoadAndInitMap(const char *name, SDL_Renderer *renderer)
{
    char *full_path = buildFilePath("resources/maps/", name, ".tmx");
    if (!full_path)
        return NULL;

    Map *map = loadMap(full_path, renderer);
    if (!map)
    {
        fprintf(stderr, "Error loading map '%s'\n", full_path);
        free(full_path);
        return NULL;
    }

    float spawn_x, spawn_y;
    if (Map_getPlayerSpawn(map, &spawn_x, &spawn_y))
    {
        map->default_x_spawn = spawn_x;
        map->default_y_spawn = spawn_y;
    }
    else
    {
        printf("Map loaded, but no player spawn point found ('PlayerSpawn' in 'PlayerObject' group). Using (0,0).\n");
    }

    Map_initAnimations(map);
    free(full_path);
    return map;
}

bool Game_InitPlayer(Game *game)
{
    // Ajustement de la position de spawn du joueur pour qu'il soit centré sur la tuile de spawn
    game->player = InitPlayer(game->current_map->default_x_spawn - 12, game->current_map->default_y_spawn - 32, game->renderer);
    if (!game->player)
    {
        fprintf(stderr, "Error creating player\n");
        return false;
    }
    return true;
}

bool Game_InitCamera(Game *game)
{
    game->camera = initCamera(0, 0, game->window_width, game->window_height,
                              game->current_map->tmx_map->width * game->current_map->tmx_map->tile_width,
                              game->current_map->tmx_map->height * game->current_map->tmx_map->tile_height);
    if (!game->camera)
    {
        fprintf(stderr, "Error creating camera\n");
        return false;
    }
    return true;
}

bool Game_InitPNJs(Game *game)
{
    game->testPNJ = createPNJ(100, 100, "resources/sprites/pnj.png", game->renderer);
    if (!game->testPNJ)
    {
        fprintf(stderr, "Error creating PNJ\n");
    }

    return true;
}

Game *Game_Create(const char *title, int width, int height)
{
    Game *game = (Game *)malloc(sizeof(Game));
    if (!game)
    {
        fprintf(stderr, "Failed to allocate Game structure\n");
        return NULL;
    }

    memset(game, 0, sizeof(Game));

    game->running = true;
    game->state = MODE_WORLD;

    if (!Game_InitSDL(game, title, width, height))
    {
        Game_Free(game);
        return NULL;
    }

    if (!Game_InitMap(game, "map3"))
    {
        Game_Free(game);
        return NULL;
    }

    if (!Game_InitPlayer(game))
    {
        Game_Free(game);
        return NULL;
    }

    if (!Game_InitCamera(game))
    {
        Game_Free(game);
        return NULL;
    }

    if (!Game_InitPNJs(game))
    {
    }

    game->input = (Input){false, false, false, false, false, false};
    game->lastTime = SDL_GetTicks();

    return game;
}

void Game_Free(Game *game)
{
    if (game)
    {
        if (game->current_map)
        {
            freeMap(game->current_map);
            game->current_map = NULL;
        }
        if (game->player)
        {
            freePlayer(game->player);
            game->player = NULL;
        }
        if (game->camera)
        {
            freeCamera(game->camera);
            game->camera = NULL;
        }
        if (game->testPNJ)
        {
            freePNJ(game->testPNJ);
            game->testPNJ = NULL;
        }
        if (game->renderer)
        {
            SDL_DestroyRenderer(game->renderer);
            game->renderer = NULL;
        }
        if (game->window)
        {
            SDL_DestroyWindow(game->window);
            game->window = NULL;
        }
        IMG_Quit();
        SDL_Quit();
        free(game);
    }
}

static bool Game_HandleInputEvents(Game *game, SDL_Event *event)
{
    game->input.space = false;
    game->input.r_key = false;

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
                game->input.space = true;
            }
            else if (event->key.keysym.scancode == SDL_SCANCODE_R)
            {
                game->input.r_key = true;
            }
        }
    }

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    game->input.left = keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A];
    game->input.right = keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D];
    game->input.up = keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W];
    game->input.down = keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_S];

    return true;
}

static void Game_UpdateData(Game *game, float deltaTime)
{
    processPlayerInput(game->player, &game->input, deltaTime, game->current_map);
    updatePNJ(game->testPNJ, deltaTime);
    updateCamera(game->camera, game->player->entity.x, game->player->entity.y);

    UpdatePNJs(game->current_map, deltaTime); // de map
}

static void Game_UpdateGraphics(Game *game, Uint32 currentTime)
{
    SDL_SetRenderDrawColor(game->renderer, 30, 30, 30, 255);
    SDL_RenderClear(game->renderer);

    Map_renderGroup(game->renderer, game->current_map, "Background", -game->camera->view_rect.x, -game->camera->view_rect.y, currentTime);
    Map_renderGroup(game->renderer, game->current_map, "PremierPlan", -game->camera->view_rect.x, -game->camera->view_rect.y, currentTime);

    renderPlayer(game->player, game->renderer, game->camera);
    renderPNJ(game->testPNJ, game->renderer, game->camera);
    Map_renderPNJs(game->renderer, game->current_map, game->camera);

    Map_renderGroup(game->renderer, game->current_map, "SecondPlan", -game->camera->view_rect.x, -game->camera->view_rect.y, currentTime);
    Map_drawCollisionsInCamera(game->renderer, game->current_map, game->camera);

    SDL_RenderPresent(game->renderer);
}

void Game_HandleEvent(Game *game)
{
    SDL_Event event;
    game->running = Game_HandleInputEvents(game, &event);
}

void Game_Update(Game *game)
{
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - game->lastTime) / 1000.0f;
    game->lastTime = currentTime;

    Game_UpdateData(game, deltaTime);
}

void Game_Render(Game *game)
{
    Uint32 currentTime = SDL_GetTicks();
    Game_UpdateGraphics(game, currentTime);
}

void Game_Run(Game *game)
{
    while (game->running)
    {
        Game_HandleEvent(game);
        Game_Update(game);
        Game_Render(game);
    }
}

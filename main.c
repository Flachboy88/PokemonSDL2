#include "game/game.h"

int main(int argc, char *argv[])
{

    Game *game = Game_Create("PokemonSDL2", 350, 350);
    if (!game)
    {
        fprintf(stderr, "Failed to create game instance. Exiting.\n");
        return 1;
    }

    Game_Run(game);

    Game_Free(game);

    return 0;
}

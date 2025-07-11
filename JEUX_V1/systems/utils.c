#include "utils.h"

char *buildFilePath(const char *dir, const char *name, const char *ext)
{
    size_t len = strlen(dir) + strlen(name) + strlen(ext) + 1;
    char *full_path = malloc(len);
    if (!full_path)
    {
        fprintf(stderr, "Erreur d'allocation mémoire dans buildFilePath\n");
        return NULL;
    }
    snprintf(full_path, len, "%s%s%s", dir, name, ext);
    return full_path;
}

bool InitSDL(SDL_Window **window, SDL_Renderer **renderer, int largeur, int hauteur)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Jeu",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               largeur, hauteur, 0);
    if (!*window)
    {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer)
    {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        fprintf(stderr, "Erreur IMG_Init : %s\n", IMG_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    return true;
}

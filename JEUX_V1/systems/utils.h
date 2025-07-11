#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

char* buildFilePath(const char* dir, const char* name, const char* ext);
bool InitSDL(SDL_Window **window, SDL_Renderer **renderer, int largeur, int hauteur);

#endif
#ifndef INPUTS_H
#define INPUTS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    bool left;
    bool right;
    bool up;
    bool down;
} Input;

#endif
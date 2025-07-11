// systems/camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL.h>

typedef struct
{
    SDL_Rect view_rect; // The portion of the world the camera is currently showing
    int world_width;    // Total width of the game world (map)
    int world_height;   // Total height of the game world (map)
} Camera;

// Initializes the camera with its view dimensions and the total world dimensions.
Camera *initCamera(int x, int y, int width, int height, int world_width, int world_height);

// Updates the camera's position to follow a target (e.g., the player).
// The camera's movement is clamped within the world boundaries.
void updateCamera(Camera *camera, float targetX, float targetY);

// Adjusts a world coordinate (e.g., entity position) to a screen coordinate
// based on the camera's current view.
SDL_Rect getScreenRect(Camera *camera, float worldX, float worldY, int width, int height);

// Frees the memory allocated for the camera.
void freeCamera(Camera *camera);

void setCameraWordSize(Camera *camera, int world_width, int world_height); // when we change map

#endif
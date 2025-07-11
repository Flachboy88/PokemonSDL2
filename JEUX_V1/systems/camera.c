// systems/camera.c
#include "camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

Camera *initCamera(int x, int y, int width, int height, int world_width, int world_height)
{
    Camera *camera = (Camera *)malloc(sizeof(Camera));
    if (!camera)
    {
        fprintf(stderr, "Failed to allocate memory for Camera.\n");
        return NULL;
    }

    camera->view_rect.x = x;
    camera->view_rect.y = y;
    camera->view_rect.w = width;
    camera->view_rect.h = height;
    camera->world_width = world_width;
    camera->world_height = world_height;

    return camera;
}

void updateCamera(Camera *camera, float targetX, float targetY)
{
    // Center the camera on the target
    camera->view_rect.x = (int)(targetX - camera->view_rect.w / 2);
    camera->view_rect.y = (int)(targetY - camera->view_rect.h / 2);

    // Clamp camera to world boundaries
    if (camera->view_rect.x < 0)
    {
        camera->view_rect.x = 0;
    }
    if (camera->view_rect.y < 0)
    {
        camera->view_rect.y = 0;
    }
    if (camera->view_rect.x + camera->view_rect.w > camera->world_width)
    {
        camera->view_rect.x = camera->world_width - camera->view_rect.w;
    }
    if (camera->view_rect.y + camera->view_rect.h > camera->world_height)
    {
        camera->view_rect.y = camera->world_height - camera->view_rect.h;
    }
}

SDL_Rect getScreenRect(Camera *camera, float worldX, float worldY, int width, int height)
{
    SDL_Rect screen_rect;
    screen_rect.x = (int)(worldX - camera->view_rect.x);
    screen_rect.y = (int)(worldY - camera->view_rect.y);
    screen_rect.w = width;
    screen_rect.h = height;
    return screen_rect;
}

void freeCamera(Camera *camera)
{
    if (camera)
    {
        free(camera);
    }
}

void setCameraWordSize(Camera *camera, int world_width, int world_height)
{
    camera->world_width = world_width;
    camera->world_height = world_height;
}
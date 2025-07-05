// map.c
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_image.h>

// Global renderer pour le loader
static SDL_Renderer *global_renderer = NULL;

// Callback pour charger les textures via SDL_Image
static void *SDL_tex_loader(const char *path)
{
    SDL_Texture *tex = IMG_LoadTexture(global_renderer, path);
    if (!tex)
    {
        fprintf(stderr, "Erreur SDL_Image: %s\n", IMG_GetError());
    }
    return tex;
}

// Callback pour libérer les textures
static void SDL_tex_deleter(void *res)
{
    SDL_DestroyTexture((SDL_Texture *)res);
}

Map *loadMap(const char *filePath, SDL_Renderer *renderer)
{
    global_renderer = renderer;
    tmx_img_load_func = SDL_tex_loader;
    tmx_img_free_func = SDL_tex_deleter;

    Map *map = malloc(sizeof(Map));
    if (!map)
        return NULL;

    map->tmx_map = tmx_load(filePath);
    if (!map->tmx_map)
    {
        fprintf(stderr, "Erreur libTMX: %s\n", tmx_strerr());
        free(map);
        return NULL;
    }

    map->default_x_spawn = map->default_y_spawn = 0.0f;
    Map_getPlayerSpawn(map, &map->default_x_spawn, &map->default_y_spawn);
    return map;
}

void freeMap(Map *map)
{
    if (map)
    {
        tmx_map_free(map->tmx_map);
        free(map);
    }
}

static void draw_tile(SDL_Renderer *ren, tmx_map *m, tmx_tile *tile, int dx, int dy)
{
    if (!tile)
        return;
    SDL_Texture *tex = (SDL_Texture *)(tile->image
                                           ? tile->image->resource_image
                                           : tile->tileset->image->resource_image);
    if (!tex)
        return;

    SDL_Rect src = {tile->ul_x, tile->ul_y, tile->width, tile->height};
    SDL_Rect dst = {dx, dy, tile->width, tile->height};
    SDL_RenderCopy(ren, tex, &src, &dst);
}

static void draw_layer(SDL_Renderer *ren, tmx_map *m, tmx_layer *layer)
{
    if (!layer->visible || layer->type != L_LAYER)
        return;
    unsigned w = m->width, h = m->height;

    for (unsigned y = 0; y < h; y++)
    {
        for (unsigned x = 0; x < w; x++)
        {
            uint32_t cell = layer->content.gids[y * w + x];
            uint32_t gid = cell & TMX_FLIP_BITS_REMOVAL;
            tmx_tile *tile = m->tiles[gid];
            if (tile)
            {
                draw_tile(ren, m, tile, x * m->tile_width, y * m->tile_height);
            }
        }
    }
}

static void draw_objects(SDL_Renderer *ren, tmx_object_group *og)
{
    SDL_Rect rect;
    tmx_object *o = og->head;
    while (o)
    {
        if (o->visible && o->obj_type == OT_SQUARE)
        {
            rect.x = o->x;
            rect.y = o->y;
            rect.w = o->width;
            rect.h = o->height;
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 128);
            SDL_RenderDrawRect(ren, &rect);
        }
        o = o->next;
    }
}

static void draw_image_layer(SDL_Renderer *ren, tmx_image *img)
{
    SDL_Texture *tex = (SDL_Texture *)img->resource_image;
    if (!tex)
        return;
    SDL_Rect dst = {0, 0, img->width, img->height};
    SDL_RenderCopy(ren, tex, NULL, &dst);
}

static void recurse_layers(SDL_Renderer *ren, tmx_map *m, tmx_layer *layer)
{
    while (layer)
    {
        if (layer->visible)
        {
            switch (layer->type)
            {
            case L_GROUP:
                recurse_layers(ren, m, layer->content.group_head);
                break;
            case L_LAYER:
                draw_layer(ren, m, layer);
                break;
            case L_OBJGR:
                draw_objects(ren, layer->content.objgr);
                break;
            case L_IMAGE:
                draw_image_layer(ren, layer->content.image);
                break;
            default:
                break;
            }
        }
        layer = layer->next;
    }
}

void Map_afficherGroup(SDL_Renderer *renderer, Map *map, const char *groupName, int offsetX, int offsetY)
{
    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, groupName);
    if (layer && layer->type == L_GROUP)
    {
        recurse_layers(renderer, map->tmx_map, layer->content.group_head);
    }
}

CollisionObject *Map_getCollisionObjects(Map *map, const char *objectGroupName, int *count)
{
    *count = 0;
    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, objectGroupName);
    if (!layer || layer->type != L_OBJGR)
        return NULL;

    tmx_object_group *og = layer->content.objgr;
    int c = 0;
    for (tmx_object *o = og->head; o; o = o->next)
        c++;
    if (c == 0)
        return NULL;

    CollisionObject *arr = malloc(c * sizeof(CollisionObject));
    int i = 0;
    for (tmx_object *o = og->head; o; o = o->next)
    {
        arr[i].rect.x = o->x;
        arr[i].rect.y = o->y;
        arr[i].rect.w = o->width;
        arr[i].rect.h = o->height;
        arr[i].name = o->name ? strdup(o->name) : NULL;
        arr[i].type = o->type ? strdup(o->type) : NULL;
        i++;
    }
    *count = c;
    return arr;
}

bool Map_getPlayerSpawn(Map *map, float *x, float *y)
{
    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, "PlayerObject");
    if (!layer || layer->type != L_OBJGR)
        return false;
    for (tmx_object *o = layer->content.objgr->head; o; o = o->next)
    {
        if (o->name && strcmp(o->name, "PlayerSpawn") == 0)
        {
            *x = o->x;
            *y = o->y;
            return true;
        }
    }
    return false;
}

bool Map_setTile(Map *map, const char *layerName, int x, int y, int gid)
{
    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, layerName);
    if (!layer || layer->type != L_LAYER)
        return false;
    if (x < 0 || y < 0 || x >= map->tmx_map->width || y >= map->tmx_map->height)
        return false;

    layer->content.gids[y * map->tmx_map->width + x] = gid;
    return true;
}

/*
void updateMapAnimations(Map *map)
{
    tmx_update_animation(map->tmx_map, SDL_GetTicks());
}
*/
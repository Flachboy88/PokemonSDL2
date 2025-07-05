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

// Liste globale (ou membre de la structure Map) pour stocker les AnimatedTileInfo
// Pour la simplicité, nous allons utiliser un tableau dynamique.
// Une meilleure approche serait une liste liée ou un tableau redimensionnable.
static AnimatedTileInfo *animated_tiles_infos = NULL;
static int animated_tiles_count = 0;
static int animated_tiles_capacity = 0;

// Fonction utilitaire pour ajouter une tuile animée à notre liste
static void add_animated_tile_info(tmx_tile *tile, uint32_t first_gid)
{
    // Vérifier si la tuile ou son animation est valide
    if (!tile || !tile->animation)
        return;

    // Calculer le GID global pour la comparaison
    uint32_t global_gid = first_gid + tile->id;

    // Vérifier si cette tuile (par son GID global) est déjà dans notre liste
    for (int i = 0; i < animated_tiles_count; ++i)
    {
        if ((animated_tiles_infos[i].local_tile_id + animated_tiles_infos[i].tileset_first_gid) == global_gid)
        {
            return; // Déjà suivi
        }
    }

    // Agrandir le tableau si nécessaire
    if (animated_tiles_count >= animated_tiles_capacity)
    {
        animated_tiles_capacity = (animated_tiles_capacity == 0) ? 10 : animated_tiles_capacity * 2;
        animated_tiles_infos = realloc(animated_tiles_infos, animated_tiles_capacity * sizeof(AnimatedTileInfo));
        if (!animated_tiles_infos)
        {
            fprintf(stderr, "Erreur d'allocation mémoire pour animated_tiles_infos.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Ajouter la nouvelle information de tuile animée
    animated_tiles_infos[animated_tiles_count].local_tile_id = tile->id;
    animated_tiles_infos[animated_tiles_count].tileset_first_gid = first_gid;
    animated_tiles_infos[animated_tiles_count].tmx_tile_ptr = tile; // Pointeur vers la tuile originale
    animated_tiles_infos[animated_tiles_count].current_frame_index = 0;
    animated_tiles_infos[animated_tiles_count].frame_start_time = SDL_GetTicks(); // Temps actuel

    animated_tiles_count++;
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

    // Initialiser les informations d'animation après le chargement de la carte
    Map_initAnimations(map);

    return map;
}

void freeMap(Map *map)
{
    if (map)
    {
        tmx_map_free(map->tmx_map);
        free(map);
    }
    if (animated_tiles_infos)
    {
        free(animated_tiles_infos);
        animated_tiles_infos = NULL;
        animated_tiles_count = 0;
        animated_tiles_capacity = 0;
    }
}

// Modification de draw_tile pour accepter un tmx_tile* qui est la frame actuelle
static void draw_tile(SDL_Renderer *ren, tmx_tile *tile, int dx, int dy, int tile_width, int tile_height)
{
    if (!tile)
        return;
    SDL_Texture *tex = (SDL_Texture *)(tile->image
                                           ? tile->image->resource_image
                                           : tile->tileset->image->resource_image);
    if (!tex)
        return;

    SDL_Rect src = {tile->ul_x, tile->ul_y, tile->width, tile->height};
    SDL_Rect dst = {dx, dy, tile_width, tile_height};
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

            if (gid == 0)
                continue; // Tuile vide

            tmx_tile *tile = m->tiles[gid]; // Obtient la tuile originale (ou la première frame si animée)
            if (tile)
            {
                tmx_tile *tile_to_draw = tile; // Par défaut, dessine la tuile statique ou la première frame

                for (int i = 0; i < animated_tiles_count; ++i)
                {
                    // Calculate the global GID of the animated tile from its stored info
                    uint32_t animated_tile_global_gid = animated_tiles_infos[i].tileset_first_gid + animated_tiles_infos[i].local_tile_id;

                    if (animated_tile_global_gid == gid) // Compare with the current tile's GID from the map layer
                    {
                        // Correction: Accès direct au tableau de frames et à sa longueur
                        // 'animation' est un tmx_anim_frame* et 'animation_len' est sa taille
                        if (animated_tiles_infos[i].tmx_tile_ptr->animation)
                        { // Vérifier que l'animation existe
                            tmx_anim_frame current_frame = animated_tiles_infos[i].tmx_tile_ptr->animation[animated_tiles_infos[i].current_frame_index];
                            // CORRECTED LINE: Use tileset_first_gid to get the actual global ID of the animation frame
                            tile_to_draw = m->tiles[animated_tiles_infos[i].tileset_first_gid + current_frame.tile_id];
                        }
                        break;
                    }
                }
                draw_tile(ren, tile_to_draw, x * m->tile_width, y * m->tile_height, m->tile_width, m->tile_height);
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
    // Note: offsetX et offsetY ne sont pas utilisés dans cette implémentation directe
    // Si vous voulez un scrolling, vous devrez ajuster les positions dx, dy dans draw_tile.
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

void Map_initAnimations(Map *map)
{
    tmx_tileset_list *ts_list_item = map->tmx_map->ts_head;
    while (ts_list_item)
    {
        tmx_tileset *tileset = ts_list_item->tileset;
        if (tileset->tiles) // Check if the tileset has individual tiles defined
        {
            uint32_t current_first_gid = ts_list_item->firstgid; // Get the first GID for this tileset
            for (unsigned int i = 0; i < tileset->tilecount; ++i)
            {
                tmx_tile *tile = &tileset->tiles[i]; // Get pointer to the tmx_tile structure
                // Si la tuile a une animation (animation pointer non NULL)
                if (tile && tile->animation)
                {
                    add_animated_tile_info(tile, current_first_gid); // Pass first_gid
                }
            }
        }
        ts_list_item = ts_list_item->next; // Passer au tileset suivant dans la liste
    }
}
void Map_updateAnimations(Map *map, uint32_t current_time)
{
    for (int i = 0; i < animated_tiles_count; ++i)
    {
        AnimatedTileInfo *info = &animated_tiles_infos[i];

        // Correction: Accès direct au tableau de frames et à sa longueur
        // 'animation' est un tmx_anim_frame* et 'animation_len' est sa taille
        if (info->tmx_tile_ptr->animation && info->tmx_tile_ptr->animation_len > 0)
        { // Vérifier que l'animation existe et a des frames
            tmx_anim_frame current_frame = info->tmx_tile_ptr->animation[info->current_frame_index];

            if (current_time - info->frame_start_time >= current_frame.duration)
            {
                // Passer à la frame suivante
                info->current_frame_index++;
                // Utiliser animation_len comme le 'count' de l'animation
                if (info->current_frame_index >= info->tmx_tile_ptr->animation_len)
                {
                    info->current_frame_index = 0; // Revenir au début de l'animation
                }
                info->frame_start_time = current_time; // Mettre à jour le temps de début de la frame
            }
        }
    }
}
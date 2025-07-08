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

// Structure pour stocker les informations d'une tuile animée
typedef struct
{
    uint32_t local_tile_id;     // Local ID of the animated tile within its tileset
    uint32_t tileset_first_gid; // First GID of the tileset this tile belongs to
    tmx_tile *tmx_tile_ptr;     // Pointeur vers la structure tmx_tile pour cette animation
    int current_frame_index;    // Index de la frame actuelle dans l'animation
    uint32_t frame_start_time;  // Temps (en ms) où la frame actuelle a commencé à s'afficher
} AnimatedTileInfo;

// Liste globale (ou membre de la structure Map) pour stocker les AnimatedTileInfo
static AnimatedTileInfo *animated_tiles_infos = NULL;
static int animated_tiles_count = 0;
static int animated_tiles_capacity = 0;

// Déclarations des fonctions statiques
static void draw_tile(SDL_Renderer *ren, tmx_tile *tile, int dx, int dy, int tile_width, int tile_height, int offsetX, int offsetY);
static void draw_layer(SDL_Renderer *ren, tmx_map *m, tmx_layer *layer, uint32_t current_time, int offsetX, int offsetY);
static void draw_objects(SDL_Renderer *ren, tmx_object_group *og, int offsetX, int offsetY);
static void draw_image_layer(SDL_Renderer *ren, tmx_image *img, int offsetX, int offsetY);
static void recurse_layers(SDL_Renderer *ren, tmx_map *m, tmx_layer *layer, uint32_t current_time, int offsetX, int offsetY);
static void add_animated_tile_info(tmx_tile *tile, uint32_t first_gid);

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

void DeBugMap(Map *map)
{
    // Debug : afficher les coordonnées des collisions
    printf("=== DEBUG COLLISIONS ===\n");
    printf("Nombre de collisions trouvées: %d\n", map->collision_count);
    for (int i = 0; i < map->collision_count; i++)
    {
        printf("Collision %d:\n", i);
        printf("  Position: x=%.2f, y=%.2f\n", (float)map->collisions[i].rect.x, (float)map->collisions[i].rect.y);
        printf("  Taille: w=%.2f, h=%.2f\n", (float)map->collisions[i].rect.w, (float)map->collisions[i].rect.h);
        printf("  Nom: %s\n", map->collisions[i].name ? map->collisions[i].name : "NULL");
        printf("  Type: %s\n", map->collisions[i].type ? map->collisions[i].type : "NULL");
        printf("\n");
    }
    printf("========================\n");
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

    map->collisions = Map_getCollisionObjects(map, "CollisionObject", &map->collision_count);

    map->pnjs = NULL;
    map->pnj_count = 0;
    Map_initPNJs(map, renderer);

    // DeBugMap(map);
    Map_initAnimations(map);

    return map;
}

void freeMap(Map *map)
{
    if (map)
    {
        if (map->collisions)
        {
            for (int i = 0; i < map->collision_count; i++)
            {
                free(map->collisions[i].name);
                free(map->collisions[i].type);
                // Libérer les points des polygones
                if (map->collisions[i].polygon_points)
                {
                    free(map->collisions[i].polygon_points);
                }
            }
            free(map->collisions);
        }

        // Libérer les PNJs
        if (map->pnjs)
        {
            for (int i = 0; i < map->pnj_count; i++)
            {
                if (map->pnjs[i])
                {
                    freePNJ(map->pnjs[i]);
                }
            }
            free(map->pnjs);
        }

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

// Modification de draw_tile pour accepter un tmx_tile* qui est la frame actuelle et offsets
static void draw_tile(SDL_Renderer *ren, tmx_tile *tile, int dx, int dy, int tile_width, int tile_height, int offsetX, int offsetY)
{
    if (!tile)
        return;
    SDL_Texture *tex = (SDL_Texture *)(tile->image
                                           ? tile->image->resource_image
                                           : tile->tileset->image->resource_image);
    if (!tex)
        return;

    SDL_Rect src = {tile->ul_x, tile->ul_y, tile->width, tile->height};
    SDL_Rect dst = {dx + offsetX, dy + offsetY, tile_width, tile_height}; // Apply offsets
    SDL_RenderCopy(ren, tex, &src, &dst);
}

// La fonction draw_layer prend maintenant un 'current_time' et offsets
static void draw_layer(SDL_Renderer *ren, tmx_map *m, tmx_layer *layer, uint32_t current_time, int offsetX, int offsetY)
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

            tmx_tile *tile = m->tiles[gid]; // Obtient la tuile originale
            if (tile)
            {
                tmx_tile *tile_to_draw = tile;

                // Si la tuile a une animation, mettez à jour son index de frame
                if (tile->animation && tile->animation_len > 0)
                {
                    // Chercher l'info d'animation pour cette tuile
                    AnimatedTileInfo *info = NULL;
                    for (int i = 0; i < animated_tiles_count; ++i)
                    {
                        uint32_t animated_tile_global_gid = animated_tiles_infos[i].tileset_first_gid + animated_tiles_infos[i].local_tile_id;
                        if (animated_tile_global_gid == gid)
                        {
                            info = &animated_tiles_infos[i];
                            break;
                        }
                    }

                    if (info) // Si on a trouvé l'info d'animation pour cette tuile
                    {
                        tmx_anim_frame current_frame_data = info->tmx_tile_ptr->animation[info->current_frame_index];

                        if (current_time - info->frame_start_time >= current_frame_data.duration)
                        {
                            info->current_frame_index++;
                            if (info->current_frame_index >= info->tmx_tile_ptr->animation_len)
                            {
                                info->current_frame_index = 0;
                            }
                            info->frame_start_time = current_time;
                        }
                        // Utiliser la GID de la frame actuelle pour obtenir la tuile à dessiner
                        tile_to_draw = m->tiles[info->tileset_first_gid + info->tmx_tile_ptr->animation[info->current_frame_index].tile_id];
                    }
                }
                draw_tile(ren, tile_to_draw, x * m->tile_width, y * m->tile_height, m->tile_width, m->tile_height, offsetX, offsetY);
            }
        }
    }
}

static void draw_objects(SDL_Renderer *ren, tmx_object_group *og, int offsetX, int offsetY)
{
    SDL_Rect rect;
    tmx_object *o = og->head;
    while (o)
    {
        if (o->visible && o->obj_type == OT_SQUARE)
        {
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 128); // Rouge semi-transparent
            rect.x = o->x + offsetX;                     // Apply offsets
            rect.y = o->y + offsetY;                     // Apply offsets
            rect.w = o->width;
            rect.h = o->height;
            SDL_RenderDrawRect(ren, &rect);
        }
        o = o->next;
    }
}

static void draw_image_layer(SDL_Renderer *ren, tmx_image *img, int offsetX, int offsetY)
{
    SDL_Texture *tex = (SDL_Texture *)img->resource_image;
    if (!tex)
        return;
    SDL_Rect dst = {offsetX, offsetY, img->width, img->height}; // Apply offsets
    SDL_RenderCopy(ren, tex, NULL, &dst);
}

static void recurse_layers(SDL_Renderer *ren, tmx_map *m, tmx_layer *layer, uint32_t current_time, int offsetX, int offsetY)
{
    while (layer)
    {
        if (layer->visible)
        {
            switch (layer->type)
            {
            case L_GROUP:
                recurse_layers(ren, m, layer->content.group_head, current_time, offsetX, offsetY);
                break;
            case L_LAYER:
                draw_layer(ren, m, layer, current_time, offsetX, offsetY); // Pass offsets to draw_layer
                break;
            case L_OBJGR:
                draw_objects(ren, layer->content.objgr, offsetX, offsetY); // Pass offsets to draw_objects
                break;
            case L_IMAGE:
                draw_image_layer(ren, layer->content.image, offsetX, offsetY); // Pass offsets to draw_image_layer
                break;
            default:
                break;
            }
        }
        layer = layer->next;
    }
}

// Renamed from Map_afficherGroup to Map_renderGroup
void Map_renderGroup(SDL_Renderer *renderer, Map *map, const char *groupName, int offsetX, int offsetY, uint32_t current_time)
{
    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, groupName);
    if (layer && layer->type == L_GROUP)
    {
        recurse_layers(renderer, map->tmx_map, layer->content.group_head, current_time, offsetX, offsetY);
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

        // Gérer les polygones
        if (o->obj_type == OT_POLYGON && o->content.shape->points_len > 0)
        {
            arr[i].is_polygon = true;
            arr[i].polygon_count = o->content.shape->points_len;
            arr[i].polygon_points = malloc(arr[i].polygon_count * sizeof(Point));

            for (int j = 0; j < arr[i].polygon_count; j++)
            {
                arr[i].polygon_points[j].x = o->x + o->content.shape->points[j][0];
                arr[i].polygon_points[j].y = o->y + o->content.shape->points[j][1];
            }
        }
        else
        {
            arr[i].is_polygon = false;
            arr[i].polygon_points = NULL;
            arr[i].polygon_count = 0;
        }

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
    // Clear previous animated tiles info if map is reloaded
    if (animated_tiles_infos)
    {
        free(animated_tiles_infos);
        animated_tiles_infos = NULL;
        animated_tiles_count = 0;
        animated_tiles_capacity = 0;
    }

    tmx_tileset_list *ts_list_item = map->tmx_map->ts_head;
    while (ts_list_item)
    {
        tmx_tileset *tileset = ts_list_item->tileset;
        if (tileset->tiles)
        {
            uint32_t current_first_gid = ts_list_item->firstgid;
            for (unsigned int i = 0; i < tileset->tilecount; ++i)
            {
                tmx_tile *tile = &tileset->tiles[i];
                if (tile && tile->animation)
                {
                    add_animated_tile_info(tile, current_first_gid);
                }
            }
        }
        ts_list_item = ts_list_item->next;
    }
}

// New function to draw collisions using camera
void Map_drawCollisionsInCamera(SDL_Renderer *renderer, Map *map, Camera *camera)
{
    if (!map || !renderer || !camera)
        return;

    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    for (int i = 0; i < map->collision_count; i++)
    {
        if (map->collisions[i].is_polygon)
        {
            Point *points = map->collisions[i].polygon_points;
            int count = map->collisions[i].polygon_count;

            for (int j = 0; j < count; j++)
            {
                int next = (j + 1) % count;
                SDL_Point p1_screen = {(int)(points[j].x - camera->view_rect.x), (int)(points[j].y - camera->view_rect.y)};
                SDL_Point p2_screen = {(int)(points[next].x - camera->view_rect.x), (int)(points[next].y - camera->view_rect.y)};

                for (int k = -1; k <= 1; k++)
                {
                    for (int l = -1; l <= 1; l++)
                    {
                        SDL_RenderDrawLine(renderer,
                                           p1_screen.x + k, p1_screen.y + l,
                                           p2_screen.x + k, p2_screen.y + l);
                    }
                }
            }
        }
        else
        {
            SDL_Rect world_rect = map->collisions[i].rect;
            SDL_Rect screen_rect = getScreenRect(camera, world_rect.x, world_rect.y, world_rect.w, world_rect.h);

            for (int k = -1; k <= 1; k++)
            {
                for (int l = -1; l <= 1; l++)
                {
                    SDL_Rect thickRect = {screen_rect.x + k, screen_rect.y + l, screen_rect.w, screen_rect.h};
                    SDL_RenderDrawRect(renderer, &thickRect);
                }
            }
        }
    }

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Map_initPNJs(Map *map, SDL_Renderer *renderer)
{
    printf("=== DEBUG PNJs ===\n");
    if (!map || !renderer)
        return;

    // Chercher le layer "PNJObject" ou similaire
    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, "PNJObject");
    if (!layer || layer->type != L_OBJGR)
        return;

    tmx_object_group *og = layer->content.objgr;

    // Compter les PNJs
    int count = 0;
    for (tmx_object *o = og->head; o; o = o->next)
    {
        if (o->name && strncmp(o->name, "PNJ", 3) == 0) // Objets qui commencent par "PNJ"
            count++;
    }

    if (count == 0)
    {
        map->pnjs = NULL;
        map->pnj_count = 0;
        return;
    }

    // Allouer le tableau de PNJs
    map->pnjs = malloc(count * sizeof(PNJ *));
    map->pnj_count = count;

    int i = 0;
    for (tmx_object *o = og->head; o; o = o->next)
    {
        if (o->name && strncmp(o->name, "PNJ", 3) == 0)
        {

            const char *spritePath = ""; // Valeur par défaut

            tmx_property *sprite_prop = tmx_get_property(o->properties, "sprite");
            if (sprite_prop && sprite_prop->type == PT_STRING)
            {
                spritePath = sprite_prop->value.string;
                printf("DEBUG: PNJ object '%s' has sprite property: %s\n", o->name, spritePath);
            }
            else
            {
                printf("DEBUG: PNJ object '%s' DOES NOT have a 'sprite' property or it's not a string. Using default empty path.\n", o->name); // Add this line
            }

            // Créer le PNJ
            map->pnjs[i] = createPNJ(o->x, o->y, spritePath, renderer);

            if (map->pnjs[i])
            {
                // Sauvegarder les valeurs par défaut
                map->pnjs[i]->default_x_spawn = o->x;
                map->pnjs[i]->default_y_spawn = o->y;
                map->pnjs[i]->default_dir = 3; // Direction par défaut (bas)

                // Récupérer la direction depuis les propriétés si elle existe
                tmx_property *dir_prop = tmx_get_property(o->properties, "direction");
                if (dir_prop && dir_prop->type == PT_INT)
                {
                    map->pnjs[i]->default_dir = dir_prop->value.integer;
                }

                setPNJDirection(map->pnjs[i], map->pnjs[i]->default_dir);
                map->pnjs[i]->aEteInit = true;
            }
            i++;
        }
    }

    printf("=== DEBUG PNJs ===\n");
    printf("Nombre de PNJs créés: %d\n", map->pnj_count);
    for (int j = 0; j < map->pnj_count; j++)
    {
        if (map->pnjs[j])
        {
            printf("PNJ %d:\n", j);
            printf("  Position spawn: x=%.2f, y=%.2f\n", map->pnjs[j]->default_x_spawn, map->pnjs[j]->default_y_spawn);
            printf("  Position actuelle: x=%.2f, y=%.2f\n", map->pnjs[j]->entity.x, map->pnjs[j]->entity.y);
            printf("  Direction par défaut: %d\n", map->pnjs[j]->default_dir);
            printf("  Direction actuelle: %d\n", map->pnjs[j]->direction);
            printf("  Vitesse: %.2f\n", map->pnjs[j]->speed);
            printf("  Taille: %.2fx%.2f\n", map->pnjs[j]->entity.width, map->pnjs[j]->entity.height);
            printf("  Visible: %s\n", map->pnjs[j]->entity.visible ? "true" : "false");
            printf("  Layer: %d\n", map->pnjs[j]->entity.layer);
            printf("  A été initialisé: %s\n", map->pnjs[j]->aEteInit ? "true" : "false");
            printf("  En mouvement: %s\n", map->pnjs[j]->moving ? "true" : "false");
            printf("  A une cible: %s\n", map->pnjs[j]->hasTarget ? "true" : "false");
            printf("\n");
        }
        else
        {
            printf("PNJ %d: NULL (erreur de création)\n", j);
        }
    }
    printf("==================\n");
}

void Map_renderPNJs(SDL_Renderer *renderer, Map *map, Camera *camera)
{
    if (!map || !renderer || !camera)
        return;

    for (int i = 0; i < map->pnj_count; i++)
    {
        if (map->pnjs[i])
        {
            renderPNJ(map->pnjs[i], renderer, camera);
        }
    }
}
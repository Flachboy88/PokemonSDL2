#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cjson/cJSON.h>

// Fonction pour charger un TSX
static Tileset* loadTSX(const char *tsx_path, int firstgid, SDL_Renderer *renderer) {
    xmlDoc *doc = xmlReadFile(tsx_path, NULL, 0);
    if (!doc) {
        fprintf(stderr, "Erreur lecture TSX: %s\n", tsx_path);
        return NULL;
    }
    
    xmlNode *root = xmlDocGetRootElement(doc);
    if (!root || xmlStrcmp(root->name, (const xmlChar*)"tileset")) {
        xmlFreeDoc(doc);
        return NULL;
    }
    
    Tileset *tileset = calloc(1, sizeof(Tileset));
    tileset->firstgid = firstgid;
    
    // Récupérer les attributs
    xmlChar *prop = xmlGetProp(root, (const xmlChar*)"name");
    if (prop) {
        tileset->name = strdup((char*)prop);
        xmlFree(prop);
    }
    
    prop = xmlGetProp(root, (const xmlChar*)"tilewidth");
    if (prop) {
        tileset->tilewidth = atoi((char*)prop);
        xmlFree(prop);
    }
    
    prop = xmlGetProp(root, (const xmlChar*)"tileheight");
    if (prop) {
        tileset->tileheight = atoi((char*)prop);
        xmlFree(prop);
    }
    
    prop = xmlGetProp(root, (const xmlChar*)"tilecount");
    if (prop) {
        tileset->tilecount = atoi((char*)prop);
        xmlFree(prop);
    }
    
    prop = xmlGetProp(root, (const xmlChar*)"columns");
    if (prop) {
        tileset->columns = atoi((char*)prop);
        xmlFree(prop);
    }
    
    // Chercher l'élément image
    xmlNode *image_node = NULL;
    for (xmlNode *child = root->children; child; child = child->next) {
        if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, (const xmlChar*)"image") == 0) {
            image_node = child;
            break;
        }
    }
    
    if (image_node) {
        prop = xmlGetProp(image_node, (const xmlChar*)"source");
        if (prop) {
            // Construire le chemin complet
            char *tsx_dir = strdup(tsx_path);
            char *last_slash = strrchr(tsx_dir, '/');
            if (last_slash) {
                *last_slash = '\0';
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s/%s", tsx_dir, (char*)prop);
                tileset->image_path = strdup(full_path);
            } else {
                tileset->image_path = strdup((char*)prop);
            }
            free(tsx_dir);
            xmlFree(prop);
        }
    }
    
    // Charger la texture
    if (tileset->image_path) {
        SDL_Surface *surface = IMG_Load(tileset->image_path);
        if (surface) {
            tileset->texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
    
    xmlFreeDoc(doc);
    return tileset;
}

// Fonction pour construire le chemin relatif
static char* buildRelativePath(const char *base_path, const char *relative_path) {
    char *base_dir = strdup(base_path);
    char *last_slash = strrchr(base_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        char *full_path = malloc(strlen(base_dir) + strlen(relative_path) + 2);
        sprintf(full_path, "%s/%s", base_dir, relative_path);
        free(base_dir);
        return full_path;
    }
    free(base_dir);
    return strdup(relative_path);
}

// Fonction pour charger les tilesets
static void loadTilesets(Map *map, cJSON *tilesets_json, const char *map_path, SDL_Renderer *renderer) {
    int tileset_count = cJSON_GetArraySize(tilesets_json);
    map->tilesets = calloc(tileset_count, sizeof(Tileset));
    map->tileset_count = 0;
    
    for (int i = 0; i < tileset_count; i++) {
        cJSON *tileset_json = cJSON_GetArrayItem(tilesets_json, i);
        cJSON *firstgid_json = cJSON_GetObjectItem(tileset_json, "firstgid");
        cJSON *source_json = cJSON_GetObjectItem(tileset_json, "source");
        
        if (firstgid_json && source_json) {
            int firstgid = firstgid_json->valueint;
            char *tsx_path = buildRelativePath(map_path, source_json->valuestring);
            
            Tileset *tileset = loadTSX(tsx_path, firstgid, renderer);
            if (tileset) {
                map->tilesets[map->tileset_count++] = *tileset;
                free(tileset);
            }
            free(tsx_path);
        }
    }
}

// Fonction pour charger un tile layer
static void loadTileLayer(TileLayer *layer, cJSON *layer_json) {
    cJSON *id_json = cJSON_GetObjectItem(layer_json, "id");
    cJSON *name_json = cJSON_GetObjectItem(layer_json, "name");
    cJSON *width_json = cJSON_GetObjectItem(layer_json, "width");
    cJSON *height_json = cJSON_GetObjectItem(layer_json, "height");
    cJSON *data_json = cJSON_GetObjectItem(layer_json, "data");
    cJSON *visible_json = cJSON_GetObjectItem(layer_json, "visible");
    cJSON *opacity_json = cJSON_GetObjectItem(layer_json, "opacity");
    cJSON *x_json = cJSON_GetObjectItem(layer_json, "x");
    cJSON *y_json = cJSON_GetObjectItem(layer_json, "y");
    
    if (id_json) layer->id = id_json->valueint;
    if (name_json) layer->name = strdup(name_json->valuestring);
    if (width_json) layer->width = width_json->valueint;
    if (height_json) layer->height = height_json->valueint;
    if (visible_json) layer->visible = cJSON_IsTrue(visible_json);
    else layer->visible = true;
    if (opacity_json) layer->opacity = opacity_json->valuedouble;
    else layer->opacity = 1.0f;
    if (x_json) layer->x = x_json->valueint;
    if (y_json) layer->y = y_json->valueint;
    
    if (data_json && cJSON_IsArray(data_json)) {
        int data_size = cJSON_GetArraySize(data_json);
        layer->data = calloc(data_size, sizeof(int));
        for (int i = 0; i < data_size; i++) {
            cJSON *tile = cJSON_GetArrayItem(data_json, i);
            if (tile) layer->data[i] = tile->valueint;
        }
    }
}

// Fonction pour charger un object layer
static void loadObjectLayer(ObjectLayer *layer, cJSON *layer_json) {
    cJSON *id_json = cJSON_GetObjectItem(layer_json, "id");
    cJSON *name_json = cJSON_GetObjectItem(layer_json, "name");
    cJSON *objects_json = cJSON_GetObjectItem(layer_json, "objects");
    cJSON *visible_json = cJSON_GetObjectItem(layer_json, "visible");
    cJSON *opacity_json = cJSON_GetObjectItem(layer_json, "opacity");
    cJSON *x_json = cJSON_GetObjectItem(layer_json, "x");
    cJSON *y_json = cJSON_GetObjectItem(layer_json, "y");
    
    if (id_json) layer->id = id_json->valueint;
    if (name_json) layer->name = strdup(name_json->valuestring);
    if (visible_json) layer->visible = cJSON_IsTrue(visible_json);
    else layer->visible = true;
    if (opacity_json) layer->opacity = opacity_json->valuedouble;
    else layer->opacity = 1.0f;
    if (x_json) layer->x = x_json->valueint;
    if (y_json) layer->y = y_json->valueint;
    
    if (objects_json && cJSON_IsArray(objects_json)) {
        int object_count = cJSON_GetArraySize(objects_json);
        layer->objects = calloc(object_count, sizeof(MapObject));
        layer->object_count = 0;
        
        for (int i = 0; i < object_count; i++) {
            cJSON *obj_json = cJSON_GetArrayItem(objects_json, i);
            MapObject *obj = &layer->objects[layer->object_count++];
            
            cJSON *obj_id = cJSON_GetObjectItem(obj_json, "id");
            cJSON *obj_name = cJSON_GetObjectItem(obj_json, "name");
            cJSON *obj_type = cJSON_GetObjectItem(obj_json, "type");
            cJSON *obj_x = cJSON_GetObjectItem(obj_json, "x");
            cJSON *obj_y = cJSON_GetObjectItem(obj_json, "y");
            cJSON *obj_width = cJSON_GetObjectItem(obj_json, "width");
            cJSON *obj_height = cJSON_GetObjectItem(obj_json, "height");
            cJSON *obj_point = cJSON_GetObjectItem(obj_json, "point");
            cJSON *obj_rotation = cJSON_GetObjectItem(obj_json, "rotation");
            cJSON *obj_visible = cJSON_GetObjectItem(obj_json, "visible");
            
            if (obj_id) obj->id = obj_id->valueint;
            if (obj_name) obj->name = strdup(obj_name->valuestring);
            if (obj_type) obj->type = strdup(obj_type->valuestring);
            if (obj_x) obj->x = obj_x->valuedouble;
            if (obj_y) obj->y = obj_y->valuedouble;
            if (obj_width) obj->width = obj_width->valuedouble;
            if (obj_height) obj->height = obj_height->valuedouble;
            if (obj_point) obj->point = cJSON_IsTrue(obj_point);
            if (obj_rotation) obj->rotation = obj_rotation->valuedouble;
            if (obj_visible) obj->visible = cJSON_IsTrue(obj_visible);
            else obj->visible = true;
        }
    }
}

// Fonction pour charger les layers d'un groupe
static void loadLayerGroup(LayerGroup *group, cJSON *group_json) {
    cJSON *id_json = cJSON_GetObjectItem(group_json, "id");
    cJSON *name_json = cJSON_GetObjectItem(group_json, "name");
    cJSON *layers_json = cJSON_GetObjectItem(group_json, "layers");
    cJSON *visible_json = cJSON_GetObjectItem(group_json, "visible");
    cJSON *opacity_json = cJSON_GetObjectItem(group_json, "opacity");
    cJSON *x_json = cJSON_GetObjectItem(group_json, "x");
    cJSON *y_json = cJSON_GetObjectItem(group_json, "y");
    
    if (id_json) group->id = id_json->valueint;
    if (name_json) group->name = strdup(name_json->valuestring);
    if (visible_json) group->visible = cJSON_IsTrue(visible_json);
    else group->visible = true;
    if (opacity_json) group->opacity = opacity_json->valuedouble;
    else group->opacity = 1.0f;
    if (x_json) group->x = x_json->valueint;
    if (y_json) group->y = y_json->valueint;
    
    if (layers_json && cJSON_IsArray(layers_json)) {
        int layer_count = cJSON_GetArraySize(layers_json);
        
        // Compter les différents types de layers
        int tile_count = 0, object_count = 0;
        for (int i = 0; i < layer_count; i++) {
            cJSON *layer_json = cJSON_GetArrayItem(layers_json, i);
            cJSON *type_json = cJSON_GetObjectItem(layer_json, "type");
            if (type_json) {
                if (strcmp(type_json->valuestring, "tilelayer") == 0) tile_count++;
                else if (strcmp(type_json->valuestring, "objectgroup") == 0) object_count++;
            }
        }
        
        // Allouer et charger les layers
        if (tile_count > 0) {
            group->tile_layers = calloc(tile_count, sizeof(TileLayer));
            group->tile_layer_count = 0;
        }
        if (object_count > 0) {
            group->object_layers = calloc(object_count, sizeof(ObjectLayer));
            group->object_layer_count = 0;
        }
        
        for (int i = 0; i < layer_count; i++) {
            cJSON *layer_json = cJSON_GetArrayItem(layers_json, i);
            cJSON *type_json = cJSON_GetObjectItem(layer_json, "type");
            if (type_json) {
                if (strcmp(type_json->valuestring, "tilelayer") == 0) {
                    loadTileLayer(&group->tile_layers[group->tile_layer_count++], layer_json);
                } else if (strcmp(type_json->valuestring, "objectgroup") == 0) {
                    loadObjectLayer(&group->object_layers[group->object_layer_count++], layer_json);
                }
            }
        }
    }
}

Map* loadMap(const char *filename, SDL_Renderer *renderer) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur ouverture fichier: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *json_string = malloc(file_size + 1);
    fread(json_string, 1, file_size, file);
    json_string[file_size] = '\0';
    fclose(file);
    
    cJSON *json = cJSON_Parse(json_string);
    free(json_string);
    
    if (!json) {
        fprintf(stderr, "Erreur parsing JSON\n");
        return NULL;
    }
    
    Map *map = calloc(1, sizeof(Map));
    
    // Charger les propriétés de base
    cJSON *width_json = cJSON_GetObjectItem(json, "width");
    cJSON *height_json = cJSON_GetObjectItem(json, "height");
    cJSON *tilewidth_json = cJSON_GetObjectItem(json, "tilewidth");
    cJSON *tileheight_json = cJSON_GetObjectItem(json, "tileheight");
    
    if (width_json) map->width = width_json->valueint;
    if (height_json) map->height = height_json->valueint;
    if (tilewidth_json) map->tilewidth = tilewidth_json->valueint;
    if (tileheight_json) map->tileheight = tileheight_json->valueint;
    
    // Charger les tilesets
    cJSON *tilesets_json = cJSON_GetObjectItem(json, "tilesets");
    if (tilesets_json) {
        loadTilesets(map, tilesets_json, filename, renderer);
    }
    
    // Charger les layers
    cJSON *layers_json = cJSON_GetObjectItem(json, "layers");
    if (layers_json && cJSON_IsArray(layers_json)) {
        int layer_count = cJSON_GetArraySize(layers_json);
        
        // Compter les différents types
        int group_count = 0, tile_count = 0, object_count = 0;
        for (int i = 0; i < layer_count; i++) {
            cJSON *layer_json = cJSON_GetArrayItem(layers_json, i);
            cJSON *type_json = cJSON_GetObjectItem(layer_json, "type");
            if (type_json) {
                if (strcmp(type_json->valuestring, "group") == 0) group_count++;
                else if (strcmp(type_json->valuestring, "tilelayer") == 0) tile_count++;
                else if (strcmp(type_json->valuestring, "objectgroup") == 0) object_count++;
            }
        }
        
        // Allouer
        if (group_count > 0) {
            map->layer_groups = calloc(group_count, sizeof(LayerGroup));
            map->layer_group_count = 0;
        }
        if (tile_count > 0) {
            map->tile_layers = calloc(tile_count, sizeof(TileLayer));
            map->tile_layer_count = 0;
        }
        if (object_count > 0) {
            map->object_layers = calloc(object_count, sizeof(ObjectLayer));
            map->object_layer_count = 0;
        }
        
        // Charger
        for (int i = 0; i < layer_count; i++) {
            cJSON *layer_json = cJSON_GetArrayItem(layers_json, i);
            cJSON *type_json = cJSON_GetObjectItem(layer_json, "type");
            if (type_json) {
                if (strcmp(type_json->valuestring, "group") == 0) {
                    loadLayerGroup(&map->layer_groups[map->layer_group_count++], layer_json);
                } else if (strcmp(type_json->valuestring, "tilelayer") == 0) {
                    loadTileLayer(&map->tile_layers[map->tile_layer_count++], layer_json);
                } else if (strcmp(type_json->valuestring, "objectgroup") == 0) {
                    loadObjectLayer(&map->object_layers[map->object_layer_count++], layer_json);
                }
            }
        }
    }
    
    cJSON_Delete(json);
    return map;
}

Tileset* getTilesetForGID(Map *map, int gid) {
    if (gid == 0) return NULL;
    
    for (int i = map->tileset_count - 1; i >= 0; i--) {
        if (gid >= map->tilesets[i].firstgid) {
            return &map->tilesets[i];
        }
    }
    return NULL;
}

static void renderTileLayer(TileLayer *layer, Map *map, SDL_Renderer *renderer) {
    if (!layer->visible || !layer->data) return;
    
    for (int y = 0; y < layer->height; y++) {
        for (int x = 0; x < layer->width; x++) {
            int gid = layer->data[y * layer->width + x];
            if (gid == 0) continue;
            
            Tileset *tileset = getTilesetForGID(map, gid);
            if (!tileset || !tileset->texture) continue;
            
            int local_id = gid - tileset->firstgid;
            int src_x = (local_id % tileset->columns) * tileset->tilewidth;
            int src_y = (local_id / tileset->columns) * tileset->tileheight;
            
            SDL_Rect src_rect = {src_x, src_y, tileset->tilewidth, tileset->tileheight};
            SDL_Rect dst_rect = {
                (x * map->tilewidth) + layer->x,
                (y * map->tileheight) + layer->y,
                map->tilewidth,
                map->tileheight
            };
            
            if (layer->opacity < 1.0f) {
                SDL_SetTextureAlphaMod(tileset->texture, (Uint8)(layer->opacity * 255));
            }
            
            SDL_RenderCopy(renderer, tileset->texture, &src_rect, &dst_rect);
            
            if (layer->opacity < 1.0f) {
                SDL_SetTextureAlphaMod(tileset->texture, 255);
            }
        }
    }
}

void renderMap(Map *map, SDL_Renderer *renderer) {
    // Render direct tile layers
    for (int i = 0; i < map->tile_layer_count; i++) {
        renderTileLayer(&map->tile_layers[i], map, renderer);
    }
    
    // Render groups
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        if (!group->visible) continue;
        
        for (int i = 0; i < group->tile_layer_count; i++) {
            renderTileLayer(&group->tile_layers[i], map, renderer);
        }
    }
}

void renderMapToLayer(Map *map, SDL_Renderer *renderer, const char *layer_name) {
    // Chercher et rendre uniquement le layer demandé
    for (int i = 0; i < map->tile_layer_count; i++) {
        if (map->tile_layers[i].name && strcmp(map->tile_layers[i].name, layer_name) == 0) {
            renderTileLayer(&map->tile_layers[i], map, renderer);
            return;
        }
    }
    
    // Chercher dans les groupes
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        if (group->name && strcmp(group->name, layer_name) == 0) {
            if (group->visible) {
                for (int i = 0; i < group->tile_layer_count; i++) {
                    renderTileLayer(&group->tile_layers[i], map, renderer);
                }
            }
            return;
        }
        
        // Chercher dans les layers du groupe
        for (int i = 0; i < group->tile_layer_count; i++) {
            if (group->tile_layers[i].name && strcmp(group->tile_layers[i].name, layer_name) == 0) {
                renderTileLayer(&group->tile_layers[i], map, renderer);
                return;
            }
        }
    }
}

void renderMapBeforeLayer(Map *map, SDL_Renderer *renderer, const char *layer_name) {
    bool found = false;
    
    // Render direct layers jusqu'au layer demandé
    for (int i = 0; i < map->tile_layer_count; i++) {
        if (map->tile_layers[i].name && strcmp(map->tile_layers[i].name, layer_name) == 0) {
            found = true;
            break;
        }
        renderTileLayer(&map->tile_layers[i], map, renderer);
    }
    
    if (found) return;
    
    // Render groups jusqu'au layer demandé
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        if (group->name && strcmp(group->name, layer_name) == 0) {
            break;
        }
        
        if (group->visible) {
            bool group_found = false;
            for (int i = 0; i < group->tile_layer_count; i++) {
                if (group->tile_layers[i].name && strcmp(group->tile_layers[i].name, layer_name) == 0) {
                    group_found = true;
                    break;
                }
                renderTileLayer(&group->tile_layers[i], map, renderer);
            }
            if (group_found) break;
        }
    }
}

void renderMapAfterLayer(Map *map, SDL_Renderer *renderer, const char *layer_name) {
    bool found = false;
    
    // Chercher le layer et render après
    for (int i = 0; i < map->tile_layer_count; i++) {
        if (found) {
            renderTileLayer(&map->tile_layers[i], map, renderer);
        } else if (map->tile_layers[i].name && strcmp(map->tile_layers[i].name, layer_name) == 0) {
            found = true;
        }
    }
    
    if (found) return;
    
    // Chercher dans les groupes
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        
        if (group->name && strcmp(group->name, layer_name) == 0) {
            // Render les groupes suivants
            for (int next_g = g + 1; next_g < map->layer_group_count; next_g++) {
                LayerGroup *next_group = &map->layer_groups[next_g];
                if (next_group->visible) {
                    for (int i = 0; i < next_group->tile_layer_count; i++) {
                        renderTileLayer(&next_group->tile_layers[i], map, renderer);
                    }
                }
            }
            return;
        }
        
        if (group->visible) {
            for (int i = 0; i < group->tile_layer_count; i++) {
                if (found) {
                    renderTileLayer(&group->tile_layers[i], map, renderer);
                } else if (group->tile_layers[i].name && strcmp(group->tile_layers[i].name, layer_name) == 0) {
                    found = true;
                }
            }
        }
    }
}

MapObject* getObjectByName(Map *map, const char *name) {
    // Chercher dans les object layers directs
    for (int i = 0; i < map->object_layer_count; i++) {
        ObjectLayer *layer = &map->object_layers[i];
        for (int j = 0; j < layer->object_count; j++) {
            if (layer->objects[j].name && strcmp(layer->objects[j].name, name) == 0) {
                return &layer->objects[j];
            }
        }
    }
    
    // Chercher dans les groupes
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        for (int i = 0; i < group->object_layer_count; i++) {
            ObjectLayer *layer = &group->object_layers[i];
            for (int j = 0; j < layer->object_count; j++) {
                if (layer->objects[j].name && strcmp(layer->objects[j].name, name) == 0) {
                    return &layer->objects[j];
                }
            }
        }
    }
    
    return NULL;
}

MapObject* getObjectsByType(Map *map, const char *type, int *count) {
    *count = 0;
    
    // Compter d'abord le nombre d'objets du type demandé
    int total_count = 0;
    
    // Compter dans les object layers directs
    for (int i = 0; i < map->object_layer_count; i++) {
        ObjectLayer *layer = &map->object_layers[i];
        for (int j = 0; j < layer->object_count; j++) {
            if (layer->objects[j].type && strcmp(layer->objects[j].type, type) == 0) {
                total_count++;
            }
        }
    }
    
    // Compter dans les groupes
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        for (int i = 0; i < group->object_layer_count; i++) {
            ObjectLayer *layer = &group->object_layers[i];
            for (int j = 0; j < layer->object_count; j++) {
                if (layer->objects[j].type && strcmp(layer->objects[j].type, type) == 0) {
                    total_count++;
                }
            }
        }
    }
    
    if (total_count == 0) {
        return NULL;
    }
    
    // Allouer et remplir le tableau de résultats
    MapObject *results = malloc(total_count * sizeof(MapObject));
    int index = 0;
    
    // Copier depuis les object layers directs
    for (int i = 0; i < map->object_layer_count; i++) {
        ObjectLayer *layer = &map->object_layers[i];
        for (int j = 0; j < layer->object_count; j++) {
            if (layer->objects[j].type && strcmp(layer->objects[j].type, type) == 0) {
                results[index++] = layer->objects[j];
            }
        }
    }
    
    // Copier depuis les groupes
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        for (int i = 0; i < group->object_layer_count; i++) {
            ObjectLayer *layer = &group->object_layers[i];
            for (int j = 0; j < layer->object_count; j++) {
                if (layer->objects[j].type && strcmp(layer->objects[j].type, type) == 0) {
                    results[index++] = layer->objects[j];
                }
            }
        }
    }
    
    *count = total_count;
    return results;
}

void freeMap(Map *map) {
    if (!map) return;
    
    // Libérer les tilesets
    for (int i = 0; i < map->tileset_count; i++) {
        free(map->tilesets[i].name);
        free(map->tilesets[i].image_path);
        if (map->tilesets[i].texture) {
            SDL_DestroyTexture(map->tilesets[i].texture);
        }
    }
    free(map->tilesets);
    
    // Libérer les tile layers directs
    for (int i = 0; i < map->tile_layer_count; i++) {
        free(map->tile_layers[i].name);
        free(map->tile_layers[i].data);
    }
    free(map->tile_layers);
    
    // Libérer les object layers directs
    for (int i = 0; i < map->object_layer_count; i++) {
        free(map->object_layers[i].name);
        for (int j = 0; j < map->object_layers[i].object_count; j++) {
            free(map->object_layers[i].objects[j].name);
            free(map->object_layers[i].objects[j].type);
        }
        free(map->object_layers[i].objects);
    }
    free(map->object_layers);
    
    // Libérer les groupes
    for (int g = 0; g < map->layer_group_count; g++) {
        LayerGroup *group = &map->layer_groups[g];
        free(group->name);
        
        // Libérer les tile layers du groupe
        for (int i = 0; i < group->tile_layer_count; i++) {
            free(group->tile_layers[i].name);
            free(group->tile_layers[i].data);
        }
        free(group->tile_layers);
        
        // Libérer les object layers du groupe
        for (int i = 0; i < group->object_layer_count; i++) {
            free(group->object_layers[i].name);
            for (int j = 0; j < group->object_layers[i].object_count; j++) {
                free(group->object_layers[i].objects[j].name);
                free(group->object_layers[i].objects[j].type);
            }
            free(group->object_layers[i].objects);
        }
        free(group->object_layers);
    }
    free(map->layer_groups);
    
    free(map);
}
    
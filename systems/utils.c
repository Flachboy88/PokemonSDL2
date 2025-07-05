#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* buildFilePath(const char* dir, const char* name, const char* ext) {
    size_t len = strlen(dir) + strlen(name) + strlen(ext) + 1;
    char* full_path = malloc(len);
    if (!full_path) {
        fprintf(stderr, "Erreur d'allocation mémoire dans buildFilePath\n");
        return NULL;
    }
    snprintf(full_path, len, "%s%s%s", dir, name, ext);
    return full_path;
}
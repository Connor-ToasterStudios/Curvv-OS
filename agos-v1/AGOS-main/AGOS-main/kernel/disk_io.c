#include "disk_io.h"
#include <string.h>

#define MAX_FILES 256
#define MAX_FILE_SIZE 4096

typedef struct {
    char name[32];
    size_t size;
    uint8_t data[MAX_FILE_SIZE];
} File;

static File files[MAX_FILES];
static int file_count = 0;

void disk_save(const char* filename, const void* data, size_t size) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            files[i].size = size;
            memcpy(files[i].data, data, size);
            return;
        }
    }
    
    if (file_count < MAX_FILES) {
        strcpy(files[file_count].name, filename);
        files[file_count].size = size;
        memcpy(files[file_count].data, data, size);
        file_count++;
    }
}

void disk_load(const char* filename, void* buffer, size_t size) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            memcpy(buffer, files[i].data, size < files[i].size ? size : files[i].size);
            return;
        }
    }
    memset(buffer, 0, size);
}
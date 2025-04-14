#ifndef DISK_IO_H
#define DISK_IO_H

#include <stddef.h>

void disk_save(const char* filename, const void* data, size_t size);
void disk_load(const char* filename, void* buffer, size_t size);

#endif
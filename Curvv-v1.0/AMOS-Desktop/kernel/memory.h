#ifndef MEMORY_H
#define MEMORY_H

void* kmalloc(unsigned int size);
void kfree(void* ptr);
void* krealloc(void* ptr, unsigned int size);
unsigned int get_free_memory();
unsigned int get_total_memory();

#endif /* MEMORY_H */

#ifndef MULTITASK_H
#define MULTITASK_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_TASKS 32
#define STACK_SIZE 4096

typedef struct {
    uint64_t rsp;
    void* stack;
    bool active;
    void (*entry)(void*);
    void* arg;
} Task;

void multitask_init(void);
void create_task(void (*entry)(void*), void* stack, void* arg);
void yield(void);
void schedule(void);

#endif
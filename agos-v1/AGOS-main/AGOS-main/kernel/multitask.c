#include "multitask.h"
#include <stdlib.h>

#define MAX_TASKS 8
#define STACK_SIZE 4096

typedef struct {
    void* stack;
    void (*entry)(void*);
    void* arg;
    bool active;
    uint64_t rsp;
} Task;

static Task tasks[MAX_TASKS];
static int current_task = 0;
static int task_count = 0;

void multitask_init(void) {
    task_count = 0;
    current_task = 0;
}

void create_task(void (*entry)(void*), void* stack, void* arg) {
    if (task_count >= MAX_TASKS) return;

    int id = task_count++;
    tasks[id].stack = stack;
    tasks[id].entry = entry;
    tasks[id].arg = arg;
    tasks[id].active = true;

    uint64_t* sp = (uint64_t*)((char*)stack + STACK_SIZE);
    *(--sp) = (uint64_t)arg;
    *(--sp) = (uint64_t)entry;
    tasks[id].rsp = (uint64_t)sp;
}

void schedule(void) {
    if (task_count == 0) return;

    int next = (current_task + 1) % task_count;
    while (next != current_task) {
        if (tasks[next].active) {
            current_task = next;
            return;
        }
        next = (next + 1) % task_count;
    }
}

// Placeholder for context switching.  Needs actual implementation.
extern void switch_to_task(int id);


void pit_timer_handler() {
    schedule();
    switch_to_task(current_task);
}
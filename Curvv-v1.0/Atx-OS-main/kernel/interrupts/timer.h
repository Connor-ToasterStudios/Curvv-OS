#pragma once

#include "utils.h"
typedef struct QueuedPrint {
    ui8 newline; // This is supposed to be a boolean value
    const char* message;
    ui64 tick;
    int x;
    int y;
} QueuedPrint;
typedef struct QueuedFunc {
    void (*func)();
    ui64 tick;
} QueuedFunc;
// Do not use these values directly! Instead use the functions queuePrint and queueFunc 
QueuedPrint _printQueue[128];
QueuedFunc _funcQueue[128];
ui64 _printQueueSize = 0;
ui64 _funcQueueSize = 0;
ui64 _smallestPqTick = 0xffffffffffffffff;
ui64 _smallestFqTick = 0xffffffffffffffff;
//
QueuedPrint QueuedPrint_ctor(ui8 newline, const char* message, ui64 tick) {
    QueuedPrint ret;
    ret.message = message;
    ret.newline = newline;
    ret.tick = tick;
    ret.x = -1;
    ret.y = -1;
    return ret;
}
QueuedPrint QueuedPrint_ctorxy(ui8 newline, const char* message, ui64 tick, int x, int y) {
    QueuedPrint ret;
    ret.message = message;
    ret.newline = newline;
    ret.tick = tick;
    ret.x = x;
    ret.y = y;
    return ret;
}
QueuedFunc QueuedFunc_ctor(void (*func)(), ui64 tick) {
    QueuedFunc ret;
    ret.func = func;
    ret.tick = tick;
    return ret;
}
void queuePrint(QueuedPrint qv) {
    _printQueue[_printQueueSize] = qv;
    _printQueueSize++;
    if (qv.tick < _smallestPqTick) {
        _smallestPqTick = qv.tick;
    }
}
void queueFunc(QueuedFunc qv) {
    _funcQueue[_funcQueueSize] = qv;
    _funcQueueSize++;
    if (qv.tick < _smallestFqTick) {
        _smallestFqTick = qv.tick;
    }
}
void timerUpdate(int tick) {
    if (tick >= _smallestPqTick) {
        for (int i = 0; i < _printQueueSize; i++) {
            QueuedPrint qp = _printQueue[i];
            if (tick == qp.tick) {
                _smallestPqTick = qp.tick;
                if (qp.x < 0 || qp.y < 0) {
                    print(qp.message);
                } else {
                    printat(qp.message, qp.x, qp.y);
                }
                if (qp.newline) {
                    printnl();
                }
                for (int j = i + 1; j < _printQueueSize; j++) {
                    _printQueue[j - 1] = _printQueue[j];
                }
                _printQueueSize--;
            }
        }
    }
    if (tick >= _smallestFqTick) {
        for (int i = 0; i < _funcQueueSize; i++) {
            QueuedFunc qf = _funcQueue[i];
            if (tick == qf.tick) {
                _smallestFqTick = qf.tick;
                qf.func();
                for (int j = i + 1; j < _funcQueueSize; j++) {
                    _funcQueue[j - 1] = _funcQueue[j];
                }
                _funcQueueSize--;
            }
        }
    }
}
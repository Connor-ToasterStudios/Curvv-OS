#include "interrupts/idt.h"
#include "utils.h"
#include "interrupts/timer.h"
#include "amosstartascii.h"
// Kernel entry point
int main() {
    printl("Hello from C");
    initIdt();
    while (1);
}
#include "utils.h"
#include "timer.h"
void irqHandlerCommon(int irq) {
    if (irq >= 8) {
        outPort(0xA0, 0x20); // Output EOI to the slave PIC
    }
    outPort(0x20, 0x20); // Output EOI to the master PIC
}
void irqHandler0() {
    tick++;
    timerUpdate(tick);
    printat("Tick: ", 0, 24);
    printintat(tick, 6, 24);
    irqHandlerCommon(0);
}
void irqHandler1() {
    ui8 sc = inPort(0x60);
    char ch = getCharFromScancode(sc);
    if (ch != '\0') {
        putchar(ch);
    }
    irqHandlerCommon(1);
}
void irqHandler2() {
    printl("IRQ 2 occured");
    irqHandlerCommon(2);
}
void irqHandler3() {
    printl("IRQ 3 occured");
    irqHandlerCommon(3);
}
void irqHandler4() {
    printl("IRQ 4 occured");
    irqHandlerCommon(4);
}
void irqHandler5() {
    printl("IRQ 5 occured");
    irqHandlerCommon(5);
}
void irqHandler6() {
    printl("IRQ 6 occured");
    irqHandlerCommon(6);
}
void irqHandler7() {
    printl("IRQ 7 occured");
    irqHandlerCommon(7);
}
void irqHandler8() {
    printl("IRQ 8 occured");
    irqHandlerCommon(8);
}
void irqHandler9() {
    printl("IRQ 9 occured");
    irqHandlerCommon(9);
}
void irqHandler10() {
    printl("IRQ 10 occured");
    irqHandlerCommon(10);
}
void irqHandler11() {
    printl("IRQ 11 occured");
    irqHandlerCommon(11);
}
void irqHandler12() {
    printl("IRQ 12 occured");
    irqHandlerCommon(12);
}
void irqHandler13() {
    printl("IRQ 13 occured");
    irqHandlerCommon(13);
}
void irqHandler14() {
    printl("IRQ 14 occured");
    irqHandlerCommon(14);
}
void irqHandler15() {
    printl("IRQ 15 occured");
    irqHandlerCommon(15);
}

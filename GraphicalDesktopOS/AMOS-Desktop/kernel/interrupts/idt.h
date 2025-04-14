#pragma once

#include "types.h"
#include "utils.h"
#include "idthandlers.h"

#define GDT_CODE_SEG_SELECTOR 0x8 //Can be represented as: GDT Index 1 (first 13 bits, note that index zero is the null header), with a TI of 0 (use GDT instead of LDT, 14th bit), and a privilege level of 0 (highest level, 15th and 16th bit)
typedef struct __attribute__((packed)) IntDesc32 {
    ui16 offset_low;
    ui16 selector;
    ui8 empty; //Unused space
    ui8 flags;
    ui16 offset_high;
} IntDesc32;
typedef struct __attribute__((packed)) IntDesc64 {
    ui16 offset1;
    ui16 selector;
    ui8 empty1; //Unused space (Note that this is not actually empty if an Interrupt Stack Table is used)
    ui8 flags;
    ui16 offset2;
    ui32 offset3;
    ui32 empty2; //More unused space
} IntDesc64;
typedef struct __attribute__((packed)) IdtDesc {
    ui16 size;
    ptr base;
} IdtDesc;
#define IDT_INTERRUPTS_SIZE 256
IntDesc64 interrupts[IDT_INTERRUPTS_SIZE];
IdtDesc idtDescriptor;

extern void loadIdt(ptr descriptor);
extern void remapPic();
void setInterrupt(int idx, ptr loc) {
    unsigned long long mask = 0b0000000000000000000000000000000000000000000000001111111111111111; //Yes I know this is ridiculous
    interrupts[idx].offset1 = loc & (mask << 0); //Mask out the first 16 bits
    interrupts[idx].selector = GDT_CODE_SEG_SELECTOR; //You can read about this where the macro is defined
    interrupts[idx].empty1 = 0; //Empty
    interrupts[idx].flags = 0b10001110; //Can be represented as: Present (bit 1), Highest privilege level (bits 2 and 3), Empty (bit 4), 64 bit interrupt (bits 5-8)
    interrupts[idx].offset2 = (loc & (mask << 16)) >> 16; //Mask out the next 16 bits
    interrupts[idx].offset3 = (loc & (mask << 32 | mask << 48)) >> 32; //Mask out the last 32 bits
    interrupts[idx].empty2 = 0; //More empty
}
//This section is the assembly external handlers, generated with python
extern void asm_inthandler0();
extern void asm_inthandler1();
extern void asm_inthandler2();
extern void asm_inthandler3();
extern void asm_inthandler4();
extern void asm_inthandler5();
extern void asm_inthandler6();
extern void asm_inthandler7();
extern void asm_inthandler8();
extern void asm_inthandler9();
extern void asm_inthandler10();
extern void asm_inthandler11();
extern void asm_inthandler12();
extern void asm_inthandler13();
extern void asm_inthandler14();
extern void asm_inthandler15();
extern void asm_inthandler16();
extern void asm_inthandler17();
extern void asm_inthandler18();
extern void asm_inthandler19();
extern void asm_inthandler20();
extern void asm_inthandler21();
extern void asm_inthandler22();
extern void asm_inthandler23();
extern void asm_inthandler24();
extern void asm_inthandler25();
extern void asm_inthandler26();
extern void asm_inthandler27();
extern void asm_inthandler28();
extern void asm_inthandler29();
extern void asm_inthandler30();
extern void asm_inthandler31();
extern void asm_inthandler32();
extern void asm_inthandler33();
extern void asm_inthandler34();
extern void asm_inthandler35();
extern void asm_inthandler36();
extern void asm_inthandler37();
extern void asm_inthandler38();
extern void asm_inthandler39();
extern void asm_inthandler40();
extern void asm_inthandler41();
extern void asm_inthandler42();
extern void asm_inthandler43();
extern void asm_inthandler44();
extern void asm_inthandler45();
extern void asm_inthandler46();
extern void asm_inthandler47();
//----------------------------------------------------------------------
void assignInterrupts() {
    // This code was generated with the simple python script:
    // for i in range(48):
    //     print("setInterrupt("+str(i)+",(ptr)asm_inthandler"+str(i)+");")

    //PM exceptions
    setInterrupt(0,(ptr)asm_inthandler0);
    setInterrupt(1,(ptr)asm_inthandler1);
    setInterrupt(2,(ptr)asm_inthandler2);
    setInterrupt(3,(ptr)asm_inthandler3);
    setInterrupt(4,(ptr)asm_inthandler4);
    setInterrupt(5,(ptr)asm_inthandler5);
    setInterrupt(6,(ptr)asm_inthandler6);
    setInterrupt(7,(ptr)asm_inthandler7);
    setInterrupt(8,(ptr)asm_inthandler8);
    setInterrupt(9,(ptr)asm_inthandler9);
    setInterrupt(10,(ptr)asm_inthandler10);
    setInterrupt(11,(ptr)asm_inthandler11);
    setInterrupt(12,(ptr)asm_inthandler12);
    setInterrupt(13,(ptr)asm_inthandler13);
    setInterrupt(14,(ptr)asm_inthandler14);
    setInterrupt(15,(ptr)asm_inthandler15);
    setInterrupt(16,(ptr)asm_inthandler16);
    setInterrupt(17,(ptr)asm_inthandler17);
    setInterrupt(18,(ptr)asm_inthandler18);
    setInterrupt(19,(ptr)asm_inthandler19);
    setInterrupt(20,(ptr)asm_inthandler20);
    setInterrupt(21,(ptr)asm_inthandler21);
    setInterrupt(22,(ptr)asm_inthandler22);
    setInterrupt(23,(ptr)asm_inthandler23);
    setInterrupt(24,(ptr)asm_inthandler24);
    setInterrupt(25,(ptr)asm_inthandler25);
    setInterrupt(26,(ptr)asm_inthandler26);
    setInterrupt(27,(ptr)asm_inthandler27);
    setInterrupt(28,(ptr)asm_inthandler28);
    setInterrupt(29,(ptr)asm_inthandler29);
    setInterrupt(30,(ptr)asm_inthandler30);
    setInterrupt(31,(ptr)asm_inthandler31);
    // IRQs
    setInterrupt(32,(ptr)asm_inthandler32);
    setInterrupt(33,(ptr)asm_inthandler33);
    setInterrupt(34,(ptr)asm_inthandler34);
    setInterrupt(35,(ptr)asm_inthandler35);
    setInterrupt(36,(ptr)asm_inthandler36);
    setInterrupt(37,(ptr)asm_inthandler37);
    setInterrupt(38,(ptr)asm_inthandler38);
    setInterrupt(39,(ptr)asm_inthandler39);
    setInterrupt(40,(ptr)asm_inthandler40);
    setInterrupt(41,(ptr)asm_inthandler41);
    setInterrupt(42,(ptr)asm_inthandler42);
    setInterrupt(43,(ptr)asm_inthandler43);
    setInterrupt(44,(ptr)asm_inthandler44);
    setInterrupt(45,(ptr)asm_inthandler45);
    setInterrupt(46,(ptr)asm_inthandler46);
    setInterrupt(47,(ptr)asm_inthandler47);
}
void initIdt() {
    remapPic();
    assignInterrupts();
    idtDescriptor.size = IDT_INTERRUPTS_SIZE * sizeof(IntDesc64) - 1;
    idtDescriptor.base = (ptr)interrupts;
    loadIdt((ptr)(&idtDescriptor));
}
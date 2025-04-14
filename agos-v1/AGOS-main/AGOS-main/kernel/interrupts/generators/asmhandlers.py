import os

os.chdir(__file__[:(__file__.rfind('\\'))])
output = ""
def addline(line = ""):
    global output
    output += line + "\n"
addline("; This was generated with the file generators/asmhandlers.py")
for i in range(48):
    addline("global asm_inthandler" + str(i))
for i in range(32):
    addline("extern exceptionHandler" + str(i))
for i in range(16):
    addline("extern irqHandler" + str(i))
for i in range(32):
    addline("asm_inthandler" + str(i) + ":")
    addline("cli")
    addline("call exceptionHandler" + str(i))
    #addline("add rsp, 16")
    addline("sti")
    addline("iretq")
for i in range(16):
    addline("asm_inthandler" + str(i + 32) + ":")
    addline("cli")
    addline("call irqHandler" + str(i))
    #addline("add rsp, 16")
    addline("sti")
    addline("iretq")
open("../idthandlers.s", "w").write(output)
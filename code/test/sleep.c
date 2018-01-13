#include "syscall.h"
main() {
    int i;
    for(i = 0; i < 5; i++) {
        Sleep(10000);
        PrintInt(50);
    }
    return 0;
}
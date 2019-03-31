#include "kernel.h"

void main() {
    int idx, i;
    char curdir;
    char argc;
    char argv[4][16];
    int succ;
    char chain[MAX_DIRS];
    int n = 0;
    char buff[MAX_FILENAME + 1];
    char dirs[SECTOR_SIZE];
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    idx = 0;
    while (idx < argc){
        interrupt(0x21, 0x23, idx, argv[idx], 0);
        idx++;
    }
    interrupt(0x21, 0x02, dirs, DIRS_SECTOR);

    while (curdir != 0xFF) {
        chain[n] = curdir;
        n++;
        curdir = dirs[curdir * ENTRY_LENGTH];
    }
    interrupt(0x21, 0x0, "/", 0, 0);
    for (idx = n - 1; idx >= 0; idx--) {
        i = 0;
        while ((dirs[chain[idx] * ENTRY_LENGTH + 1 + i] != '\0') && (i < MAX_FILENAME)) {
            buff[i] = dirs[chain[idx] * ENTRY_LENGTH + 1 + i];
            i++;
        }
        buff[i] = '\0';
        interrupt(0x21, 0x0, buff, 0, 0);
        interrupt(0x21, 0x0, "/", 0, 0);
    }
    interrupt(0x21, 0x0, "\n", 0, 0);
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}
#include "kernel.h"

char cmpArray(char * arr1, char * arr2, int length);

void main() {
    int idx;
    char curdir;
    char argc;
    char argv[4][32];
    int succ;
    char buff[MAX_FILENAME + 1];
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    idx = 0;
    while(idx < argc){
        interrupt(0x21, 0x23, idx, argv[idx], 0);
        idx++;
    }
    if (argc > 0) {
        // Try deleting file.
        interrupt(0x21, (curdir << 8) | 0x09, argv[0], &succ, 0);
        if (succ != SUCCESS) {
            // Try deleting directory.
            interrupt(0x21, (curdir << 8) | 0x0A, argv[0], &succ, 0);
        }
        if (succ != SUCCESS){
            interrupt(0x21, 0x0, "Unable to delete.\n", 0, 0);
        }
        else if (succ == NOT_FOUND) {
            interrupt(0x21, 0x0, "File not found.\n", 0, 0);
        }
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}
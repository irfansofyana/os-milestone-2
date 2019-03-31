#include "kernel.h"

char cmpArray(char * arr1, char * arr2, int length);
int div(int a, int b);

void main() {
    int idx, i, j;
    char curdir;
    char argc;
    char argv[4][32];
    int succ;
    char buff[SECTOR_SIZE * MAX_SECTORS];
    char l_buffer[128];
    int sectors;
    char mode = 0;
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    idx = 0;
    while(idx < argc){
        interrupt(0x21, 0x23, idx, argv[idx], 0);
        idx++;
    }
    if (argc > 0) {
        if (argc > 1) {
            if (cmpArray("-w", argv[1], 2)) {
                mode = 1;
            }
        }
        if (mode == 0) {
            interrupt(0x21, (curdir << 8) | 0x04, buff, argv[0], &succ);
            if (succ == SUCCESS) {
                interrupt(0x21, 0x0, buff, 0, 0);
                interrupt(0x21, 0x0, "\n", 0, 0);
            } else {
                interrupt(0x21, 0x0, "File not found\n", 0, 0);
            }
        } else if (mode == 1) {
            for (idx = 0; idx < SECTOR_SIZE * MAX_SECTORS; idx++) {
                buff[idx] = '\0';
            }
            interrupt(0x21, 0x0, "Press Enter to End.\n", 0, 0);
            j = 0;
            interrupt(0x21, 0x01, l_buffer, 0, 0);
            while (l_buffer[0] != '\0') {
                i = 0;
                while (l_buffer[i] != '\0') {
                    buff[j] = l_buffer[i];
                    i++;
                    j++;
                }
                buff[j] = '\n';
                j++;
                interrupt(0x21, 0x01, l_buffer, 0, 0);
            }
            buff[j - 1] = '\0';
            interrupt(0x21, 0x0, "Saving File.\n", 0, 0);
            sectors = div(j, SECTOR_SIZE) + 1;
            interrupt(0x21, (curdir << 8) | 0x05, buff, argv[0], &sectors);
        }
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}

int div(int a, int b) {
    int q = 0;
    while(q*b <=a) {
        q = q+1;
    }
    return q-1;
}

char cmpArray(char * arr1, char * arr2, int length) {
    int i = 0;
    char equal = TRUE;
    while ((i < length) && (equal)) {
        equal = arr1[i] == arr2[i];
        if (equal) {
            if (arr1[i] == '\0') {
                i = length;
            }
        }
        i++;
    }
    return equal;
}
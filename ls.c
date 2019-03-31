#include "kernel.h"

void main(){
    char directory[SECTOR_SIZE];
    char file[SECTOR_SIZE];
    char curdir;
    int idx;
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x02, directory, DIRS_SECTOR, 0);
    interrupt(0x21, 0x02, file, FILES_SECTOR, 0);
    interrupt(0x21, 0x00, "List File : \r", 0, 0);
    for(idx=0;idx<MAX_FILES;idx++){
        if(file[idx*DIR_ENTRY_LENGTH+1] != '\0' && file[idx*DIR_ENTRY_LENGTH] == curdir ){
            interrupt(0x21, 0x00, file+(idx*DIR_ENTRY_LENGTH+1), 0, 0);
            interrupt(0x21, 0x00, "\r", 0, 0);
        }
    }
    interrupt(0x21, 0x00, "List Folder :\r\n", 0, 0);
    for(idx=0;idx<MAX_FILES;idx++){
        if(directory[idx*DIR_ENTRY_LENGTH+1] != '\0' && directory[idx*DIR_ENTRY_LENGTH] == curdir){
            interrupt(0x21, 0x00, directory+(idx*DIR_ENTRY_LENGTH+1), 0, 0);
            interrupt(0x21, 0x00, "\r\n", 0, 0);
        }
    }
    interrupt(0x21, 0x07, &idx, 0, 0);
}
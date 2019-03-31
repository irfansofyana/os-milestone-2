#include "kernel.h"

int main() {
    int succ = 0;
    char buffer[MAX_SECTORS * SECTOR_SIZE];
    int x;
    int y;
    char curdir = 0xFF; // root
    char argc = 0;
    char *argv[2];
    int i = 0;
    int j = 0;
    makeInterrupt21();
    
    while (i <= 14){
        if (j == 80){
            j = 0;
            i++;
        }
        putInMemory(0xB000, 0x8000 + ((80*i + j-1)*2), ' ' );
        putInMemory(0xB000, 0x8001 + ((80*i + j-1)*2), 0x3 );
        j++;
    }

    // Set default args.
    interrupt(0x21, 0x20, curdir, argc, argv);
    // Calls shell.
    interrupt(0x21, 0xFF << 8 | 0x6, "shell", 0x2000, &succ);
    while (1);  
}
void handleInterrupt21 (int AX, int BX, int CX, int DX) {
    char AL, AH;
    int i = 0;
    int * p;
    char d;
    AL = (char) (AX);
    AH = (char) (AX >> 8);
    switch (AL) {
        case 0x00:
            printString(BX);
            break;
        case 0x01:
            readString(BX);
            break;
        case 0x02:
            readSector(BX, CX);
            break;
        case 0x03:
            writeSector(BX, CX);
            break;
        case 0x04:
            readFile(BX, CX, DX, AH);
            break;
        case 0x05:
            writeFile(BX, CX, DX, AH);
            break;
        case 0x06:
            executeProgram(BX, CX, DX, AH);
            break;
        case 0x07:
            terminateProgram(BX);
            break;
        case 0x08:
            makeDirectory(BX, CX, AH);
            break;
        case 0x09:
            deleteFile(BX, CX, AH);
            break;
        case 0x0A:
            deleteDirectory(BX, CX, AH);
            break;
        case 0x20:
            putArgs(BX, CX, DX);
            break;
        case 0x21:
            getCurdir(BX);
            break;
        case 0x22:
            getArgc(BX);
            break;
        case 0X23:
            getArgv(BX, CX);
            break;
        case 0X90:
            findDir(&AH, &d, BX, &i, CX);
            p = (int *) CX;
            if (*p == SUCCESS) {
                *p = 1;
            } else {
                *p = 0;
            }
            p = (int *) DX;
            *p = d;
            break;
        case 0X91:
            findFile(&AH, &d, BX, &i, CX);
            p = (int *) CX;
            if (*p == SUCCESS) {
                *p = 1;
            } else {
                *p = 0;
            }
            p = (int *) DX;
            *p = d;
            break;
        default:
            printString("Invalid interrupt");
    }
}

void printString(char *string) {
    while (*string != '\0') {
        if ((*string) == '\r') {
            interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
        } else if ((*string) == '\n') {
            interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
        }
        interrupt(0x10, 0xE00 + (*string), 0, 0, 0);
        string++;
    }
}

void readString(char *string) {
    char reading = TRUE;
    int count = 0;
    while (reading) {
        char c = interrupt(0x16, 0, 0, 0, 0);
        if (c == '\r') { // Return/Enter
            interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
            interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
            (*string) = '\0';
            reading = FALSE;
        } else if (c == '\b') {
            if (count > 0) {
                interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
                interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
                interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
                (*string) = '\0';
                string--;
                count--;
            }
        } else {
            interrupt(0x10, 0xE00 + c, 0, 0, 0);
            (*string) = c;
            string++;
            count++;
        }
    }
}

int mod(int a, int b) {
    while(a >= b){
        a = a - b;
    }
    return a;
}

int div(int a, int b) {
    int q = 0;
    while(q*b <=a) {
        q = q+1;
    }
    return q-1;
}

void readSector(char *buffer, int sector) {
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector) {
    interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

char cmpArray(char * arr1, char * arr2, int length) {
    int idx = 0;
    char same = 1;
    while (idx < length && same){
        if (arr1[idx] == arr2[idx]) same = 1;
        else same = 0;
        if (same && arr1[idx] == '\0')
            idx = length;
        idx++;
    }
    return same;
}

void findFile(char * parent, char * current, char * filename, int * idx, int * result) {
    char name[MAX_FILENAME+3];
    char dir[SECTOR_SIZE];
    char file; char found = 0;
    int cnt = 0; int i = 0;
    if (filename[*idx] == '/')
        *idx++;
    for (i = 0; filename[*idx+i] != '/' && filename[*idx+i] != '\0'; i++)
        name[i] = filename[*idx + i];
    if (filename[*idx+i] == '\0')
        file = 1;
    else file = 0;
    name[i] = '\0';
    int j = i;
    if (!file){
        readSector(dir, DIRS_SECTOR);
        cnt = MAX_DIRS;
    }else{
        readSector(dir, FILES_SECTOR);
        cnt = MAX_FILES; 
    }

    i = 0;
    while ((i < cnt) && !found) {
        if ((dir[i * ENTRY_LENGTH] == *parent) && (cmpArray(name, dir + (i * ENTRY_LENGTH) + 1, MAX_FILENAME)))
            found = 1;
        else i++
    }
    
    if (found){
        *current = i;
        if (file)
            *result = SUCCESS;
        else{
            *parent = *current;
            *idx = *idx + j + 1;
            findFile(parent, current, filename, idx, result);
        }
    }else 
        *result = NOT_FOUND;
}

void findDir(char * parent, char * current, char * filename, int * idx, int * result) {
    char name[MAX_FILENAME+1];
    int i = 0;
    char end; char dir[SECTOR_SIZE];
    char j; char found=0;
    if (filename[*idx] == '/') *idx++;
    for (i = 0; filename[*idx+i] != '/' && filename[*idx+i] != '\0'; i++)
        name[i] = filename[*idx + i];
    if (filename[*idx + i] == '\0')
        end = 1;
    else end = 0;
    name[i] = '\0';
    int j = i;
    readSector(dir, DIRS_SECTOR);
    i = 0;
    while (i < MAX_DIRS && !found){
        int k = 0;
        if ((dir[i * ENTRY_LENGTH] == *parent) && (cmpArray(name, dir + (i * ENTRY_LENGTH) + 1, MAX_FILENAME))) {
            found = 1;
        } else {
            i++;
        }
    }
    if (found){
        *current = i;
        if (end)
            *result = SUCCESS;
        else{
            *idx = *idx + j + 1;
            *parent = *current;
            findDir(parent, current, filename, idx, result);
        }
    }else 
        *result = NOT_FOUND;
}   

void readFile(char *buffer, char *path, int *result, char parentIndex) {
    int i = 0;
    char current;
    char dir[SECTOR_SIZE];
    findFile(parentIndex, &current, path, &i, result);
    readSector(dir, SECTORS_SECTOR);

    if (*result == SUCCESS) {
        char processing = TRUE;
        char * sectors = dir + (current * ENTRY_LENGTH);
		for (i = 0; (i < MAX_SECTORS) && (processing == TRUE); i++){
            if (sectors[i] == 0) processing = FALSE;
			else readSector(buffer + i * SECTOR_SIZE, sectors[i]);
		}
    }
}

void clear(char *buffer, int length){
    int i;
    for(i = 0; i < length; ++i){
        buffer[i] = EMPTY;
    }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex) {
    int dirIndex;
    int i, j, sectorCount;
	
    char map[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    char sector[SECTOR_SIZE];
    char sectorBuffer[SECTOR_SIZE];

    readSector(map, MAP_SECTOR);
    readSector(files, FILES_SECTOR);
    readSector(sector, SECTORS_SECTOR);
	
	i = 0; sectorCount = 0;
	while ((i < MAX_BYTE) && (sectorCount < *sectors)) {
        if (map[i] == EMPTY)
            sectorCount++;
		i++;
    }
	
    if (sectorCount >= *sectors){
		dirIndex = 0;
		char found = FALSE;
        while ((dirIndex < MAX_FILES) && (found == FALSE)) {
            if (files[dirIndex * ENTRY_LENGTH + 1] == '\0')
				found = TRUE;
			else
				dirIndex++;
        }
        if (dirIndex < MAX_FILES) {
            char parent = parentIndex;
            char current;
            int result;
            j = 0;
            findFile(&parent, &current, path, &j, &result);
            if (result != SUCCESS) {
                char filename[MAX_FILENAME + 1];
                char offset = j;
                char file;
                for(; (path[j] != '\0') && (path[j] != '/'); j++)
                    filename[j - offset] = path[j];
                filename[j - offset] = '\0';
                file = path[j] == '\0';
                if (file) {
                    files[dirIndex * ENTRY_LENGTH] = parent;
                    for (j = 0; filename[j] != '\0'; j++)
                        files[dirIndex * ENTRY_LENGTH + 1 + j] = filename[j];
                    writeSector(files, FILES_SECTOR);
					i = 0; sectorCount = 0;
                    while (i < MAX_BYTE && sectorCount < *sectors) {
                        if (map[i] == EMPTY) {
                            map[i] = USED;
                            sector[dirIndex * ENTRY_LENGTH + sectorCount] = i;
                            clear(sectorBuffer, SECTOR_SIZE);
                            for (j = 0; j < SECTOR_SIZE; j++)
                                sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
                            writeSector(sectorBuffer, i);
                            sectorCount++;
                        }
						i++;
                    }
                    writeSector(map, MAP_SECTOR);
                    writeSector(sector, SECTORS_SECTOR);
                } else
                    *sectors = NOT_FOUND;
            } else
                *sectors = ALREADY_EXISTS;
        } else
            *sectors = INSUFFICIENT_ENTRIES;
    } else
        *sectors = INSUFFICIENT_SECTORS;

}

void executeProgram(char *path, int segment, int *result, char parentIndex) {
    char buffer[MAX_SECTORS * SECTOR_SIZE];
    readFile(buffer, path, result, parentIndex);
    if (*result == SUCCESS) {
        int i;
		for (i = 0; i < MAX_SECTORS * SECTOR_SIZE; i++)
            putInMemory(segment, i, buffer[i]);
        launchProgram(segment);
    }
}

void terminateProgram (int *result) {
    char shell[6];
    shell[0] = 's';
    shell[1] = 'h';
    shell[2] = 'e';
    shell[3] = 'l';
    shell[4] = 'l';
    shell[5] = '\0';
    executeProgram(shell, 0x2000, result, 0xFF);
}

void makeDirectory(char *path, int *result, char parentIndex) {
    char parentidx = parentIndex;
    int idx = 0;
    char current;
    findDir(&parentidx, &current, path, &idx, result);
    if (*result == SUCCESS) {
        *result = ALREADY_EXISTS;
    } else {
        char filename[MAX_FILENAME + 1];
        int i = 0;
        while ((path[idx + i] != '/') && (path[idx + i] != '\0')) {
            filename[i] = path[idx + i];
            i = i+1;
        }
        filename[i] = '\0';
        if (path[idx + i] == '/') {
            *result = NOT_FOUND;
        } else {
            char directory[SECTOR_SIZE];
            current = 0;
            readSector(directory, DIRS_SECTOR);
            while ((current < MAX_DIRS) && (directory[current * ENTRY_LENGTH + 1] != '\0')) {
                current = current+1;
            }
            if (current < MAX_DIRS) {
                directory[current * ENTRY_LENGTH] = parentidx;
                i = 0;
                while (filename[i] != '\0') {
                    directory[current * ENTRY_LENGTH + 1 + i] = filename[i];
                    i = i+1;
                }
                writeSector(directory, DIRS_SECTOR);
                *result = SUCCESS;
            } else {
                *result = INSUFFICIENT_ENTRIES;
            }
        }
    }
}

void deleteFileIndex(char current) {
    char file[SECTOR_SIZE];
    readSector(file, FILES_SECTOR);
    char sector[SECTOR_SIZE];
    readSector(sector, SECTORS_SECTOR);
    char map[SECTOR_SIZE];
    readSector(map, MAP_SECTOR);
    int idx;
    idx = 0
    // Set name to null terminator.
    file[current * ENTRY_LENGTH] = 0x00;
    file[current * ENTRY_LENGTH + 1] = '\0';
    while ((idx < MAX_SECTORS) && (sector[current * ENTRY_LENGTH + idx] != 0))
    {
        map[sector[current * ENTRY_LENGTH + idx]] = EMPTY;
        sector[current * ENTRY_LENGTH + idx] = 0;
        idx = idx +1;
    }
    writeSector(file, FILES_SECTOR);
    writeSector(sector, SECTORS_SECTOR);
    writeSector(map, MAP_SECTOR);
}

void deleteDirectoryIndex(char current) {
    char directory[SECTOR_SIZE];
    int idx=0;
    char files[SECTOR_SIZE];
    readSector(directory, DIRS_SECTOR);
    // Set name to null terminator.
    directory[current * ENTRY_LENGTH] = 0x00;
    directory[current * ENTRY_LENGTH + 1] = '\0';
    writeSector(directory, DIRS_SECTOR);
    readSector(files, FILES_SECTOR);
    while(idx<MAX_FILES){
        if ((files[idx * ENTRY_LENGTH] == current) && (files[idx * ENTRY_LENGTH + 1] != '\0')) {
            deleteFileIndex(idx);
        }
        idx++;
    }
    idx = 0;
    while (idx<MAX_DIRS){
        if ((directory[idx * ENTRY_LENGTH] == current) && (directory[idx * ENTRY_LENGTH + 1] != '\0')) {
            deleteDirectoryIndex(idx);
        }
        idx++;
    }
}

void deleteFile(char *path, int *result, char parentIndex) {
    char parent = parentIndex;
    int idx;
    idx = 0;
    char current;
    findFile(&parent, &current, path, &idx, result);
    if (*result == SUCCESS) {
        deleteFileIndex(current);
    }
}

void deleteDirectory(char *path, int *success, char parentIndex) {
    char parent = parentIndex;
    int idx;
    idx = 0;
    char current;
    findDir(&parent, &current, path, &idx, success);
    if (*success == SUCCESS) {
        deleteDirectoryIndex(current);
    }
}

void putArgs (char curdir, char argc, char **argv) {
    char args[SECTOR_SIZE];
    int i, j, p;
    clear(args, SECTOR_SIZE);
    args[0] = curdir;
    args[1] = argc;
    i = 0;
    j = 0;
    for (p = 2; p < ARGS_SECTOR && i < argc; ++p) {
        args[p] = argv[i][j];
        if (argv[i][j] == '\0') {
            ++i;
            j = 0;
        } else {
            ++j;
        }
    }
    writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *curdir = args[0];
}

void getArgc (char *argc) {
    char args[SECTOR_SIZE];
    readSector(args, ARGS_SECTOR);
    *argc = args[1];
}

void getArgv (char index, char *argv) {
    char args[SECTOR_SIZE];
    int i, j, p;
    readSector(args, ARGS_SECTOR);
    i = 0;
    j = 0;
    for (p = 2; p < ARGS_SECTOR; ++p) {
        if (i == index) {
            argv[j] = args[p];
            ++j;
        }
        if (args[p] == '\0') {
            if (i == index) {
                break;
            } else {
                ++i;
            }
        }
    }
}

#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 32
#define MAX_DIRS 32
#define MAX_FILENAME 15
#define ENTRY_LENGTH 16
#define MAX_SECTORS 16
#define MAP_SECTOR 0x100
#define DIRS_SECTOR 0x101
#define FILES_SECTOR 0x102
#define SECTORS_SECTOR 0x103
#define ARGS_SECTOR 512
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_ENTRIES -3
#define ALREADY_EXISTS -2
#define EMPTY 0x00
#define USED 0xFF
#define DIR_ROOT 0xFF
#define NOT_FOUND -1
#define SUCCESS 0
#define DIR_ENTRY_LENGTH 16

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
char cmpArray(char * arr1, char * arr2, int length);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *path, int segment, int *result, char parentIndex);
void terminateProgram (int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void putArgs (char curdir, char argc, char **argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);
void findDir(char * parent, char * current, char * filename, int * idx, int * result);
void findFile(char * parent, char * current, char * filename, int * idx, int * result);

#define INT_PRINTSTRING 0x00
#define INT_READSTRING 0x01
#define INT_READSECTOR 0x02
#define INT_WRITESECTOR 0x03
#define INT_READFILE 0x04
#define INT_WRITEFILE 0x05
#define INT_EXCPROGRAM 0x06
#define INT_TERMPROGRAM 0x07
#define INT_MAKEDIR 0x08
#define INT_DELFILE 0x09
#define INT_DELDIR 0x0A
#define INT_PUTARGS 0x20
#define INT_GETCWD 0x21
#define INT_GETARGC 0x22
#define INT_GETARGV 0X23
#define INT_FINDDIR 0X90
#define INT_FINDFILE 0X91

#include "kernel.h"

void main(){
	char argc;
	int argv[64];
	int idx;
	interrupt(0x21, 0x22, &argc, 0, 0);
	if(argc>1)
		argc = 1;
	if(argc == 1){
		interrupt(0x21, 0x23, 0, argv, 0);
		interrupt(0x21, 0x00, argv, 0, 0);
	}
	interrupt(0x21, 0x00, "\r\n", 0, 0);
	interrupt(0x21, 0x07, &idx, 0, 0);
}
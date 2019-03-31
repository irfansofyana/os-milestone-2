#define SECTOR_SIZE 512
#define DIRS_SECTOR 0x101
#define SUCCESS 0
#define NOT_FOUND -1

int strncmp(char *a, char *b, int len);
void handleArg(char *input);

void main(){
	char curDir, useDir;
	char input[512];
	char directory[SECTOR_SIZE];
	int valid=1;
	int offset;
	int se[4], idx;
	interrupt(0x21, 0x21, &curDir, 0, 0);
	// read input
	interrupt(0x21, 0x00, "$ ", 0, 0);
	interrupt(0x21, 0x01, input, 0, 0);
	interrupt(0x21, 0x00, "\r\n", 0, 0);

	if(strncmp(input,"cd", 2)){
		useDir = curDir;
		offset = 2;
		while(input[offset]==' ')
			offset++;
		if(input[offset]==0){
			interrupt(0x21, 0x20, 0xFF, 0, 0);
		}
		else if(input[2] == ' '){
			interrupt(0x21, 0x30, input+offset, se, &useDir);
			if(se[2] == NOT_FOUND){
				interrupt(0x21, 0x00, "ERROR! Directory Not Found.\r\n", 0, 0);
			}
			else{
				interrupt(0x21, 0x02, directory, DIRS_SECTOR, 0);
				interrupt(0x21, useDir<<8|0x31, directory, input+offset, se);
				if(se[2] == NOT_FOUND){
					interrupt(0x21, 0x00, "ERROR! Directory Not Found.\r\n", 0, 0);
				}
				else{
					curDir = (char)se[2];
					interrupt(0x21, 0x20, curDir, 0, 0);
				}
			}
		}
	}
	else if(strncmp(input,"exit", 4)){
	}
	else if(input[0]!=0){
		// Prepare the argument
		useDir = 0xFF;
		offset = 0;
		if(strncmp(input, "./", 2)){
			useDir = curDir;
			offset = 2;
		}
		handleArg(input+offset, curDir);

		// Run the program

		for(idx=offset;input[idx]!=' ' && input[idx]!='\0';idx++);
		input[idx] = '\0';
		interrupt(0x21, 0xFF<<8|0x6, input, 0x2000, &valid);
		if(valid!=SUCCESS){
			if(valid == NOT_FOUND)
				interrupt(0x21, 0x00, "ERROR! Program Not Found.\r\n", 0, 0);
			else
				interrupt(0x21, 0x00, "ERROR OCCURRED!!\r\n", 0, 0);
		}
	}
	interrupt(0x21, 0x07, &valid, 0, 0);
}


void handleArg(char *input, curDir){
	int idx =0, i=0;
	int argc = 0;
	char *argv[16];
	char dummy[16][128];
	while(input[idx]!=' ' && input[idx]!='\0')
		idx++;
	if(input[idx]!='\0'){
		while(1){
			if(input[idx]=='\0'||input[idx]==' '){
				if(i>0){
					dummy[argc][i] = '\0';
					argv[argc] = dummy[argc];
					i = 0;
					argc++;
					if(input[idx]=='\0')
						break;
				}
			}
			else{
				dummy[argc][i] = input[idx];
				i++;
			}
			idx++;
		}
	}
	interrupt(0x21, 0x20, curDir, argc, argv);

}

int strncmp(char *a, char *b, int len){
	int idx;
	for(idx=0;idx<len;idx++){
		if(a[idx]=='\0' || b[idx] == '\0' || a[idx] != b[idx]){
			return 0;
		}
	}
	return 1;
}

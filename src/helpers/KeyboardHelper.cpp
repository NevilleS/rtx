#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "RTXStructures.h"
	

inputbuf * inputMem;

void getInput(int _parentPID){
	char c;
	do{
		c = getchar(); //get the char
		if (((c > 47)&&(c < 59))||((c >96)&&(c<123))||(c==127)||(c==13)||(c==32)){ //accept only if it is alphnumeric or "space", "CR", "Backspace"
			inputMem->data[0] = c;	//store and send
			kill(_parentPID,SIGUSR1);
		}
	}
	while(1);
}
void terminate(int signal){
	exit(0);
}
int main(int argc, char * argv[]) {
	int _bufsize = 4096;
	int _parentPID, FID;
	void* mapPointer;
	
	sigset(SIGINT,terminate);

	sscanf(argv[1], "%d", &_parentPID ); //set up the temp file
	sscanf(argv[0], "%d", &FID );
	mapPointer = mmap((caddr_t) 0,_bufsize,PROT_READ | PROT_WRITE,MAP_SHARED,FID,(off_t) 0);
    if (mapPointer == MAP_FAILED){
		//printf("Keyboard Map Failed\n");//print error to file
		terminate(SIGINT);
    }
	inputMem = (inputbuf *) mapPointer;
	getInput(_parentPID);
	while(1);

}

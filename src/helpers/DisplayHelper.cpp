#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <curses.h>
#include <stdlib.h>
#include "RTXStructures.h"
#include <string.h>


inputbuf * outputMem;
int _parentPID,inputy,inputx,outputx,outputy,position;
char input[60];
WINDOW* timeWin;
WINDOW* mainWin;
WINDOW* inputWin;
WINDOW* outputWin;

void putTime(){		//output the clock in the subwindow
	if (outputMem->clockData[0]==NULL){
		wclear(timeWin);
	}
	else{
		mvwprintw(timeWin,0,0,"%s",outputMem->clockData);
	}
	for (int count = 0; count < strlen(outputMem->clockData);count++){
		outputMem->clockData[count] = 0;
	}
	wrefresh(timeWin);
	wrefresh(inputWin);
}

void putString(){			//just output the whole message
	wmove(outputWin,outputy,0);
	wprintw(outputWin,"%s",outputMem->data);
	wmove(inputWin,inputy,inputx);
	wrefresh(outputWin);
	wrefresh(inputWin);
	getyx(outputWin,outputy,outputx);
	for (int count = 0; count < strlen(outputMem->data);count++){
		outputMem->data[count] = 0;
	}
}

void putChar(){
	if (outputMem->charData[0] == 0){		//move the entire line up to the output screen
		for (inputx;inputx >4;inputx--){	//clear the input line
			mvwdelch(inputWin,0,inputx);
		}
		wmove(outputWin,outputy,outputx);	
		wprintw(outputWin,"\nCCI:  %s\n",input);
		getyx(outputWin,outputy,outputx);
		wrefresh(outputWin);
		int length = strlen(input);
		for (int count = 0; count < length;count++){
			input[count] = 0;
		}
		position =0;
		wmove(inputWin,0,0);
		inputx = 5;
		wmove(inputWin,inputy,inputx);
		wrefresh(inputWin);
	}
	else if (outputMem->charData[0] == 127){	//if it is a backspace
		if (inputx >4){						//remove the char from the buffer and from the screen
			mvwdelch(inputWin,0,inputx);
			wrefresh(inputWin);
			position--;
			input[position] = 0;
			inputx--;
		}
	}
	else{		//if it is a letter just echo the char
		inputx++;
		wmove(inputWin, 0,inputx);
		wechochar(inputWin,outputMem->charData[0]);
		input[position]=outputMem->charData[0];
		position++;
	}
	for (int count = 0; count < strlen(outputMem->charData);count++){
		outputMem->charData[count] = 0;
	}
}

void putOutput(int signal){
	if (outputMem->dataInuse){	//Determine where the data should go
		putString();			//call the function to output it
		outputMem->dataInuse = false;	//reset the flag
	}
	else if(outputMem->charInuse){
		putChar();
		outputMem->charInuse = false;
	}
	else if(outputMem->clockInuse){
		putTime();
		outputMem->clockInuse = false;
	}
	kill(_parentPID,SIGUSR2);	//data has been output signal for more data
}
void terminate(int signal){
	delwin(timeWin);
	delwin(inputWin);
	delwin(outputWin);
	endwin();
	exit(0);
}
int main(int argc, char * argv[]) {
	void* mapPointer;
	int FID;
	int _bufsize = 5016;
	char c;
	sigset(SIGUSR2,putOutput);
	sigset(SIGINT,terminate);	// catch kill signals 

	sscanf(argv[1], "%d", &_parentPID );
	sscanf(argv[0], "%d", &FID );  // get the file id

	mapPointer = mmap((caddr_t) 0,_bufsize,PROT_READ | PROT_WRITE,MAP_SHARED,FID,(off_t) 0);
    if (mapPointer == MAP_FAILED){
		//fprintf(stderr,"%d\n",FID);//print error to file
		//fprintf(stderr,"Display Map Failed\n");//print error to file
		terminate(SIGINT);
    }
	
	outputMem = (inputbuf *) mapPointer; //outputMem is now active

	mainWin = initscr();
	getmaxyx(mainWin,outputy,outputx);
	timeWin = subwin(mainWin,1,0,0,outputx-8);
	inputWin = subwin(mainWin,1,0,outputy-1,0);
	outputWin = subwin(mainWin,outputy-3,0,2,0);
	noecho();
	cbreak();	//do not wait for CR on input
	idlok(outputWin,TRUE);			// enable scrolling on the standard screen
	scrollok(outputWin,TRUE);		// enable scrolling on the standard screen
	wprintw(inputWin,"CCI: ");
	touchwin(mainWin);
	refresh();		
	inputy = 0;
	inputx = 5;
	refresh();
	outputy =2;
	position =0;
	wmove(inputWin,inputy,inputx);

	while (1);
}

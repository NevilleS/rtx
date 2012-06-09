#include <stdlib.h>
#include <signal.h>
#include <unistd.h>	//getpid,execl command
#include <sys/mman.h>
#include <fcntl.h>  //open command
#include <sys/wait.h>
#include <stdio.h> //testing statement
#include <string.h>
#include "List.cpp"
#include "RTX.h"
#include "PostOffice.h"

extern RTX* g_RTX;
extern PostOffice* g_mailSystem;

void IOHandler::terminate() {

	//Signal the helper processes to terminate themselves
	kill(_keyboardPID,SIGINT);
	kill(_displayPID,SIGINT);
	
	//unmap the two shared memory files
    if (munmap((void*)_kbSMem, BUFFER_SIZE) == -1)
		printf("Termination Error: Unable to unmap 'kbMem'\n");//print error
    if (munmap((void*)_crtSMem, BUFFER_SIZE) == -1)
		printf("Termination Error: Unable to unmap 'dspMem'\n");//print error

	//close the two files
    if (close(_displayFID) == -1)
		printf("Termination Error: Unable to close 'dspMem'\n");//print error
	if (close(_keyboardFID) == -1)
		printf("Termination Error: Unable to close 'kbMem'\n");//print error

	//unlink the two files
    if (unlink(DSP_FILENAME) == -1)
		printf("Termination Error: Unable to unlink 'dspMem'\n");//print error
	if (unlink(KB_FILENAME) == -1)
		printf("Termination Error: Unable to unlink 'kbMem'\n");//print error

	exit(1);//exit the program
}

int IOHandler::getKeyboardRequest(MsgEnv* Message){
	if (Message->_messageType == GET_CHARS){ //Check to see if the message is for keyboard
		_keyboardQueue.enqueue(Message);	//if it is, add it to the queue waiting for input
	}
	else{
		g_mailSystem->send_message(Message->_destinationID,Message);	//otherwise, return it.
	}
	return SUCCESS;
}

void IOHandler::getKeyboardReceive(){
	
	if (_position<MAX_COMMAND_LENGTH){	//if we have room left in the message array
		if (_kbSMem->data[0] == 13){	//if the character is an EOL, place an EOL in the array and
			input[_position]=0;			//call the getKeyboardReturn function
			sendDisplayChars(0);
			getKeyboardReturn(input);
			_kbSMem->data[0]=0;			//clear the shared mem
		}
		else if(_kbSMem->data[0] == 127){	//if the character is a backspace, remove the previous
			if (_position > 0){				//char from the array and call sendDisplayChars to remove
				_position --;				//previous character from display
				input[_position] =0;
				sendDisplayChars(127);
				_position --;
			}
			else if (_position == 0)		//if the backspace has been sent and no chars are in the array
				_position --;				//adjust the _position to start at 0 on next pass
			}
		else{								//otherwise just put the chars in the array and send to CRT
			input[_position]=_kbSMem->data[0];
			sendDisplayChars(input[_position]);
			_kbSMem->data[0]=0;				//clear the shared mem
		}
	}
	else if(_position>=MAX_COMMAND_LENGTH){	//if the array is already full
		getKeyboardReturn(input);			//send the array to CCI
		sendDisplayChars(0);
		_position++;
		input[_position] = _kbSMem->data[0];
	}
	_position++;							//increment the position counter
}

void IOHandler::getKeyboardReturn(char input[]){
	_currentEnvelope = _keyboardQueue.getHead();	//point to the older envelope on the queue
	if (_currentEnvelope!=NULL){					//check if it is Null
		_keyboardQueue.dequeue();					
		sprintf(_currentEnvelope->_messageContents,"%s",input);	//place data in message and send it
		g_mailSystem->send_message(_currentEnvelope->_destinationID,_currentEnvelope);
	}
	for (int count=0; count < MAX_COMMAND_LENGTH; count++){
		input[count] = 0;					//clear the data array
	}
	_position = -1;							//reset the position and set the pointer to NULL
	_currentEnvelope = NULL;
}

void IOHandler::sendDisplayChars(char input){
	if (!_crtSMem->charInuse){
		_crtSMem->charInuse=true;			//set the in use flag
		_crtSMem->charData[0]=input;		//store the received char in the CRT shared Mem
		kill(_displayPID,SIGUSR2);		//signal CRT Helper
	}
}

void IOHandler::sendDisplayTime(int hours,int minutes, int seconds){
	if (!_crtSMem->clockInuse){
		char blank[8]={0};
		if (hours !=25){	//check the time, if it is 25 hours turn out display
			sprintf(_crtSMem->clockData,"%02d:%02d:%02d",hours,minutes,seconds); //if valid send time
		}
		else
			sprintf(_crtSMem->clockData,"%s",blank);
		_crtSMem->clockInuse = true; //set inuse and time flags
		kill(_displayPID,SIGUSR2);	//send signal to user
	}
}

int IOHandler::sendDisplayRequest(MsgEnv* Message){
	if (Message->_messageType == PUT_CHARS){	//check that the message envelope is for us
		_displayQueue.enqueue(Message);			//put the envelope on the display queue
		if (_displayQueue.getCount() == 1){		//if it is the only message on the queue, send it
			sendDisplayReceive();
		}
	}
	else
		g_mailSystem->send_message(Message->_destinationID,Message);	//if not for us, return to sender
	return SUCCESS;
}

void IOHandler::sendDisplayReceive(){
	if (_crtSMem->dataInuse == false){		//check that the shared memory is not in use
		_currentEnvelope = _displayQueue.getHead(); //get the next envelope
		if (_currentEnvelope!=NULL){			//pull the file from the queue
			_displayQueue.dequeue();
			sprintf(_crtSMem->data,"%s",_currentEnvelope->_messageContents);	//put the message into shared mem
			_crtSMem->dataInuse = true;	//set the status flag
			kill(_displayPID,SIGUSR2);	//signal the helper
			g_mailSystem->send_message(_currentEnvelope->_destinationID,_currentEnvelope); //return envelope
		}
	}
	_currentEnvelope = NULL; //clear the pointer
}

void IOHandler::initialize() {
	char _parentPID[20];
	char kbMem[20];
	char dspMem[20];
    _position = 0;

	/*Setup the Keyboard shared memory and fork the helper process*/
	_keyboardFID = open(KB_FILENAME, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 ); //Open the temp file to be used.
	if (_keyboardFID < 0){
		//fprintf(stderr,"Unable to open Keyboard File -> Terminating");//print error to file
		g_RTX->terminate();
	}

	ftruncate(_keyboardFID,BUFFER_SIZE);	//Set the file to the buffer size
	_myPID = getpid();							//get PID to forward to helpers
	sprintf(kbMem, "%d",_keyboardFID);		
	sprintf(_parentPID, "%d",_myPID);

	_keyboardPID = fork();	//fork the keyboard helper
	if (_keyboardPID == 0){
		execl("bin/KB.out", kbMem,_parentPID, NULL); //start the helper
		//printf("Unable to exec keyboardHelper -> Terminating");//print error to file
		g_RTX->terminate();
	}
	sleep(1);	//the process some time

	void* kbPointer = mmap((caddr_t) 0,BUFFER_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,_keyboardFID,(off_t) 0);
    if (kbPointer == MAP_FAILED){
		//printf("Unable to map Keyboard File -> Terminating");//print error to file
		g_RTX->terminate();
    };
	_kbSMem = (inputbuf *)kbPointer;

	/*Setup the Display shared memory and fork the helper process*/

	_displayFID = open(DSP_FILENAME, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );
	if (_displayFID < 0){
		//printf("Unable to open Display File -> Terminating");//print error to file
		g_RTX->terminate();
	}
	
	ftruncate(_displayFID,BUFFER_SIZE);
	sprintf(dspMem, "%d",_displayFID);

	_displayPID = fork();	//fork the display helper
	if (_displayPID == 0){
		execl("bin/DSP.out", dspMem,_parentPID, NULL); //start the helper
		//printf("Unable to exec displayHelper -> Terminating");//print error to file
		g_RTX->terminate();
	}
	sleep(1);

	void* crtPointer = mmap((caddr_t) 0,BUFFER_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED, _displayFID,(off_t) 0);
    if (crtPointer == MAP_FAILED){
		//printf("Unable to map display file -> Terminating");//print error to file
		g_RTX->terminate();
    }
	_crtSMem = (inputbuf *)crtPointer;
}

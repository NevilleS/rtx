#ifndef IOHandler_H
#define IOHandler_H

#include "RTXConstants.h"
#include "RTXStructures.h"
#include "List.h"

class IOHandler {

private:

    List<MsgEnv> _displayQueue; //Queue used to store messages waiting to be output to CRT
    List<MsgEnv> _keyboardQueue; //Queue of envelopes waiting to receive messages from KB
    MsgEnv* _currentEnvelope;	//pointer used to store envelope not current on queue (on dequeue)
    int _myPID;				//_myPID stores the PID of RTX, passed to helper processes for signalling
    int _keyboardPID;		//internal reference of the KeyboardHelper PID - for signalling
    int _displayPID;		//internal reference of the DisplayHelper PID - for signalling
    int _keyboardFID;		//internal reference of the keyboard shared memory file (File ID)
    int _displayFID;		//internal reference of the display shared memory file (File ID)
	inputbuf* _kbSMem;		//the keyboard shared mem pointer
	inputbuf* _crtSMem;		//the display shared mem pointer
	int _position;			//array index
	char input[MAX_COMMAND_LENGTH];			//char array

public:
	/*The initialize function is used to set up the two shared memory files used for passing data 
	/between the IOHandler and the KeyboardHelper and DisplayHelper. This function is also responsible
	/creating the new processes and executing their code.*/
	void initialize();

	/*The getKeyboardRequest function is used to internally accept requests from RTX (CCI) to receive
	/an input from the keyboard. The MsgEnv pointer that is passed to the function is checked for
	/integrity and is then placed on the _keyboardQueue. If the envelope is not intended for the
	/function it is sent back via the PostOffice.*/
	int getKeyboardRequest(MsgEnv* Message);
	
	/*getKeyboardReceive is called by the InterruptHandler when a SIGUSR1 signal is received from
	/the KeyboardHelper. The function is responsible for moving the data collected by the KeyboardHelper
	/into the storage queue. Each char is then output to the screen. Once the queue is full or ended, the
	/contents are pushed to the getKeyboardReturn function.*/
	void getKeyboardReceive();

	/*Similar to the getKeyboardRequest, the sendDisplayRequest is used to queue internal requests for
	/messages to be output to the monitor. The MsgEnv pointer passed to the function is placed on the
	/ _displayQueue.If the envelope being placed on the queue is the only one, then the sendDisplayReceive
	/ function is immediately called to output the message.*/
	int sendDisplayRequest(MsgEnv* Message);
	
	/*The getKeyboardReturn function is responsible for placing the input array into the message contents
	/ If there is no envelope waiting the message is destroyed.*/
	void getKeyboardReturn(char input[]);

	/*sendDisplayRequest is called when RTX receives a SIGUSR2 signal or if the envelope that was queued 
	/in sendDisplayRequest was the only one on the queue. The next envelope (or only) is removed from the 
	/queue and the message contents are placed into the shared memory file and the inuse flag is set. The 
	/process then signals the DisplayHelper to output the message. Lastly, the envelope is returned to sender
	/(CCI)*/
	void sendDisplayReceive();

	/*sendDisplayChars is used to send one char at a time to the display. This function's purpose is to provide
	/ and output for the user so they can see what has been typed (commands etc.)*/
	void sendDisplayChars(char input);

	/*The sendDisplayTime function accepts three integer values representing hour:minute:second of the time
	/to be output onto the display. The hour is tested against a value of 25 (a signal to remove the time)
	/and is then converted into a char. If the hour is 25, the array of chars is null, if it is a valid time
	/the time is converted in the hh:mm:ss format, the time and inuse flags are set, and then the message is
	/immediately sent to the monitor*/
	void sendDisplayTime(int hours,int minutes, int seconds);
	
	/*The terminate function is responsible for cleaning up after the IOHandler and the helper processes.
	/First the processes are signaled to terminate themselves. Afterwards the shared memory is unlinked
	/and then closed.*/
	void terminate();

};

#endif


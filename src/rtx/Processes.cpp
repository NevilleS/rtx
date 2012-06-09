#include "Processes.h"
#include <stdio.h>
#include "RTX.h"
#include "List.cpp"
#include "RTXConstants.h"
#include <string.h>

#define PROCESS_C_DELAY_TICKS 10 // 1s (1 tick = 1s)

extern RTX* g_RTX;

/***
 * This function is the definition of the first user process. This process waits for a message
 * from the console (when the user types 's') to start the test sequence. Once the sequence starts,
 * this process continously requests an envelope to send a message to processB that simply contains
 * the count of messages sent.
 ***/
void processA() {
    MsgEnv* startMsg = g_RTX->receive_message();
    g_RTX->release_msg_env(startMsg);
    int num = 0;
    while(true) {
        MsgEnv* tmpMsg = g_RTX->request_msg_env();
        tmpMsg->_senderID = PROCESS_A;
        tmpMsg->_destinationID = PROCESS_B;
        tmpMsg->_messageType = COUNT_REPORT;
        tmpMsg->_messageContents[0] = ++num;
        g_RTX->send_message(PROCESS_B, tmpMsg); // send the latest count to processB
        g_RTX->release_processor();
    }
}

/***
 * This defines of the second user process for the test sequence. This process simply redirects messages
 * from process A to process C. If this process is too low of priority, it will not perform its function since
 * process A will continously send it messages but not allow it to run.
 ***/
void processB() {
    while(true) {
        MsgEnv* relayMsg = g_RTX->receive_message();
        relayMsg->_senderID = PROCESS_B;
        relayMsg->_destinationID = PROCESS_C;
        g_RTX->send_message(PROCESS_C, relayMsg);
    }
}

/***
 * processC is the most complex of the test processes. This process' main function is to output "Process C" to the
 * screen after every 20 messages it receives from process A. However, this process also sleeps after each output,
 * queuing incoming messages from process A until 10 seconds have elapsed. It also releases the messages, which
 * is important for the test sequence to run continously. If this process is lower priority than process A, messages
 * will be allocated faster than they are released, which will eventually starve the system.
 ***/
void processC() {
    List<MsgEnv> localQueue;
    MsgEnv* nextMsg;
    while(true) {
        if(localQueue.getCount() != 0) {
            nextMsg = localQueue.dequeue();
        }
        else {
            nextMsg = g_RTX->receive_message();
        }
        if(nextMsg->_messageType == COUNT_REPORT) {
            if(((int)nextMsg->_messageContents[0]) % 20 == 0) {
                nextMsg->_senderID = PROCESS_C;
                nextMsg->_destinationID = PROCESS_C;
                nextMsg->_messageType = PUT_CHARS;
                sprintf(nextMsg->_messageContents, "Process C\n");
                g_RTX->send_console_chars(nextMsg);
                nextMsg = NULL;
                while(nextMsg == NULL) { // wait for display acknowledgment
                    nextMsg = g_RTX->receive_message();
                    if(nextMsg->_messageType != PUT_CHARS ) { // if it's not from the RTX queue it for now
                        localQueue.enqueue(nextMsg);
                        nextMsg = NULL;
                    }
                }
                nextMsg->_senderID = PROCESS_C;
                nextMsg->_destinationID = PROCESS_C;
                nextMsg->_messageType = WAKEUP_10;
                g_RTX->request_delay(PROCESS_C_DELAY_TICKS, WAKEUP_10, nextMsg);
                nextMsg = NULL;
                while(nextMsg == NULL) {
                    nextMsg = g_RTX->receive_message();
                    if(nextMsg->_messageType != WAKEUP_10) {
                        localQueue.enqueue(nextMsg);
                        nextMsg = NULL;
                    }
                }
            }
        }
        g_RTX->release_msg_env(nextMsg);
        g_RTX->release_processor();
    }
}

void nullProcess() {
    while(true) {
        g_RTX->release_processor();
    }
}
////////////////////////////
// CCI Parsers & Functions
////////////////////////////
bool isNumber(char toCheck){
	if((toCheck>47)&&(toCheck<58))
		return true;
	else
		return false;
}

int charToInt(char toTranslate){
	return static_cast<int>(toTranslate)-48;
}

bool extractTime(char* command, int &hr, int &min, int &sec){
	hr=0;
	min=0;
	sec=0;
	bool validTime=true;
	if(!isNumber(command[2]))
		validTime=false;
	if(!isNumber(command[3]))
		validTime=false;
	if(!isNumber(command[5]))
		validTime=false;
	if(!isNumber(command[6]))
		validTime=false;
	if(!isNumber(command[8]))
		validTime=false;
	if(!isNumber(command[9]))
		validTime=false;
	if (validTime){
		hr = 10*charToInt(command[2])+charToInt(command[3]);
		min = 10*charToInt(command[5])+charToInt(command[6]);
		sec = 10*charToInt(command[8])+charToInt(command[9]);
		if((hr>23)||(min>59)||(sec>59))
			validTime = false;
	}
	return validTime;
}

bool extractChange(char* command, int &newPriority, int &processID){
	newPriority=3;
	processID=0;
	bool validChange=true;
	if(!isNumber(command[2]))
		validChange=false;
	else{
		newPriority=charToInt(command[2]);
		if((newPriority<0)||(newPriority>3))
			validChange=false;
	}
	if (validChange){
		if (command[5]==0)
			processID=charToInt(command[4]);
		else if(command[6]==0)
			processID=10*charToInt(command[4])+charToInt(command[5]);
	}
	return validChange;
}

int parseCommand(char* command){
	if ((command[0]=='s')&&(command[1]==0))
		return 1;
	else if ((command[0]=='p')&&(command[1]=='s')&&(command[2]==0))
		return 2;
	else if (command[0]=='c'){
		if ((command[1]==32)&&(command[4]==58)&&(command[7]==58))
			return 3;
		else if((command[1]=='d')&&(command[2]==0))
			return 4;
		else if ((command[1]=='t')&&(command[2]==0))
			return 5;
	}
	else if ((command[0]=='b')&&(command[1]==0))
		return 6;
	else if ((command[0]=='t')&&(command[1]==0))
		return 7;
	else if ((command[0]=='n')&&(command[1]==32)&&(command[3]==32)&&((command[5]==0)||(command[6]==0)))
		return 8;
	else if ((command[0]=='w')&&(command[1]=='a')&&(command[2]=='i')&&(command[3]=='t')&(command[4]==32)&&
             (command[7]==32)&&(command[8]=='t')&&(command[9]=='h')&&(command[10]=='e')&(command[11]=='n'))
        return 9;

	return 0;
}


///////////////////////////////////////
// User Callable Functions (via CCI)
///////////////////////////////////////

void messageProcessA(char* command, MsgEnv* message){
	MsgEnv* toSend = g_RTX->request_msg_env();
	toSend->_messageType = START_TEST;
	toSend->_destinationID = PROCESS_A;
	toSend->_senderID = CCI;
	g_RTX->send_message(PROCESS_A, toSend);

}

void displayProcessInfo(char* command, MsgEnv* message){
	MsgEnv* processStatus =  message;
	int retVal = g_RTX->request_process_status(processStatus);
    if(retVal == SUCCESS) {
        processStatus->_senderID = CCI;
        processStatus->_destinationID = CCI;
        processStatus->_messageType = PUT_CHARS;
    	g_RTX->send_console_chars(processStatus);
    }
}

void setTime(char* command, MsgEnv* message){
	int hr, min, sec;
	if(extractTime(command, hr, min, sec)){
		g_RTX->set_clock(sec, min, hr);
	}
}

void displayClock(char* command, MsgEnv* message){
	g_RTX->display_clock(true);
}

void hideClock(char* command, MsgEnv* message){
	g_RTX->display_clock(false);
}

void printBuffers(char* command, MsgEnv* message){
	MsgEnv* traceBuffer = message;
	traceBuffer->_messageType = PUT_CHARS;
	traceBuffer->_destinationID=CCI;
	traceBuffer->_senderID=CCI;
	int retVal = g_RTX->get_trace_buffers(traceBuffer);
    if(retVal == SUCCESS) {
    	g_RTX->send_console_chars(traceBuffer);
    }
	else{
		sprintf(traceBuffer->_messageContents, "Couldn't obtain trace buffer (Reason unknown)\n");
		g_RTX->send_console_chars(traceBuffer);
	}
}

void terminateOS(char* command, MsgEnv* message){
	MsgEnv* terminateMessage = message;
	terminateMessage->_messageType = PUT_CHARS;
	terminateMessage->_destinationID=CCI;
	terminateMessage->_senderID=CCI;
	sprintf(terminateMessage->_messageContents, "Terminating!\n");
	g_RTX->send_console_chars(terminateMessage);
	g_RTX->terminate();
}

void priorityChangeReq(char* command, MsgEnv* message){
	int priority, pid;
	MsgEnv* successMessage = message;
	successMessage->_messageType = PUT_CHARS;
	successMessage->_destinationID=CCI;
	successMessage->_senderID=CCI;
	if(extractChange(command, priority, pid)){
		if ((g_RTX->change_priority(priority, pid))==0){
			sprintf(successMessage->_messageContents, "Priority Change Successful\n");
			g_RTX->send_console_chars(successMessage);
		}
		else{
			sprintf(successMessage->_messageContents, "Priority Change Failed!  (Reason Unknown)\n");
			g_RTX->send_console_chars(successMessage);
		}
	}
	else{
		sprintf(successMessage->_messageContents, "Priority Change Failed!  (Bad Request)\n");
		g_RTX->send_console_chars(successMessage);
	}
	g_RTX->release_msg_env(successMessage);
}

void badCommand(char* command, MsgEnv* message){
	sprintf(message->_messageContents, "Invalid command entered.  Please try again.\n");
	g_RTX->send_console_chars(message);
}

///////////////
// CCI
///////////////

void consoleProcess() {
	// setup of array of function pointers
	void (*functionList[9])(char*, MsgEnv*) = {&badCommand,&messageProcessA,&displayProcessInfo,&setTime,&displayClock,&hideClock,&printBuffers,&terminateOS,&priorityChangeReq};
	
	MsgEnv* command;
	MsgEnv* receivedMsg;
	MsgEnv* forFunctions;

	for (int i=0; i<4; i++){
		command = g_RTX->request_msg_env();
		command->_senderID=CCI;
		command->_destinationID=CCI;
		command->_messageType=GET_CHARS;
		g_RTX->get_console_chars(command);
	}

	forFunctions = g_RTX->request_msg_env();
	forFunctions->_senderID=CCI;
	forFunctions->_destinationID=CCI;
	forFunctions->_messageType=PUT_CHARS;

	// Standard Operation (infinite loop that will repeatedly block on recieve)
	while (true){
		receivedMsg = g_RTX->receive_message();
        if (receivedMsg->_messageType == GET_CHARS){
			command = receivedMsg;

			// Parse current command and act appropriately
			functionList[parseCommand(command->_messageContents)](command->_messageContents, forFunctions);

			// clear envelope contents
			for(int i=0; i<MESSAGE_SIZE; i++)
				command->_messageContents[i]=0;
			command->_messageType=GET_CHARS;
			
			// request next input
			g_RTX->get_console_chars(command);
		}
		else if(receivedMsg->_messageType=PUT_CHARS)
			forFunctions = receivedMsg;
		else
			g_RTX->release_msg_env(receivedMsg);
	}
}


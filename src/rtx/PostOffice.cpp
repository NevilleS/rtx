#include <string.h>
#include <stdio.h>
#include "PostOffice.h"
#include "Scheduler.h"
#include "TimingService.h"
#include "RTXConstants.h"
#include "List.cpp"	


void clearMessage(MsgEnv* msgEnvelope);
char* boolToString(bool isSend);

extern Scheduler* g_scheduler;
extern PostOffice* g_mailSystem;
extern TimingService* g_timer;

int PostOffice::initialize(){
    _emptyMessages = new List<MsgEnv>;
    _traceBuffer = new List<MessageLog>;
	//Create the messages
	for (int i=0; i < NUMBER_OF_MESSAGES; i++) _emptyMessages->enqueue(new MsgEnv);
	return SUCCESS;
}

MsgEnv* PostOffice::request_msg_env(){
	return _emptyMessages->dequeue();
}

int PostOffice::release_msg_env(MsgEnv* msg_env_ptr){
	int retVal;
	PCB* blockedPCB;
	//Clear all the values in the envelope and enqueue it
	clearMessage(msg_env_ptr);
	_emptyMessages->enqueue(msg_env_ptr);

	//Check if there are any processes blocked on message_allocate
	blockedPCB = g_scheduler->getNextBlockedPCB(BLOCKED_MSG_ALLOC);
				
	//If a process is blocked on received, then make that process ready
	//(just the highest priority process, not all blocked ones)
	if (blockedPCB != NULL){
		g_scheduler->makeReady(blockedPCB);
	}
	//Otherwise - or once the PCB is made ready - just return
	return SUCCESS;
}

int PostOffice::send_message(int dest_process_id, MsgEnv* msg_envelope){
	int retVal;
	PCB* destPCB = g_scheduler->getPCB(dest_process_id);
	//Make sure we were passed a valid pID
	if (destPCB == NULL) return INVALID_INPUT;

	//Send the message and add a trace of this send
	destPCB->_mailbox.enqueue(msg_envelope);
	addTrace(msg_envelope, true);

	//Make the process ready if it was blocked on message receive, and return
	if (destPCB->_status == BLOCKED_MSG_RECEIVE){
		retVal = g_scheduler->makeReady(destPCB);
		return retVal;
	}

	//If the process wasn't blocked, just return
	return SUCCESS;
}

MsgEnv* PostOffice::receive_message(){
	PCB* receiverPCB = g_scheduler->getCurrentPCB();
	MsgEnv* currentMsg = receiverPCB->_mailbox.dequeue();
	if (currentMsg != NULL) addTrace(currentMsg, false);
	return currentMsg;
}

int PostOffice::get_trace_buffers(MsgEnv* message_envelope){
	if (message_envelope == NULL){
		return INVALID_INPUT;
	}

	//Make sure the envelope's contents are empty
	if (strlen(message_envelope->_messageContents) > 0 ){
		for (int i = 0; i < MESSAGE_SIZE; i ++) message_envelope->_messageContents[i] = 0;
	}
	MessageLog* nextLog;

	char* dataPtr = message_envelope->_messageContents;
    dataPtr = dataPtr + sprintf(dataPtr, "Send/Receive    From         To           Message Type    Timestamp\n");
	for (int i=0; i < _traceBuffer->getCount(); i++){
		nextLog = _traceBuffer->dequeue();
		dataPtr = dataPtr + sprintf(dataPtr, "%-16s", boolToString(nextLog->_isSend));
        dataPtr = dataPtr + sprintf(dataPtr, "%-13s", PROCESS_DESCS[nextLog->_senderID]);
        dataPtr = dataPtr + sprintf(dataPtr, "%-13s", PROCESS_DESCS[nextLog->_destinationID]);
        dataPtr = dataPtr + sprintf(dataPtr, "%-16s", MESSAGETYPE_DESCS[nextLog->_messageType]);
		dataPtr = dataPtr + sprintf(dataPtr, "%02i:%02i:%02i\n", nextLog->_hours,nextLog->_minutes,nextLog->_seconds);
		_traceBuffer->enqueue(nextLog);
	}
	return SUCCESS;
}

int PostOffice::terminate(){
	delete _emptyMessages;
	delete _traceBuffer; 
	return SUCCESS;
}

void PostOffice::addTrace(MsgEnv* msgEnvelope, bool isSend){
	//Create the new trace
	MessageLog* newLog = new MessageLog();
	newLog->_senderID = msgEnvelope->_senderID;
	newLog->_destinationID = msgEnvelope->_destinationID;
	newLog->_messageType = msgEnvelope->_messageType;
	newLog->_isSend = isSend;
	g_timer->getTime(&(newLog->_seconds),&(newLog->_minutes),&(newLog->_hours));
 
	//Add the trace to the queue (make sure there's room first)
	while ( _traceBuffer->getCount() >= TRACE_BUFFER_SIZE)	delete _traceBuffer->dequeue();
	_traceBuffer->enqueue(newLog);
}


/**************************************
*HELPER METHODS
**************************************/

/***
*Clear all the values in a given envelope
*
*Parameters:
*	msgEnvelope: The envelope to clear the values in
*
*Returns:
*	none
***/
void clearMessage(MsgEnv* msgEnvelope){
	for(int i = 0; i < MESSAGE_SIZE; i++) msgEnvelope->_messageContents[i] = 0;
}


/***
*Converts a bool to Send/Receive
*
*Parameters:
*	isSend: the bool to add to dest
*
*Returns:
*	none
***/
char* boolToString(bool isSend){
	if (isSend)
		return "Send";
	else 
		return "Receive";
}

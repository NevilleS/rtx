#ifndef RTXStructures_H
#define RTXStructures_H

#include "RTXConstants.h"
#include "List.h"
#include <setjmp.h>

// Shared memory structure
typedef struct inputbuf_t{
	char data[MAXCHAR];
	char clockData[CLOCK_SIZE];
	char charData[2];
	bool dataInuse;
	bool clockInuse;
	bool charInuse;
}inputbuf;

//Message Envelopes
typedef struct MsgEnv_t {
	int _senderID;
	int _destinationID;
	int _messageType;
	char _messageContents[MESSAGE_SIZE];
	MsgEnv_t() {
        _senderID = DEFAULT_ID;
		_destinationID = DEFAULT_ID;
        _messageType = DEFAULT_TYPE;
    }
}MsgEnv;

// Record (used to populate a PCB)
typedef struct Record_t {
    int _processID;
    Priority _priority;
    Process _processPtr;
}Record;

// Process Control Block: the RTX representation of a user-level process
typedef struct PCB_t {
	int _processID;
    int _runCount;
	int _priority;
	Status _status;
    Process _processPtr;
    char* _stackPtr;
    jmp_buf _context;
    List<MsgEnv> _mailbox;
    PCB_t(Record r) {
        _processID = r._processID;
        _runCount = 0;
        _priority = r._priority;
        _status = READY;
        _processPtr = r._processPtr;
        _stackPtr = (char*)malloc(STACK_SIZE);
    }
    ~PCB_t() {
        free(_stackPtr);
    }
}PCB;

// Internal structure to keep track of delay requests
typedef struct TimeRequest_t {
	int _processID;
	int _time;
	MsgEnv* _message;
	int _wakeUpCode;
	TimeRequest_t() {
        _processID = 0;
        _time = 1;
        _message = NULL;
        _wakeUpCode=-1;
    }
}TimeRequest;

//Message Logs - used to keep a message send/receive history for getTraceBuffer
typedef struct MessageLog_t {
	int _senderID;
	int _destinationID;
	int _messageType;
	int _seconds;
	int _minutes;
	int _hours;
	bool _isSend;
    MessageLog_t() {
        _senderID = 0;
        _destinationID = 0;
        _messageType = 0;
        _seconds = 0;
        _minutes = 0;
        _isSend = 0;
    }
}MessageLog;

#endif


#ifndef RTXConstants_H
#define RTXConstants_H

/***
 * The RTX representation of a Process is a simple C function that returns void
 * and accepts no input arguments. 
 ***/
typedef void (*Process)(void);

/***
 * Priority Constants
 ***/
#define PRIORITY_COUNT 5
enum Priority { CRITICAL, HIGH, MEDIUM, LOW, NULL_PROCESS };
static const char* PRIORITY_DESCS[PRIORITY_COUNT] = { "CRITICAL", "HIGH", "MEDIUM", "LOW", "NULL" };


/***
 * Status Constants
 ***/
#define BLOCKED_QUEUE_COUNT 1 // BLOCKED_MSG_RECEIVE does not require a queue
#define STATE_COUNT 4
enum Status { BLOCKED_MSG_ALLOC, BLOCKED_MSG_RECEIVE, READY, RUNNING };
static const char* STATUS_DESCS[STATE_COUNT] = { "BLOCKED_MSG_ALLOC", "BLOCKED_MSG_RECEIVE", "READY", "RUNNING" };

/***
 * PCB Constants
 ***/
#define STACK_SIZE 16384

/***
* Constant defining number of ticks equivalent to 1 second
* (will be dependant on how often we ask UNIX to send signal
***/
#define TICKS_PER_SECOND 1 //Should apparently be 100ms

/***
 * Process IDs. This needs to be manually synced with the index of the process in InitializationTable.h
 ***/
#define PROCESS_COUNT 5
#define CCI 1
#define PROCESS_A 2
#define PROCESS_B 3
#define PROCESS_C 4
static const char* PROCESS_DESCS[PROCESS_COUNT] = { "DEFAULT ID", "CCI", "PROCESS A", "PROCESS B", "PROCESS C" };

/***
* Messaging service constants
***/
#define NUMBER_OF_MESSAGES 100
#define MESSAGE_SIZE 4096
#define DEFAULT_ID 0
#define DEFAULT_TYPE -1
#define TRACE_BUFFER_SIZE 16
#define MESSAGETYPE_COUNT 6
enum MessageTypes { GET_CHARS,PUT_CHARS,START_TEST,COUNT_REPORT,INFO_REQ,WAKEUP_10 };
static const char* MESSAGETYPE_DESCS[MESSAGETYPE_COUNT] = {"GET_CHARS","PUT_CHARS","START_TEST","COUNT_REPORT","INFO_REQ","WAKEUP_10"};

/***
* Interrupt Handler constants
***/
#define ON true
#define OFF false

/***
* IO Handler
***/
#define MAXCHAR 4096 // maximum buffer size for the keyboard input buffer
#define BUFFER_SIZE 5016 // mmap'd buffer size (at least the size of inputbuf)
#define CLOCK_SIZE 8
#define DSP_FILENAME "dspMem"
#define KB_FILENAME  "kbMem"
#define MAX_COMMAND_LENGTH 60

/***
* Return Codes
***/
enum ErrorCodes { SUCCESS, UNKNOWN_ERROR, NULL_POINTER, OUT_OF_BOUNDS, BUFFER_OVERFLOW, OUT_OF_MEMORY, INVALID_INPUT };

#endif


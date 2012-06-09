#ifndef RTX_H
#define RTX_H


#include<stdlib.h>
#include<stdio.h>
#include"InterruptHandler.h"
#include"PostOffice.h"
#include"IOHandler.h"
#include"Scheduler.h"
#include"TimingService.h"
#include"List.h"
#include"RTXStructures.h"
#include"RTXConstants.h"
#include"InitializationTable.h"

class RTX {

private:
	// Services and Sub Processes
	PostOffice* _mailSystem;
	Scheduler* _scheduler;
	IOHandler* _ioHandler;
	InterruptHandler* _interruptHandler;
	TimingService* _timer;
	
public:
	// Initialization, Operation and Termination of RTX
	int initialize(); // initialize the kernel objects
	int execute(); // start operation of the RTX (dispatch first process)
	int terminate(); // termination sequence (free memory etc.)

    /***
     * The RTX kernel is implemented by a collection of specific objects (Scheduler, IOHandler, etc). To keep the stack
     * valid as context switches are performed, these objects are allocated in the global scope of the main() function.
     * However, the RTX needs to access these objects, so the attach() function allows the kernel to be assembled from
     * its parts. After the main() function allocates these objects, it calls this function to transfer control of the objects
     * to the kernel itself, which becomes responsible for deleting them on termination.
     *
     * Params:
     *   pointers to each of the 5 main kernel components
     *
     * Returns:
     *   integer error code
     ***/
    int attach(PostOffice* mailSystem, Scheduler* scheduler, IOHandler* ioHandler,
               InterruptHandler* interruptHandler, TimingService* timer); //assemble the kernel objects

	 /***
     * Let the current process receive the next message in it's message queue.
	 * Blocks the current process on MessageReceive if there are no messages
     * Params:
     *   none
     *
     * Returns:
     *   the first message envelope in the queue
     ***/
	MsgEnv* receive_message();

    /***
     * Asks messaging services for a new message envelope
	 * Blocks on message request if there are no available envelopes
     *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
	MsgEnv* request_msg_env();

	 /***
     * Tells the scheduler to release the current process, and then calls dispatch
	 *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
    int release_processor();

	 /***
     * Sends the message envelope to the destination id
	 * Checks if the receiver is blocked on message receive
	 *
     * Params:
     *  dest_process_id: the PID of the process to receive the message
	 *	msg_envelope: the envelope to send
     *
     * Returns:
     *   integer error code
     ***/
	int send_message(int dest_process_id, MsgEnv* msg_envelope);

	 /***
     * Receives the current status and priority of all processes from the scheduler.
	 * The statuses are passed in the envelope with a number indicating the number of process
	 * and a 2D array with proc_id, status, and priority
	 * Doesn't need to send the message back to the calling process
	 * because that process will already have a pointer to it
	 *
     * Params:
	 *	msg_envelope: the envelope for the scheduler to pass the statuses in
     *
     * Returns:
     *   integer error code
     ***/
	int request_process_status(MsgEnv* msg_env_ptr);

	 /***
     * Gets the scheduler to change the target processes priority to the new priority
	 *
     * Params:
	 *	new_priority: the new priority for the process
	 *	target_process_id: the Pid of the process to change
     *
     * Returns:
     *   integer error code
     ***/
	int change_priority(int new_priority, int target_process_id);

	 /***
     * Sends chars to the console (passed through a message envelope)
	 * Implemented by the IO Handler
	 *
     * Params:
	 *	message_envelope: the envelope with the console chars
     *
     * Returns:
     *   integer error code
     ***/
	int send_console_chars(MsgEnv* message_envelope);

	 /***
     * Gets chars from the console (passed through a message envelope)
	 * Implemented by the IO Handler
	 *
     * Params:
	 *	message_envelope: the envelope for the chars to be stored int
     *
     * Returns:
     *   integer error code
     ***/
	int get_console_chars(MsgEnv* message_envelope);

	/***
     * Gets trace buffers for debugging (up to 16 traces)
	 * Implemented by messaging services
	 *
     * Params:
	 *	message_envelope: the envelope for the trace data to be stored in
     *
     * Returns:
     *   integer error code
     ***/
	int get_trace_buffers(MsgEnv* message_envelope);

	/***
     * Gets trace buffers for debugging (up to 16 traces)
	 * Implemented by timing services
	 *
     * Params:
	 *	time_delay: the amount of time that the process wants to delay for
	 *	wakeup_code: the message_type that will identify the message as a wake-up message
	 *	message_envelope: the envelope for the wakeup message to be sent in
     *
     * Returns:
     *   integer error code
     ***/
	int request_delay(int time_delay, int wakeup_code, MsgEnv* message_envelope);

	/***
     * Releases an envelope for other processes to use
	 * Checks if there are any processes blocked on memory_allocation
	 * Implemented by messaging services
	 *
     * Params:
	 *	message_envelope: the envelope to release
     *
     * Returns:
     *   integer error code
     ***/
	int release_msg_env(MsgEnv* msg_env_ptr);

    /***
     * Toggles the display of the clock.
     *
     * Params:
     *  display: whether the clock should be displayed
     *
     * Returns:
     *  integer error code
     ***/
	int display_clock(bool display);

    /***
     * Sets the current time of the clock.
     *
     * Params:
     *  second, minute, hour: the time to set the clock to
     *
     * Returns:
     *  integer error code
     ***/
	int set_clock(int second, int minute, int hour);

    /***
     * Queries the display status of the clock.
     *
     * Params:
     *  none
     *
     * Returns:
     *  the current status of the clock (true = displayed)
     ***/
	bool get_clock_display_status(){ return _timer->fetchBool(); }

};

#endif

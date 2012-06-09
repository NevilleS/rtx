#ifndef PostOffice_H
#define PostOffice_H

#include "RTXConstants.h"
#include "RTXStructures.h"
#include "List.h"

class PostOffice {
private:
	List<MsgEnv>* _emptyMessages;
	List<MessageLog>* _traceBuffer;

	/***
	*Adds a new message log to the traceBuffer
	*
	*Parameters:
	*	isSend: indicates whether the trace is for a send or receive command (send == true)
	*
	*Returns:
	*	none
	***/
	void addTrace(MsgEnv* msgEnvelope, bool isSend);
public:

	 /***
     * Creates the lists and fills the emptyMessage list
	 *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
	int initialize();

	 /***
     * Dequeue and return a message from the list of empty messages (NULL if the list is empty)
	 *
     * Params:
     *   none
     *
     * Returns:
     *   the first message envelope in the queue
     ***/
	MsgEnv* request_msg_env();

	 /***
     * Clear the envelope's contents, assign it to blocked process with highest priority, 
	 * or enqueue in _emptyMessages if there are no process waiting on envelopes
	 *
     * Params:
     *	msg_env_ptr: the message envelope to release
     *
     * Returns:
     *   integer error code
     ***/
	int release_msg_env(MsgEnv* msg_env_ptr);

	 /***
     * Set the sender and receiver pIDs in the envelope, and send the env. to the receiver
	 * Format: isSend;senderID;destinationID;messageType;timeStamp;
     * Params:
	 *	dst_process_id: the pID of the destination process
     *	msg_envelope: the message envelope to send
     *
     * Returns:
     *   integer error code
     ***/
	int send_message(int dest_process_id, MsgEnv* msg_envelope);

	/***
	* Lets the current process receive a message
	*
	* Params:
	*	none
	*
	* Returns:
	*	the first message in the current process' mailbox
	***/
	MsgEnv* receive_message();

	/***
	* Gets the history of the last 16 send/receive operations
	*
	* Params:
	*	message_envelope: the envelope to store the history in
	*
	* Returns:
	*	integer error code
	***/
	int get_trace_buffers(MsgEnv* message_envelope);

	/***
	* Frees all of the class' member variables
	*
	* Params:
	*	none
	*
	* Returns:
	*	integer error code
	***/
	int terminate();
};

#endif

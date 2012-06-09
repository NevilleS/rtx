#ifndef Scheduler_H
#define Scheduler_H

#include "RTXConstants.h"
#include "RTXStructures.h"
#include "List.h"
#include "InitializationTable.h"

/***
 * The Scheduler class provides process management facilities to the RTX. Whenever the RTX runs, the
 * Scheduler receives calls to dispatch(), which switches processes as necessary. The Scheduler
 * is responsible for managing the state of the processes and storing the PCBs intelligently in
 * various queues. Finally, the Scheduler provides functions to allow the RTX to change the status
 * of a given process, flagging them as ready or blocking their execution.
 ***/
class Scheduler {

private:
    // Array of PCB lists, indexed by priority
    List<PCB> _readyQueues[PRIORITY_COUNT];

    // Array of PCB lists, indexed by blocking type
    List<PCB> _blockedQueues[BLOCKED_QUEUE_COUNT];

    // Pointer to the PCB of the current process
    PCB* _currentPCB;
    
    // Array of PCBs, indexed by process ID. Populated during initialization
    PCB* _processTable[PROCESS_COUNT];

    /***
     * Find the next process to run, and remove it from its ready queue. This is
     * calculated by taking the first element of the highest priority ready queue. Due
     * to the inclusion of the null process, this will always find a process.
     * 
     * Params:
     *   none
     *
     * Returns:
     *   pointer to the PCB of the highest priority ready process
     ***/
    PCB* dequeueNextProcess();

    /***
     * Perform a context switch, changing the current running process to a new process. This
     * stores the current process' context, and resumes the execution of the new process. This
     * function is not responsible for managing the _currentPCB pointer or the PCB state, it
     * only switches the thread of execution.
     *
     * Params:
     *   currentPCB: pointer to the PCB of the currently executing process
     *   nextPCB: pointer to the PCB of the process to switch to
     *
     * Returns:
     *   void
     ***/
    void contextSwitch(PCB* currentPCB, PCB* nextPCB);

public:

    /***
     * Initialize the Scheduler. Called by the RTX on startup.
     *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
    int initialize();
 
    /***
     * Terminate the Scheduler. Called by the RTX on exit.
     *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
    int terminate();

    /***
     * Run the dispatcher, which finds and starts the execution of the next ready process. This function
     * should only be called after the current process has been blocked or released. Changes _currentPCB
     * to the next ready process using dequeueNextProcess, and updates its status accordingly.
     * Due to the nature of the context switching, this call will not return until the calling process
     * has been re-dispatched.
     *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
    int dispatch();

    /***
     * Makes a process ready, changing its status and adding it to the correct ready queue. If this
     * process is already ready, or running, this command is ignored.
     *
     * Params:
     *   readyPCB: pointer to the PCB to be made ready
     *
     * Returns:
     *   integer error code
     ***/
    int makeReady(PCB* readyPCB);

    /***
     * Blocks the currently executing process, changing its status and adding it to the correct blocked queue.
     * A call to dispatch() should follow this to actually perform the process switch.
     *
     * Params:
     *   blockStatus: blocking status for the PCB
     *
     * Returns:
     *   integer error code
     ***/
    int blockCurrentProcess(Status blockStatus);

    /***
     * Releases the currently executing process, changing its status and adding it to the correct ready queue.
     * A call to dispatch() should follow this to actually perform the process switch.
     *
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
    int releaseCurrentProcess();

    /***
     * Changes the priority of a process. This may or may not result in queues being re-organized.
     *
     * Params:
     *   newPriority: new priority value for the target process
     *   processID: process ID of the target process
     *
     * Returns:
     *   integer error code
     ***/
    int changePriority(int newPriority, int processID);

    /***
     * Retrieve the currently running process' PCB.
     *
     * Params:
     *   none
     *
     * Returns:
     *   pointer to the currently running process' PCB
     ***/
    PCB* getCurrentPCB();

    /***
     * Retrieve the next PCB from a specific blocked queue. Note that this does not remove the PCB from the queue.
     *
     * Params:
     *   blockStatus: the blocking status
     *
     * Returns:
     *   pointer to the next PCB from the specific blocked queue, or NULL, if the specific blocked queue is empty
     ***/
    PCB* getNextBlockedPCB(Status blockStatus);

    /***
     * Retrieve a process' PCB based on a process ID.
     *
     * Params:
     *   processID: process ID of the process whose PCB is required
     *
     * Returns:
     *   pointer to the PCB with the provided process ID, or NULL, if no PCB can be found
     ***/
    PCB* getPCB(int processID);

    /***
     * Attach a schedulable process' PCB to the process table. This does not explicitly make
     * the PCB ready - this should be done with the "makeReady" function. This function does not modify
     * the size of the processTable, it can only be used to populate it.
     *
     * Params:
     *   newPCB: pointer to the PCB of the newly schedulable process
     *
     * Returns:
     *   integer error code
     ***/
    int attachPCB(PCB* newPCB);

    /***
     * Retrieve the status of all processes and return them in the provided message envelope.
     *
     * Params:
     *   returnMsg: pointer to a message envelope to place the process statuses in
     *
     * Returns:
     *   integer error code
     ***/
    int requestProcessStatus(MsgEnv* returnMsg);
};

#endif

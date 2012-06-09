#include <setjmp.h>
#include <stdio.h>
#include "Scheduler.h"
#include "List.cpp"
#include "RTX.h"

extern RTX* g_RTX;

/***
 * Sort two PCB's based on priority. Used to arrange the blocked queues.
 ***/
PCB* prioritySort(PCB* current, PCB* toInsert) {
    // lower values of _priority indicate higher importance
    if(current->_priority <= toInsert->_priority) {
        return current;
    }
    else {
        return toInsert;
    }
}

PCB* Scheduler::dequeueNextProcess() {
    for(int i = 0; i < PRIORITY_COUNT; i++) {
        if(_readyQueues[i].getHead() != NULL) {
            return _readyQueues[i].dequeue();
        }
    }
    return NULL; // This should never happen, maybe auto return null process
}

void Scheduler::contextSwitch(PCB* currentPCB, PCB* nextPCB) {
    // Change execution from the current process to the process controlled by nextPCB
    if( setjmp(currentPCB->_context) == 0 )  {
        // Start execution of nextPCB
        longjmp(nextPCB->_context, 1);
    }
    // At this point, execution can be resumed
}

int Scheduler::initialize() {
    return SUCCESS;
}

int Scheduler::terminate() {
    for(int i=0; i < PROCESS_COUNT; i++) {
        delete _processTable[i];
    }
    return SUCCESS;
}

int Scheduler::dispatch() {
    /***
     * On the first dispatch, _currentPCB will be NULL. In this special case, the logic is slightly different.
     ***/
    if(_currentPCB == NULL) {
        _currentPCB = dequeueNextProcess();
        _currentPCB->_status = RUNNING;
        _currentPCB->_runCount++;
        jmp_buf tmpBuffer;
        if(setjmp(tmpBuffer) == 0) {
            longjmp(_currentPCB->_context,1);
        }
        return SUCCESS;
    }

    // Ensure that _currentPCB has been properly queued
    if(_currentPCB->_status == RUNNING) {
        return g_RTX->terminate(); // a serious error has occured, which may result in unexpected behaviour. Fail-fast
    }

    /***
     * Switch processes to the next process. When dispatch is called, the current process' status and storage will
     * have already been handled by the appropriate system call (makeReady, etc). The dispatcher only needs to find
     * a new process to run, update the _currentPCB variable and status, and perform the context switch.
     ***/
    PCB* oldPCB = _currentPCB;
    _currentPCB = dequeueNextProcess(); //removes it from the ready queue
    _currentPCB->_status = RUNNING;
    _currentPCB->_runCount++;
    contextSwitch(oldPCB, _currentPCB);
    return SUCCESS;
}

int Scheduler::makeReady(PCB* readyPCB) {
    // Check that the process is not the currently running process
    if(readyPCB == _currentPCB) {
        return INVALID_INPUT;
    }
    // Flag the process as READY and enqueue it in the correct queue
    readyPCB->_status = READY;
    _readyQueues[readyPCB->_priority].enqueue(readyPCB);
    return SUCCESS;
}

int Scheduler::blockCurrentProcess(Status blockStatus) {
    if(blockStatus > BLOCKED_MSG_RECEIVE) {
        return INVALID_INPUT;
    }
    else if(blockStatus == BLOCKED_MSG_RECEIVE) {
        _currentPCB->_status = blockStatus;
        return SUCCESS;
    }
    else {
        _currentPCB->_status = blockStatus;
        _blockedQueues[blockStatus].insert(_currentPCB, &prioritySort); // sort the blocked queues by priority
        return SUCCESS;
    }
}

int Scheduler::releaseCurrentProcess() {
    _currentPCB->_status = READY;
    _readyQueues[_currentPCB->_priority].enqueue(_currentPCB);
    return SUCCESS;
}

int Scheduler::changePriority(int newPriority, int processID) {
    // Validate input...
    if(newPriority < CRITICAL || newPriority >= NULL_PROCESS || processID >= PROCESS_COUNT) {
        return OUT_OF_BOUNDS;
    }
    PCB* changedPCB = getPCB(processID);
    int oldPriority = changedPCB->_priority;
    // Cannot change the priority of the null process
    if(oldPriority == NULL_PROCESS) {
        return INVALID_INPUT;
    }
    // If no change is required, return immediatly
    if(oldPriority == newPriority) {
        return SUCCESS;
    }
    else {
        changedPCB->_priority = newPriority;
    }
    // Inspect the PCB to find out where it is
    if(changedPCB->_status == RUNNING || changedPCB->_status == BLOCKED_MSG_RECEIVE) {
        // No shuffling required
        return SUCCESS;
    }
    else if(changedPCB->_status == READY) {
        // Switch ready queues
        _readyQueues[oldPriority].remove(changedPCB);
        _readyQueues[newPriority].enqueue(changedPCB);
        return SUCCESS;
    }
    else if(changedPCB->_status < BLOCKED_QUEUE_COUNT) {
        // Rearrange it in its blocked queue
        _blockedQueues[changedPCB->_status].remove(changedPCB);
        _blockedQueues[changedPCB->_status].insert(changedPCB, &prioritySort);
        return SUCCESS;
    }
    // PCB status is invalid
    return UNKNOWN_ERROR;
}


PCB* Scheduler::getCurrentPCB() {
    return _currentPCB;
}

PCB* Scheduler::getNextBlockedPCB(Status blockStatus) {
    // Check that the block status is valid, then return the PCB. Do not dequeue
    if(blockStatus < BLOCKED_QUEUE_COUNT) {
        return _blockedQueues[blockStatus].getHead();
    }
    else {
        return NULL;
    }
}

PCB* Scheduler::getPCB(int processID) {
    // Check that the processID is valid, then...
    if(processID < PROCESS_COUNT) {
        return _processTable[processID];
    }
    return NULL;
}

int Scheduler::attachPCB(PCB* newPCB) {
    // Check that the processID is valid, then update the processTable
    if(newPCB == NULL) {
        return NULL_POINTER;
    }
    if(newPCB->_processID < PROCESS_COUNT) {
         _processTable[newPCB->_processID] = newPCB;
         return SUCCESS;
    }
    return INVALID_INPUT;
}

int Scheduler::requestProcessStatus(MsgEnv* returnMsg) {
    char* dataPtr = returnMsg->_messageContents;
    dataPtr = dataPtr + sprintf(dataPtr, "Process ID    Process Name   Run Count    Priority    Status\n");
    for(int i=0; i<PROCESS_COUNT; i++) {
        PCB* nextPCB = getPCB(i);
        dataPtr = dataPtr + sprintf(dataPtr, "%-14d", nextPCB->_processID);
        dataPtr = dataPtr + sprintf(dataPtr, "%-15s", PROCESS_DESCS[nextPCB->_processID]);
        dataPtr = dataPtr + sprintf(dataPtr, "%-13d", nextPCB->_runCount);
        dataPtr = dataPtr + sprintf(dataPtr, "%-12s", PRIORITY_DESCS[nextPCB->_priority]);
        dataPtr = dataPtr + sprintf(dataPtr, "%s\n", STATUS_DESCS[nextPCB->_status]);
    }
    return SUCCESS;
}


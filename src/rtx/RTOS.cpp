#include "RTX.h"
#include "List.cpp"
#include "RTXStructures.h"
#include "RTXConstants.h"
#include "InitializationTable.h"

extern "C" int _set_sp(char* alt_stack);

#ifdef i386
#define SET_SP(alt_stack) __asm__ ( "movl %0, %%esp" :: "m" (alt_stack))
#endif
#ifdef __sparc
#define SET_SP(alt_stack) _set_sp(alt_stack)
#endif


RTX* g_RTX;
PostOffice* g_mailSystem;
Scheduler* g_scheduler;
IOHandler* g_ioHandler;
InterruptHandler* g_interruptHandler;
TimingService* g_timer;

int* max(int* A, int* B) {
    if( (*A) >= (*B) ) {
        return A;
    }
    else {
        return B;
    }
}

void initialize() {
    //allocate and assemble the kernel from its components
    g_RTX = new RTX();
    g_mailSystem = new PostOffice();
    g_scheduler = new Scheduler();
    g_ioHandler = new IOHandler();
    g_interruptHandler = new InterruptHandler();
    g_timer = new TimingService();
    g_RTX->attach(g_mailSystem, g_scheduler, g_ioHandler, g_interruptHandler, g_timer);

    //initialize RTX kernel
    g_RTX->initialize();

    //initialize processes
    g_interruptHandler->atomic(true);
    for(int i=0; i < PROCESS_COUNT; i++) {
        PCB* newPCB = new PCB(INIT_TABLE[i]);
        g_scheduler->attachPCB(newPCB);
        g_scheduler->makeReady(newPCB);
        jmp_buf tempBuffer;
        if(setjmp(tempBuffer)==0) {
            char* stackPtr = newPCB->_stackPtr + STACK_SIZE;
            SET_SP(stackPtr);
            if(setjmp(newPCB->_context) == 0) {
                // Context is initialized, continue with next PCB
                longjmp(tempBuffer,1);
            }
            else {
                PCB* currentPCB = g_scheduler->getCurrentPCB();
                g_interruptHandler->atomic(OFF); // disable atomicity
                (*currentPCB->_processPtr)();
                exit(0);
            }
        }
    }
    g_interruptHandler->atomic(false);
}

void execute() {
    g_RTX->execute();
}

int main(){
    initialize();
    execute();
    return 0;
}

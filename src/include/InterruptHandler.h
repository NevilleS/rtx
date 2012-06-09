#ifndef InterruptHandler_H
#define InterruptHandler_H

#include <sys/types.h>
#include <signal.h>

class InterruptHandler{
private:
	sigset_t _blocked; //the signal mask to ignore signals (for atomic operations)
	sigset_t _unblocked; //regular signal mask
	bool _signalsBlocked; //atomic state
public:
	void initialize(); //initialize signals
	void atomic(bool state); //toggle atomicity
};

#endif

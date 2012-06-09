#include "InterruptHandler.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "RTX.h"
#include "IOHandler.h"
#include "TimingService.h"
#include "RTXConstants.h"

extern RTX* g_RTX;
extern IOHandler* g_ioHandler;
extern TimingService* g_timer;
extern InterruptHandler* g_interruptHandler;


////////////////////////
// Interrupt Handlers
////////////////////////
void catchINT(int signal){
	//g_interruptHandler->atomic(ON);
	g_RTX->terminate();
}

void catchTERM(int signal){
	//g_interruptHandler->atomic(ON);	
	g_RTX->terminate();
}

void catchSEG(int signal){
	//g_interruptHandler->atomic(ON);
	g_RTX->terminate();
}

void catchUSR1(int signal){
	//g_interruptHandler->atomic(ON);
	g_ioHandler->getKeyboardReceive();
	//g_interruptHandler->atomic(OFF);
}

void catchUSR2(int signal){
	//g_interruptHandler->atomic(ON);
	g_ioHandler->sendDisplayReceive();
	//g_interruptHandler->atomic(OFF);
}

void catchALRM(int signal){
	//g_interruptHandler->atomic(ON);
	g_timer->tick();
	alarm(1);
	//g_interruptHandler->atomic(OFF);
}


////////////////////////
// Class Functions
////////////////////////

void InterruptHandler::initialize(){
	// set unblocked mask to block nothing
	sigemptyset(&this->_unblocked);

	// set blocked mask necessary signals
	sigemptyset(&this->_blocked);
	sigaddset(&this->_blocked, SIGALRM);
	sigaddset(&this->_blocked, SIGUSR1);
	sigaddset(&this->_blocked, SIGUSR2);
	sigaddset(&this->_blocked, SIGTERM);
	sigaddset(&this->_blocked, SIGINT);

	
	// setup sig action structure for each signal
	struct sigaction USR1;
	USR1.sa_handler = &catchUSR1;
	sigemptyset(&USR1.sa_mask);
	USR1.sa_flags = SA_RESTART;

	struct sigaction USR2;
	USR2.sa_handler = &catchUSR2;
	sigemptyset(&USR2.sa_mask);
	USR2.sa_flags = SA_RESTART;

	struct sigaction TERM;
	TERM.sa_handler = &catchTERM;
	sigemptyset(&TERM.sa_mask);
	TERM.sa_flags = SA_RESTART;

	struct sigaction INT;
	INT.sa_handler = &catchINT;
	sigemptyset(&INT.sa_mask);
	INT.sa_flags = SA_RESTART;

	struct sigaction ALRM;
	ALRM.sa_handler = &catchALRM;
	sigemptyset(&ALRM.sa_mask);
	ALRM.sa_flags = SA_RESTART;

	struct sigaction SEG;
	SEG.sa_handler = &catchSEG;
	sigemptyset(&SEG.sa_mask);
	SEG.sa_flags = SA_RESTART;
	
	// identify signal handlers to UNIX
	sigaction(SIGUSR1, &USR1, NULL);
	sigaction(SIGUSR2, &USR2, NULL);
	sigaction(SIGTERM, &TERM, NULL);
	sigaction(SIGINT, &INT, NULL);
	sigaction(SIGALRM, &ALRM, NULL);
	sigaction(SIGSEGV, &SEG, NULL);

	// enter atomic state
	sigprocmask(SIG_SETMASK, &this->_blocked, NULL);	
	this->_signalsBlocked=true;
}

void InterruptHandler::atomic(bool state){
	if((state == true)&&(this->_signalsBlocked==false)){
		sigprocmask(SIG_SETMASK, &this->_blocked, NULL);
		this->_signalsBlocked = true;
	}
	else if((state == false)&&(this->_signalsBlocked==true)){
		sigprocmask(SIG_SETMASK, &this->_unblocked, NULL);
		this->_signalsBlocked = false;
	}
}

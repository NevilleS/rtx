#include <stdlib.h>
#include <stdio.h>
#include "RTX.h"

int RTX::initialize() {
	//Initialize the Intertupt Handler and turn on atomicity
	_interruptHandler->initialize();
	_interruptHandler->atomic(ON);
	//Initialize the rest of the OS's main components
	_ioHandler->initialize();
	_mailSystem->initialize();
	_timer->initialize();
    _scheduler->initialize();
	//Turn off atomicity and return success if there were no errors
	_interruptHandler->atomic(OFF);
	return SUCCESS;
}

int RTX::execute() {
    _scheduler->dispatch();
    terminate();
	return UNKNOWN_ERROR;
}

int RTX::terminate() {
	//Turn on atomicity
	_interruptHandler->atomic(ON);
	//Terminate all of the RTX's member variables, except the interrupt handler
	_mailSystem->terminate();
	_timer->terminate();
	_ioHandler->terminate();
	_scheduler->terminate();
	//Turn off atomictiy and then terminate the interrupt handler
	_interruptHandler->atomic(OFF);
	//Free all of the RTX's member variables
	delete _interruptHandler;
	delete _mailSystem;
	delete _timer;
	delete _ioHandler;
	delete _scheduler;
	//Exit
	exit(0);
	return UNKNOWN_ERROR;
}

int RTX::attach(PostOffice* mailSystem, Scheduler* scheduler, IOHandler* ioHandler,
                InterruptHandler* interruptHandler, TimingService* timer) {
    _interruptHandler = interruptHandler;
    _mailSystem = mailSystem;
    _timer = timer;
    _ioHandler = ioHandler;
    _scheduler = scheduler;
    return SUCCESS;
}

MsgEnv* RTX::receive_message(){
	MsgEnv* msgEnvelope = NULL;
	while (msgEnvelope == NULL){
		_interruptHandler->atomic(ON);
		msgEnvelope = _mailSystem->receive_message();
		if (msgEnvelope == NULL){
			_scheduler->blockCurrentProcess(BLOCKED_MSG_RECEIVE);
			_scheduler->dispatch();
		}
		_interruptHandler->atomic(OFF);
	}
	return msgEnvelope;
}

MsgEnv* RTX::request_msg_env(){
	MsgEnv* msgEnvelope = NULL;
	while (msgEnvelope == NULL){
		_interruptHandler->atomic(ON);
		msgEnvelope = _mailSystem->request_msg_env();
		if (msgEnvelope == NULL){
			_scheduler->blockCurrentProcess(BLOCKED_MSG_ALLOC);
			_scheduler->dispatch();
		}
		_interruptHandler->atomic(OFF);
	}
	return msgEnvelope;
}

int RTX::release_processor() {
	_interruptHandler->atomic(ON);
    _scheduler->releaseCurrentProcess();
    _scheduler->dispatch();
	_interruptHandler->atomic(OFF);
	return SUCCESS;	
}

int RTX::send_message(int dest_process_id, MsgEnv* msg_envelope){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _mailSystem->send_message(dest_process_id, msg_envelope);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::request_process_status(MsgEnv* msg_env_ptr){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _scheduler->requestProcessStatus(msg_env_ptr);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::change_priority(int new_priority, int target_process_id){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _scheduler->changePriority(new_priority, target_process_id);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::send_console_chars(MsgEnv* message_envelope){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _ioHandler->sendDisplayRequest(message_envelope);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::get_console_chars(MsgEnv* message_envelope){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _ioHandler->getKeyboardRequest(message_envelope);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::get_trace_buffers(MsgEnv* message_envelope){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _mailSystem->get_trace_buffers(message_envelope);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::request_delay(int time_delay, int wakeup_code, MsgEnv* message_envelope){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _timer->requestDelay(time_delay, wakeup_code, message_envelope);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::release_msg_env(MsgEnv* msg_env_ptr){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _mailSystem->release_msg_env(msg_env_ptr);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::display_clock(bool display){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _timer->displayClock(display);
	_interruptHandler->atomic(OFF);
	return retVal;
}

int RTX::set_clock(int second, int minute, int hour){
	int retVal;
	_interruptHandler->atomic(ON);
	retVal = _timer->setClock(second, minute, hour);
	_interruptHandler->atomic(OFF);
	return retVal;
}

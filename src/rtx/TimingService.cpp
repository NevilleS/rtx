#include <stdlib.h>
#include "TimingService.h"
#include "RTXStructures.h"
#include "RTXConstants.h"
#include "List.cpp"
#include "RTX.h"
#include "IOHandler.h"
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

extern RTX* g_RTX;
extern IOHandler* g_ioHandler;

TimeRequest* timeRequestSort(TimeRequest* current, TimeRequest* toInsert){
	if(current->_time <= toInsert->_time) {
		toInsert->_time = toInsert->_time - current->_time;
        return current;
    }
    else {
		current->_time = current->_time - toInsert->_time;
        return toInsert;
    }
}

int TimingService::addRequest(int delay, int sender, int wakeUpCode, MsgEnv* message){
	TimeRequest* req = new TimeRequest;
	req->_time = delay;
	req->_processID = sender;
	req->_wakeUpCode = wakeUpCode;
	req->_message = message;
	_requests.insert(req, &timeRequestSort);
	return SUCCESS;
}

void TimingService::checkPendingReq(){
	// check list to see if _relativeTime == time of first item in list
	// if so, dequeue first item and send requested message using postal service
	if(_requests.getHead() != NULL){
		TimeRequest* first = _requests.getHead();
		if(_relativeClock >= first->_time){
			TimeRequest* request = _requests.dequeue();
			MsgEnv* requestedMessage = request->_message;
			_relativeClock=0;
			g_RTX->send_message(requestedMessage->_destinationID, requestedMessage);
		}
	}
}

int TimingService::requestDelay(int delay, int wakeUpCode, MsgEnv* message){
	if(delay > 0 && message != NULL)
		return addRequest(delay, message->_senderID, wakeUpCode, message);
	else
		return UNKNOWN_ERROR;
}

void TimingService::tick(){
	_absoluteClock++;
	if(_requests.getHead() == NULL)
		_relativeClock=0;
	else
		_relativeClock++;
	if (_absoluteClock % TICKS_PER_SECOND == 0){
		incrementClock();
	}
	checkPendingReq();
}
	
int TimingService::initialize(){
	_absoluteClock=0;
	_relativeClock=0;
	_hour=0;
	_min=0;
	_sec=0;
	alarm(1);
	_displayClock=true;
	g_ioHandler->sendDisplayTime(_hour,_min,_sec);
	return SUCCESS;
}

void TimingService::getTime(int* sec, int* min, int* hour){
	*sec = _sec;
	*min = _min;
	*hour = _hour;
}

void TimingService::incrementClock(){
	if (_sec < 59){
		_sec++;
	}
	else if (_min < 59){
		_sec = 0;
		_min++;
	}
	else if (_hour < 23){
		_sec = 0;
		_min = 0;
		_hour++;
	}
	else{
		_sec = 0;
		_min = 0;
		_hour = 0;
	}
	if(this->_displayClock == true)
		g_ioHandler->sendDisplayTime(_hour, _min, _sec);
	else
		g_ioHandler->sendDisplayTime(25,0,0);
}

int TimingService::setClock(int sec, int min, int hour){
	_sec = sec;
	_min = min;
	_hour = hour;
	if(this->_displayClock == true)
		g_ioHandler->sendDisplayTime(_hour, _min, _sec);
	return SUCCESS;
}

int TimingService::displayClock(bool display){
	_displayClock = display;
	return SUCCESS;
}

int TimingService::terminate(){
	while(_requests.getCount()>0)
		delete _requests.dequeue();
	return SUCCESS;
}

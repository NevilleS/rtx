#ifndef TimingService_H
#define TimingService_H

#include "RTXConstants.h"
#include "RTXStructures.h"
#include "List.h"

class TimingService{
private:
	// Relative clock (in clock ticks) (relative to last time request completion)
	int _relativeClock;

	// Absolute clock (in clock ticks) (from startup)
	int _absoluteClock;

	// Real world clock variables
	int _sec;
	int _min;
	int _hour;

	// Boolean indicating wether or not to display clock
	bool _displayClock;

	// List of time requests
	List<TimeRequest> _requests;

	/***
     * Add a request to the queue which is sorted chronologically by expiry of request.  Time
	 * stored with request is equal to time between expiry of previous request and request in
     * question.
	 * 
     * Params:
     *   (length of) delay, (ID of) sender, wakeUpCode, message (to be sent on req expiry)
     *
     * Returns:
     *   Integer error code
     ***/
	int addRequest(int delay, int sender, int wakeUpCode, MsgEnv* message);

	/***
     * Check time value of first item in request list against the relative.  If they are equal
	 * dequeue message and send it where requested.  Also, reset relative clock whenver this occurs
	 * [Note: function does not check that lsit is empty, that occurs in public function tick()]
	 * 
     * Params:
     *   none
     *
     * Returns:
     *   nothing (void)
     ***/
	void checkPendingReq();

	/***
     * Increment (24 hr) clock by one second (by adjusting all clock values as necessary)
	 * 
     * Params:
     *   none
     *
     * Returns:
     *   nothing (void)
     ***/
	void incrementClock();

public:
	/***
     * Allows function to request that a message (it defines) be sent to it after a certain
	 * period of time has elapsed.  (Basically just repackages and send to private function
	 * addRequest(...)
	 * 
     * Params:
     *   (length of) delay, wakeUpCode, message (to be sent on req expiry)
     *
     * Returns:
     *   integer error code
     ***/
	int requestDelay(int delay, int wakeUpCode, MsgEnv* message);

	/***
     * Function called by Interrupt handler on each SIGALRM.  Completes the following tasks:
	 * 1) Update absolute and relative clock
	 * 2) Advance real world clock if specific tick corresponds to elapse of a second
	 * 3) If request list is not empty, call checkPendingReq()
	 * 
     * Params:
     *   none
     *
     * Returns:
     *   nothing (void)
     ***/
	void tick();
	
	/***
     * Places time components into the three locations pointed to in the function parameters
	 * 
     * Params:
     *   pointers to location to put hour, minute, and second portions of time
     *
     * Returns:
     *   nothing (void)
     ***/
	void getTime(int* sec, int* min, int* hour);
	
	/***
     * initializes all clock values to zero
	 * 
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
	int initialize();

	/***
     * Set clock to specific time (validity of time not checked, that's done in CCI)
	 * 
     * Params:
     *   time components (hour, minute and second)
     *
     * Returns:
     *   integer error code
     ***/
	int setClock(int sec, int min, int hour);
	
	/***
     * Simple mutator for _display clock boolean variable
	 * 
     * Params:
     *   display
     *
     * Returns:
     *   nothing (void)
     ***/
	int displayClock(bool display);
	
	/***
     * Clean up all requests sitting in the request queue
	 * 
     * Params:
     *   none
     *
     * Returns:
     *   integer error code
     ***/
	int terminate();

    /***
     * Return the current display status of the clock.
     *
     * Params:
     *  none
     *
     * Returns:
     *  current display status (true = displayed)
     ***/
	bool fetchBool(){return _displayClock;}
};

#endif

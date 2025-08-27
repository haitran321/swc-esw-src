/**
* $Id: EventProcessor.cpp
*/
#include <iostream>
//#include <ioLib.h>
//#include <errnoLib.h>
#include <errno.h>
//#include <msgQEvLib.h>
//#include <pipeDrv.h>
#include <sstream>
#include "EventProcessor.h"
// #include "EventProcessorMgr.h"
// #include "UTCTimeUtils.h"
// #include "ElapsedTimer.h"
// #include "WVEvents.h"

#include <string.h>

using namespace std;

/** Shutdown device event priority */

const int shutdownPriority = 10;

EventProcessor::EventProcessor() : 
_terminated(false),
_fdSize(-1),
_totalEventsProcessed(0),
_selectEarlyExitFailure(false),
_totalExecTime(0),
_avgExecTime(0),
_minExecTime(0),
_maxExecTime(0),
_selTimeoutValue(0),
_ticksTimeout(60e6),
_timeoutSet(false),
_selEarlyExitCount(0),
_name("TEST")
//_logEvPrcr(NULL)
{
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	FD_ZERO(&_statusSet);
	// EventProcessorMgr::getInstance().addEventProcessor(this);

	// Initialize select timeout to 1 second.

	_selectTimeout.tv_sec = 10;
	_selectTimeout.tv_usec = 0;
	_selTimeoutValue = static_cast<double>(_selectTimeout.tv_sec) +
	(static_cast<double>(_selectTimeout.tv_usec) / 1000);
}

EventProcessor::~EventProcessor()
{
	try
	{
		// Instruct event processor task to terminate, so that when it
		// breaks out of the wait loop, it does not try to process another
		// loop. This can cause a call to the virtual method processError
		// which is used for normal error processing.

		terminate();

		// Clear event list.

//		clearEventList();

		// Remove event processor from event processor manager.

		// EventProcessorMgr::getInstance().deleteEventProcessor(this);
	}
	catch (...)
	{
	}
}

STATUS EventProcessor::start()
{
	while (!_terminated)
	{
		// Wait on select.

		waitOnSelect();
	}

	return OK;

}

void EventProcessor::dispatchEvent(DeviceEvent *event)
{
	// Increment event occurrence.

	event->incCount();

	// Process read event.

	switch (event->getEventDispatchType())
	{
		case DispatchToEventFunc:

			(this->*(event->getEventFunc()))();
			break;

		case DispatchToEventFuncInt:

			(this->*(event->getEventFuncInt()))(event->getEventParam());
			break;

		default:

			break;
	}
}

void EventProcessor::waitOnSelect()
{
	fd_set readSet;
	fd_set writeSet;
	fd_set statusSet;
	int numdesc;
	double elapsedTime = 0.0;

	// Initialize device descriptors to wait on.

	memcpy(&readSet, &_readSet, sizeof(readSet));
	memcpy(&writeSet, &_writeSet, sizeof(writeSet));
	memcpy(&statusSet, &_statusSet, sizeof(statusSet));

	// Time the select call in attempt to trap select non-error-errors.

//	_execTimer.start();

	// Wait for next event to occur.

	// printf("Wait for next event to occur.\n");

//	if ((numdesc = select(_fdSize+1, &readSet, &writeSet, &statusSet, &_selectTimeout)) == ERROR)
	if ((numdesc = select(_fdSize+1, &readSet, &writeSet, &statusSet, 0)) == ERROR)
	{
		//processError(errnoGet());
		return;
	}

	// Increment total events occurred and check for select to have early-exited.

	_totalEventsProcessed++;
//	elapsedTime = _execTimer.getElapsedTimeSecs();

	if ((numdesc == 0) && //no events detected, and
		(elapsedTime < _selTimeoutValue)) //select returned before timing out.
	{
		if (!_selectEarlyExitFailure)
		{
			_selEarlyExitCount++;
			if (_selEarlyExitCount > 5)
			{
				_selectEarlyExitFailure = true;
			}
		}
	}

	if (numdesc == 0)
	{
		if (_timeoutSet)
		{
			// Derived event processor set timeout, so call timeout
			// processing.

			processWaitTimeout();
		}
	}
	else
	{
		int priorityGroupProcessed = -1;                    // Variable to hold priority group being processed. (-1 = none)
		for (EventMap::iterator iter = _eventList.begin();  // Step through registered Event items.
			iter != _eventList.end(); iter++)
		{
			int priority = (*iter).first;           // Get priority level
			DeviceEvent *event = (*iter).second;            // and event information for current event item.
			if (priorityGroupProcessed != -1 &&             // If we have already processed a group and the current item
				priorityGroupProcessed != priority)         // is not in the previously processed group then stop event processing.
			{                                               // NOTE: EventMaps keys are sorted during storage. This means that we automatically
				break;                                      // go from highest priority (1) to lowest while iterating through the map.
			}

			// Dispatch according to event.

			switch (event->getType())
			{
				case READ_EVENT:

					if (FD_ISSET(event->getFd(), &readSet))     // Test to see if FD is set (Input Pending). Drivers "should" clear this after any reads which empty the buffer
					{                                           // thereby allowing a while() type loop. However, our internal drivers do not do this properly. So, we have to do
						// Dispatch event.                      // a single read then let the next select() call poll the driver for the proper state.
						// _logEvPrcr->logTrace("Dispatched.");
						dispatchEvent(event);
						priorityGroupProcessed = priority;      // Save priority of current event.
					}
					break;

				case WRITE_EVENT:

					if (FD_ISSET(event->getFd(), &writeSet))     // Our internal write drivers do not buffer pending writes so there is no need to perform a while() statement.
					{                                            // Doing so would not work since our internal drivers do not handle FD sets properly.
						// Dispatch event.
						//_logEvPrcr->logTrace("Dispatched.");
						dispatchEvent(event);
						priorityGroupProcessed = priority;      // Save priority of current event.
					}
					break;

				case STATUS_EVENT:

					if (FD_ISSET(event->getFd(), &statusSet))    // We have hijacked the "exception" event to use as a "status" event. Mainly used
					{                                            // by our internal timers. Again, driver does not reset FD state properly so just
						// Dispatch event.                       // "read and go".
						//_logEvPrcr->logTrace("Dispatched.");
						dispatchEvent(event);
						priorityGroupProcessed = priority;      // Save priority of current event.
					}
					break;

				default:

					printf("Invalid event type: %d\n", event->getType());
					break;
			}
		}
	}

	// Calculate execution time for this event.

//	elapsedTime = _execTimer.getElapsedTimeSecs() - elapsedTime;

	// Update stats.

	if (_minExecTime == 0.0)
	{
		_minExecTime = elapsedTime;
		_maxExecTime = elapsedTime;
	}
	else if (elapsedTime < _minExecTime)
	{
		_minExecTime = elapsedTime;
	}
	else if (elapsedTime > _maxExecTime)
	{
		_maxExecTime = elapsedTime;
	}
	_totalExecTime += elapsedTime;
}

//void EventProcessor::threadBody()
//{
//	// Enter processing loop, looping until requested to terminate.
//
//	 while (!_terminated)
//	 {
//	 	try
//	 	{
//	 		// Wait on select.
//
//	 		waitOnSelect();
//	 	}
//	 	catch (string &s)
//	 	{
//	 		printf("Unhandled exception has occurred in %s (%s)\n", _name.c_str(),
//	 			s.c_str());
//	 	}
//	 	catch (...)
//	 	{
//	 		printf("Unhandled exception has occurred in %s\n", _name.c_str());
//	 	}
//	 }
//
//}

//void EventProcessor::cleanup()
//{
//	// clearEventList();
//
//	// // Close and delete shutdown pipe.
//
//	// _shutdownDev.close();
//	// string pipeName = "/pipe/shutdown_" + _name;
//	// if (pipeDevDelete(pipeName.c_str(), 0) == ERROR)
//	// {
//	// 	printf("Error deleting shutdown pipe %s\n", pipeName.c_str());
//	// }
//
//	// // Perfrom clean up processing in base class.
//
//	// Thread::cleanup();
//}

void EventProcessor::printEventList()
{
	printf("_totalEventsProcessed = %u\n", _totalEventsProcessed);
	printf("_selectEarlyExitFailure = %s\n",
		(_selectEarlyExitFailure ? "true" : "false"));
	printf("_totalExecTime = %.6f\n", _totalExecTime);
	printf("_minExecTime = %.6f\n", _minExecTime);
	printf("_maxExecTime = %.6f\n", _maxExecTime);
	if (_totalEventsProcessed != 0)
	{
		_avgExecTime = _totalExecTime / _totalEventsProcessed;
	}
	else
	{
		_avgExecTime = 0;
	}
	printf("_avgExecTime = %.6f\n", _avgExecTime);
	printf("_fdSize = %d\n\n", _fdSize);
	printf("fd     (device name)                    type  pri  count\n");
	printf("--- ---------------------------------- ------ ---- -----\n");

	for (EventMap::iterator iter = _eventList.begin();
		iter != _eventList.end(); iter++)
	{
		DeviceEvent *event = (*iter).second;
		string typeString;
		if (event->getType() == READ_EVENT)
		{
			typeString = "READ";
		}
		else if (event->getType() == WRITE_EVENT)
		{
			typeString = "WRITE";
		}
		else
		{
			typeString = "STATUS";
		}

		printf("%-3d %-34s %-6s %3d %6d\n",
			event->getFd(), event->getName().c_str(),
			typeString.c_str(), event->getPriority(),
			event->getCount());
	}
}

STATUS EventProcessor::addEvent(DeviceEvent *event)
{
	// Add DeviceEvent object to event list.

	_eventList.insert(EventMapPair(event->getPriority(), event));

	// Add fd to appropriate select fd set.

	int fd = event->getFd();

	// Validate fd.

	if (fd == NO_FD)
	{
		return(ERROR);
	}

	// Add fd to appropriate event set.

	switch (event->getType())
	{
		case READ_EVENT:

			FD_SET(fd, &_readSet);
			if (fd > _fdSize)
			{
				_fdSize = fd;
			}
			break;

		case WRITE_EVENT:

			FD_SET(fd, &_writeSet);
			if (fd > _fdSize)
			{
				_fdSize = fd;
			}
			break;

		case STATUS_EVENT:

			FD_SET(fd, &_statusSet);
			if (fd > _fdSize)
			{
				_fdSize = fd;
			}
			break;

		default:

			return(ERROR);
			break;
	}

	return(OK);

}

STATUS EventProcessor::addEvent(
	Device &device, EventType eventType, int priority,
	EventFunc eventFunc)
{
	// Allocate memory for DeviceEvent object.

	DeviceEvent *event = NULL;
	try
	{
		event = new DeviceEvent(device, eventType, priority, eventFunc);
	}
	catch (std::bad_alloc&)
	{
		return(ERROR);
	}

	// Add event to event list.

	return(addEvent(event));
}

STATUS EventProcessor::addEvent(
	Device &device, EventType eventType, int priority,
	EventFuncInt eventFuncInt, int eventParam)
{
	// Allocate memory for DeviceEvent object.

	DeviceEvent *event = NULL;
	try
	{
		event = new DeviceEvent(device, eventType, priority,
			eventFuncInt, eventParam);
	}
	catch (std::bad_alloc&)
	{
		return(ERROR);
	}

	// Add event to event list.

	return(addEvent(event));
}

STATUS EventProcessor::deleteEvent(Device &device, EventType eventType)
{
	STATUS status = ERROR;

	// Remove event from list.

	for (EventMap::iterator iter = _eventList.begin();
		iter != _eventList.end(); iter++)
	{
		DeviceEvent *event = (*iter).second;
		if (event->getFd() == device.getFd() && event->getType() == eventType)
		{
			// Remove fd from approprite select set.

			int fd = event->getFd();
			if (fd != NO_FD)
			{
				switch (eventType)
				{
					case READ_EVENT:

						FD_CLR(fd, &_readSet);
						break;

					case WRITE_EVENT:

						FD_CLR(fd, &_writeSet);
						break;

					case STATUS_EVENT:

						FD_CLR(fd, &_statusSet);
						break;

					default:

						break;
				}
			}

			// Delete event object and remove from list.

			delete event;
			event = NULL;
			_eventList.erase(iter);
			status = OK;
			break;
		}
	}

	// Reset _fdSize

	_fdSize = -1;
	for (EventMap::iterator iter = _eventList.begin();
		iter != _eventList.end(); iter++)
	{
		DeviceEvent *event = (*iter).second;
		if (event->getFd() > _fdSize)
		{
			_fdSize = event->getFd();
		}
	}
	return(status);
}

void EventProcessor::terminate()
{
	_terminated = true;
}

//void EventProcessor::printInfo()
//{
//	printf("printInfo is not defined for event processor %s\n", getName().c_str());
//}

void EventProcessor::processWaitTimeout()
{
	printf("processWaitTimeout is not defined for event processor %s\n", getName().c_str());
}

//STATUS EventProcessor::setSelectTimeout(int msec)
//{
//	//return setSelectTimeout(TimeValue(msec, TimeValue::Milliseconds));
//	return setSelectTimeout(100);
//}

STATUS EventProcessor::setSelectTimeout(int timeout)
//STATUS EventProcessor::setSelectTimeout(const TimeValue& timeout)
{
//	_selectTimeout.tv_sec = timeout.convertTo<long>(TimeValue::Seconds);
//	_selectTimeout.tv_usec = (timeout -
//		TimeValue(_selectTimeout.tv_sec, TimeValue::Seconds)).convertTo<long>(
//		TimeValue::Microseconds);
//
//	_selTimeoutValue = timeout.convertTo<double>(TimeValue::Seconds);

	_selectTimeout.tv_sec = 1;
	_selectTimeout.tv_usec = 100;
	_selTimeoutValue = 100;


	if (_selTimeoutValue == 0)
	{
		_timeoutSet = false;
	}
	else
	{
		_timeoutSet = true;
	}

	return OK;
}

void EventProcessor::addCounter(int counterId)
{
	_counters[counterId] = 0;
}

void EventProcessor::incCounter(int counterId)
{
	++_counters[counterId];
}

void EventProcessor::resetCounter(int counterId)
{
	_counters[counterId] = 0;
}

int EventProcessor::getCounter(int counterId)
{
	return _counters[counterId];
}

//void EventProcessor::processError(int errnoValue)
//{
//	char strErrorMsg[256];
//
//	// Log/Output error message and event listing.
//
////	snprintf(strErrorMsg, 256, "Select failed in %s with errno=%s (%d). "
////		"Terminating this thread.",
////		getName().c_str(), strerror(errnoGet()), errnoGet());
//	//_logEvPrcr->logError(strErrorMsg);
//	printf("\n%s\n", strErrorMsg);
//	printEventList();
//
//	// Attempt to determine which device is causing the problem.
//
////	if (errnoGet() == EBADF)
////	{
////		for (EventMap::iterator iter = _eventList.begin();
////			iter != _eventList.end(); iter++)
////		{
////			DeviceEvent *event = (*iter).second;
////
////			// Perform a bogus ioctl call to see if fd is in system
////			// descriptor table. If it is no longer in descriptor table,
////			// it will fail with errno = EBADF.
////
////			int value;
////			if (ioctl(event->getFd(), FIONREAD, (int)&value) == ERROR)
////			{
////				if (errnoGet() == EBADF)
////				{
////					printf("\n\nFailure occured for device = %s fd = %d\n",
////						event->getName().c_str(), event->getFd());
////				}
////			}
////		}
////	}
//
//	// Terminate thread because we are in an unrecoverable situation. This
//	// task will ultimately be detected as being inactive.
//
//	terminate();
//}

Device* EventProcessor::createDevice(const std::string& name, DeviceFactory::Mode mode,
	EventFunc callback, EventType eventType, int priority)
// Device* EventProcessor::createDevice(const std::string& name, int mode,
// 	EventFunc callback, EventType eventType, int priority)
{
	printf("****in EventProcessor::createDevice****\n");
	// create the device
	// Device* device = DeviceFactory::getInstance().createDevice(name.c_str(), mode);
	Device* device = new Device(name.c_str());
	if (device == NULL)
	{
//		fprintf(stderr, "%s: Failed to create device '%s': %s\n",
//			getName().c_str(), name.c_str(), strerror(errnoGet()));
		printf("ERROR:  Failed to create device '%s': %s\n", getName().c_str(), name.c_str());
		return NULL;
	}

	// open it
	if (device->open() != OK)
	{
//		fprintf(stderr, "%s: Failed to open device '%s': %s\n",
//			getName().c_str(), device->getName().c_str(), strerror(errnoGet()));
		printf("ERROR:  Failed to open device '%s': %s\n", getName().c_str(), name.c_str());
		delete device;
		return NULL;
	}

	// if
	if (callback)
	{
		if (addEvent(*device, eventType, priority, callback) != OK)
		{
//			fprintf(stderr, "%s: Failed to add event for '%s' to '%s'\n",
//				getName().c_str(), device->getName().c_str(), getName().c_str());
			printf("ERROR:  Failed to add event for '%s': %s\n", getName().c_str(), name.c_str());
			device->close();
			delete device;
			return NULL;
		}
	}

	return device;
}

// TimerDevice* EventProcessor::createTimer(const string& name, EventFunc callback, TimeValue timeout, int priority, bool start)
// {
// 	TimerDevice* timer = DeviceFactory::getInstance().createTimerDevice(name);
// 	if (timer == NULL)
// 	{
// 		fprintf(stderr, "%s: Failed to create timer\n", getName().c_str());
// 		return NULL;
// 	}

// 	if (timer->open() != OK)
// 	{
// 		fprintf(stderr, "%s: Failed to open timer %s: %s\n", getName().c_str(),
// 			timer->getName().c_str(), strerror(errnoGet()));
// 		delete timer;
// 		return NULL;
// 	}

// 	if (addEvent(*timer, STATUS_EVENT, priority, callback) != OK)
// 	{
// 		fprintf(stderr, "%s: Failed to add timer event handler\n", getName().c_str());
// 		delete timer;
// 		return NULL;
// 	}

// 	timer->setTimeout(timeout);

// 	if (start)
// 	{
// 		if (timer->start() != OK)
// 		{
// 			fprintf(stderr, "%s: Failed to start timer with period of %dms\n",
// 				getName().c_str(),
// 				static_cast<int>(timeout.convertTo(TimeValue::Milliseconds)));
// 			deleteEvent(*timer, STATUS_EVENT);
// 			delete timer;
// 			return NULL;
// 		}
// 	}

// 	return timer;
// }

// TimerDevice* EventProcessor::createTimer(EventFunc callback, TimeValue timeout, int priority, bool start)
// {
// 	return createTimer("", callback, timeout, priority, start);
// }

// void EventProcessor::clearEventList()
// {
// 	for (EventMap::iterator iter = _eventList.begin();
// 		iter != _eventList.end(); iter++)
// 	{
// 		delete (*iter).second;
// 		(*iter).second = NULL;
// 	}
// 	_eventList.clear();
// }

void EventProcessor::processShutdownNotification()
{
	// Set terminated flag, so thread will terminate when it returns to its
	// select loop.

	terminate();
}

//void EventProcessor::initiateTermination()
//{
//	// Generate a read event on the shutdown pipe for this event processor.
//
//	int dummy = 0;
//	if (_shutdownDev.write(&dummy, sizeof(dummy)) == ERROR)
//	{
//		printf("Error writing to shutdown pipe for %s\n", getName().c_str());
//	}
//}

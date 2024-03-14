#ifndef TIMER_H
#define TIMER_H

// Include Libraries
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <iostream>

#include <sys/neutrino.h>
#include <sys/syspage.h>

class Timer {
private:
	// Data
	int channelId, connectionId;
	char msgBuffer[100];
	uint64_t cyclePerSecond, tickCycles, tockCycles;
	timer_t timerId;

	// Structures
	struct sigevent event;
	struct itimerspec timerSpec;


public:
	// Constructor & Destructor
	Timer(uint32_t timeInSec);
	Timer(uint32_t sec, uint32_t msec);
	virtual ~Timer();
	

	// Set & Get
	int getChannelId() const;
	void setChannelId(int channelId);

	int getConnectionId() const;
	void setConnectionId(int connectionId);

	uint64_t getCyclePerSecond() const;
	void setCyclePerSecond(uint64_t cyclePerSecond);

	const struct sigevent& getEvent() const;
	void setEvent(const struct sigevent &event);

	const char* getMsgBuffer() const;

	uint64_t getTickCycles() const;
	void setTickCycles(uint64_t tickCycles);

	timer_t getTimerId() const;
	void setTimerId(timer_t timerId);

	const struct itimerspec& getTimerSpec() const;
	void setTimerSpec(uint32_t sec, uint32_t msec);

	uint64_t getTockCycles() const;
	void setTockCycles(uint64_t tockCycles);

	// Roles
	void waitTimer();
	void startTimer();
	void tick();
	double tock();

};

#endif /* TIMER_H */

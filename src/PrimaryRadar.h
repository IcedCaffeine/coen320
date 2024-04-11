#ifndef PRIMARY_RADAR_H
#define PRIMARY_RADAR_H


// C++ Libraries
#include <fstream>
#include <iostream>
#include <vector>

// C++ Headers
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Object Headers
#include "Aircraft.h"
#include "Limits.h"
#include "SecondaryRadar.h"
#include "Timer.h"

// forward declaration
class Aircraft;
class SecondaryRadar;

class PrimaryRadar {
private:
	// Data
	std::vector<void *> planePtrs;
	std::vector<std::string> waitingFileNames, flyingFileNames;
	Timer *timer;
	int numWaitingPlanes;
	int currPeriod;

	pthread_t primaryRadarThread;
	pthread_attr_t attr;
	pthread_mutex_t mutex;

	time_t at, et;
	time_t startTime,finishTime;
	int shm_waitingPlanes;
	void *waitingPlanesPtr;
	int shm_flyingPlanes;
	void *flyingPlanesPtr;
	int shm_period;
	void *periodPtr;

	// Roles
	int initialize(int numberOfPlanes);
	void *setupPrimaryRadar(void);
	void updateTimer();
	bool readWaitingPlanes();
	void writeFlyingPlanes();

public:
	PrimaryRadar(int numberOfPlanes);
	~PrimaryRadar();

	// Role
	void start();
	int stop();
	static void *startThread(void *context);

	// Set and Get
	int getCurrPeriod() const;
	void setCurrPeriod(int currPeriod);
	std::vector<std::string> getFlyingFileNames() const;
	void setFlyingFileNames(std::vector<std::string> flyingFileNames);
	int getNumWaitingPlanes() const;
	void setNumWaitingPlanes(int numWaitingPlanes);
	std::vector<void*> getPlanePtrs() const;
	void setPlanePtrs(std::vector<void*> planePtrs);
	Timer *getTimer() const;
	void setTimer(Timer *timer);
	std::vector<std::string> getWaitingFileNames() const;
	void setWaitingFileNames(std::vector<std::string> waitingFileNames);
};

#endif /* PRIMARY_RADAR_H */

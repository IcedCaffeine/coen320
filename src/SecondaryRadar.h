#ifndef SECONDARYRADAR_H
#define SECONDARYRADAR_H

// C++ Libraries
#include <fstream>
#include <iostream>
#include <vector>

// C++ Headers
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Object Headers
#include "Aircraft.h"
#include "Limits.h"
#include "Timer.h"

class Aircraft;

class SecondaryRadar {
private:
	// Data
	int numberOfPlanes, currentPeriod;
	std::vector<std::string> fileNames, flyingFileNames,waitingFileNames;

	pthread_t secondaryRadarThread;
	pthread_attr_t attr;
	pthread_mutex_t mutex;

	time_t startTime,finishTime;

	std::vector<void *> planePtrs, waitingPtrs;

	int shm_airspace, shm_period, shm_flyingPlanes;
	void *airspacePtr, *periodPtr, *flyingPlanesPtr;

	// Component object
	Timer *timer;

	// Data
	int initialize(int numberOfPlanes);
	void *setupSecondaryRadar(void);
	void updateTimer();
	bool getPlaneData();
	bool readFlyingPlanes();
	void writeFlyingPlanes();



public:
	// COnstructor & Destructor
	SecondaryRadar(int numberOfPlanes);
	~SecondaryRadar();

	// Set & Get
	static void *startSecondaryRadar(void *args);
	int getCurrentPeriod() const;
	void setCurrentPeriod(int currentPeriod);
	std::vector<std::string> getFileNames() const;
	void setFileNames(std::vector<std::string> fileNames);
	std::vector<std::string> getFlyingFileNames() const;
	void setFlyingFileNames(std::vector<std::string> flyingFileNames);
	int getNumberOfPlanes() const;
	void setNumberOfPlanes(int numberOfPlanes);
	Timer* getTimer() const;
	void setTimer(Timer *timer);
	std::vector<std::string> getWaitingFileNames() const;
	void setWaitingFileNames(std::vector<std::string> waitingFileNames);

	// Roles
	int start();
	int stop();

};

#endif /* SECONDARYRADAR_H */

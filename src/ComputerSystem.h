#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

// C++ Library
#include <fstream>
#include <list>
#include <cstdlib>
#include <vector>

// C++ Header
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>

// Object
#include "DataDisplay.h"
#include "Limits.h"
#include "Aircraft.h"
#include "SecondaryRadar.h"
#include "Timer.h"

class Aircraft; // Call Aircraft class

// prediction container
struct trajectoryPrediction {
	int id,time;
	bool keep;
	std::vector<int> posX,posY,posZ;
};

struct plane {
	int id,arrivalTime;
	int posX,posY,posZ,velX,velY,velZ;
	bool keep,moreInfo;
	int commandCounter;
};

class ComputerSystem {
private:
	/* Data */
	int planeCount,currentPeriod;
	std::vector<plane *> flyingPlanesInfo;
	std::vector<trajectoryPrediction *> trajectoryPredictions;
	std::vector<std::string> communicationNames;
	std::vector<void *> communicationPtr;

	// Timer
	Timer *timer;

	// thread members
	pthread_t computerSystemThread;
	pthread_attr_t attr;
	pthread_mutex_t mutex;

	// execution time members
	time_t startTime,finishTime;

	// shm members
	int shm_airspace,shm_period,shm_display;
	void *airspacePtr,*periodPtr,*displayPtr;

	// Roles
	int initialize();
	void *computePath();
	bool readAirspace();
	void updatePeriod(int chid);
	void clearPredictions();
	void findViolations(std::ofstream *out);
	void writeAndDisplay();

public:
	// Constructor & Destructor
	ComputerSystem(int planeCount);
	~ComputerSystem();
	
	// Set & Get
	std::vector<std::string> getCommunicationNames() const;
	void setCommunicationNames(std::vector<std::string> communicationNames);
	std::vector<void*> getCommunicationPtrs() const;
	void setCommunicationPtrs(const std::vector<void*> communicationPtr);
	int getCurrentPeriod() const;
	void setCurrentPeriod(int currentPeriod);
	std::vector<plane*> getFlyingPlanesInfo() const;
	void setFlyingPlanesInfo(const std::vector<plane*> flyingPlanesInfo);
	int getPlaneCount() const;
	void setPlaneCount(int planeCount);
	std::vector<trajectoryPrediction*> getTrajectoryPredictions() const;
	void setTrajectoryPredictions(std::vector<trajectoryPrediction*> trajectoryPredictions);
	

	// Thread
	void start();
	int stop();
	static void *startThread(void *context);

};

#endif /* COMPUTERSYSTEM_H_ */

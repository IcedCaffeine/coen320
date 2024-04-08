#ifndef COMPUTERSYSTEM_H_
#define COMPUTERSYSTEM_H_

#include <cstdlib>
#include <errno.h>
#include <fstream>
#include <list>
#include <pthread.h>
#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <time.h>
#include <vector>
#include "Aircraft.h"
#include "DataDisplay.h"
#include "Limits.h"
#include "SecondaryRadar.h"
#include "Timer.h"

class Aircraft; // forward declaration

// prediction container
struct trajectoryPrediction {
	int id;
	int time;

	std::vector<int> posX;
	std::vector<int> posY;
	std::vector<int> posZ;

	bool keep; // keep for next iteration
};

struct plane {
	int id;
	int arrivalTime;
	int posX,posY,posZ;
	int velX,velY,velZ;
	bool keep; // keep for next iteration
	bool moreInfo;
	int commandCounter;
};

class ComputerSystem {
private:
	/* Data */
	int planeCount;
	int currentPeriod;
	std::vector<plane *> flyingPlanesInfo;
	std::vector<trajectoryPrediction *> trajectoryPredictions;
	std::vector<void *> communicationPtr;
	std::vector<std::string> communicationNames;

	// Timer
	Timer *timer;

	// thread members
	pthread_t computerSystemThread;
	pthread_attr_t attr;
	pthread_mutex_t mutex;

	// execution time members
	time_t startTime;
	time_t finishTime;

	// shm members
	int shm_airspace;
	void *flyingPlanesPtr;
	int shm_period;
	void *periodPtr;
	int shm_display;
	void *displayPtr;

	// Roles
	int initialize();
	void *calculateTrajectories();
	bool readAirspace();
	void cleanPredictions();
	void computeViolations(std::ofstream *out);
	void writeAndDisplay();
	void updatePeriod(int chid);

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

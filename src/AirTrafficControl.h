#ifndef AIR_TRAFFIC_CONTROL_H
#define AIR_TRAFFIC_CONTROL_H

// C++ Library
#include <fstream>
#include <string>
#include <list>
#include <vector>

// C++ Header
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <vector>

// Objects Header
#include "Aircraft.h"
#include "PrimaryRadar.h"
#include "SecondaryRadar.h"
#include "ComputerSystem.h"
#include "DataDisplay.h"
#include "Limits.h"
#include "Timer.h"

class AirTrafficControl {
private:
	// Components

	  PrimaryRadar *primaryRadar;
	  SecondaryRadar *secondaryRadar;
	  DataDisplay *dataDisplay;
	  ComputerSystem *computerSystem;
	  std::vector<Aircraft *> planes; // vector of plane objects

	  // timers
	  time_t startTime;
	  time_t endTime;

	  // shm
	  int shm_waitingPlanes,shm_flyingPlanes,shm_period,shm_display,shm_airspace;
	  void *waitingPtr,*flyingPlanesPtr,*periodPtr,*displayPtr,*airspacePtr;

	  // thread
	  pthread_mutex_t mutex;

protected:
	int readInput();
	int initialize();

public:
	// Constructor and Destructor
	AirTrafficControl();
	~AirTrafficControl();

	// Set & Get
	ComputerSystem *getComputerSystem() const;
	void setComputerSystem(ComputerSystem *computerSystem);
	DataDisplay* getDataDisplay() const;
	void setDataDisplay(DataDisplay *dataDisplay);
	std::vector<Aircraft*> getPlanes() const;
	void setPlanes(std::vector<Aircraft*> planes);
	PrimaryRadar* getPrimaryRadar() const;
	void setPrimaryRadar(PrimaryRadar *primaryRadar);
	SecondaryRadar* getSecondaryRadar() const;
	void setSecondaryRadar(SecondaryRadar *secondaryRadar);

	// Role
	int start();

};

#endif /* AIR_TRAFFIC_CONTROL_H */

#ifndef ATC_H_
#define ATC_H_

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <list>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <time.h>
#include <vector>
#include "Aircraft.h"

#include "ComputerSystem.h"
#include "DataDisplay.h"
#include "Limits.h"
#include "PrimaryRadar.h"
#include "SecondaryRadar.h"
#include "Timer.h"

class ATC {
private:
	// Components
	  std::vector<Aircraft *> planes; // vector of plane objects
	  PrimaryRadar *primaryRadar;
	  SecondaryRadar *secondaryRadar;
	  DataDisplay *dataDisplay;
	  ComputerSystem *computerSystem;

	  // timers
	  time_t startTime;
	  time_t endTime;

	  // shm
	  int shm_waitingPlanes;
	  void *waitingPtr;
	  int shm_flyingPlanes;
	  void *flyingPlanesPtr;
	  int shm_airspace;
	  void *airspacePtr;
	  int shm_period;
	  void *periodPtr;
	  int shm_display;
	  void *displayPtr;

	  // thread
	  pthread_mutex_t mutex;


public:
	// Constructor and Destructor
	ATC();
	~ATC();

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


protected:
  int readInput();
  int initialize();
};

#endif /* ATC_H_ */

#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

#include "DataDisplay.h"
#include "Communication_System.h"
#include "DataDisplay.h"
#include "Aircraft.h"

#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include "Structure.h"
#include <pthread.h>


class Computer_System{
public:
	int calculationInterval;

	// Computer system thread
	pthread_t Computer_System_Thread();

	// Thread body
	void * Computer_System_Main(void * arg);
	// Functions to change parameters
	void changeSpeed(Aircraft aircraft, double s);
	void changeAltitude(Aircraft aircraft, int a);
	void changePosition(Aircraft aircraft, float orient);
};

// Prototypes
void * Computer_System_Main(void *arg);
void addingAirplane();
void removingAirplane();
void radarUpdate();
void collisionCheck();
void collisionCallback(std::string message);


#endif

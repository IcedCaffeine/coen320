#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

#include "DataDisplay.h"
#include <pthread.h>


class Computer_System{
public:
	int calculationInterval;

	// Computer system thread
	pthread_t Computer_System_Thread();

	// Thread body
	void * Computer_System_Main(void * arg);
	// Functions to change parameters
	void changeSpeed(int id, double s);
	void changeAltitude(int id, int a);
	void changePosition(int id, float orient);
};


#endif

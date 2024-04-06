#ifndef COMMUNICATION_SYSTEM_H
#define COMMUNICATION_SYSTEM_H

#include "DataDisplay.h"
#include <pthread.h>


class Communication_System{
public:
	int calculationInterval;

	// Computer system thread
	pthread_t Computer_System_Thread();

	// Thread body
	void * Computer_System_Main(void * arg);
};


#endif

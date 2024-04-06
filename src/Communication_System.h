#ifndef COMMUNICATION_SYSTEM_H
#define COMMUNICATION_SYSTEM_H

#include <pthread.h>
#include <vector>
#include "Aircraft.h"
#include "Computer_System.h"

class Communication_System{
public:
	Communication_System();
	pthread_t createCommunication();

private:
	void * communicationMain();

	void sendMessage();

	void disconnectFromChannel(int coid);

	Communication_System AircraftVector(std::vector<Aircraft> p);




};


#endif

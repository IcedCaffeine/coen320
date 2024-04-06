#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

#include <pthread.h>
#include <vector>
#include "Communication_System.h"
#include "Aircraft.h"

class Computer_System{
public:
	Computer_System();
	pthread_t createCommunication();

	// Functions to change parameters
	void changeSpeed(int id, double s);
	void changeAltitude(int id, int a);
	void changePosition(int id, float orient);

private:
	void * communicationMain();

	void sendMessage(int chid, const void *msg, int size);

	void disconnectFromChannel(int coid);

	Communication_System AircraftVector(std::vector<Aircraft> p);




};


#endif

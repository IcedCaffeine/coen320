#ifndef COMMUNICATION_SYSTEM_H
#define COMMUNICATION_SYSTEM_H

#include <pthread.h>
#include <vector>
#include "Aircraft.h"
#include "Computer_System.h"

class Communication_System {
public:
	Communication_System();

};

pthread_t createCommunicationThread();

void * communicationMain();
void sendMessage(std::string fileName);
void disconnectFromChannel(int coid);
Communication_System AircraftVector(std::vector<Aircraft> p);

#endif

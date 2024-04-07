#ifndef AIRCRAFT_H_
#define AIRCRAFT_H_

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "Limits.h"
#include "Timer.h"

class Aircraft {
private:
	// Data
	int arrivalTime;
	int ID;
	int x,y,z;
	int speedX,speedY,speedZ;

	int commandCounter;
	bool commandInProgress;

	//Threads
	pthread_t thread;
	pthread_attr_t attr;
	pthread_mutex_t mutex;

	// Time members
	time_t startTime;
	time_t finishTime;

	// shm
	int shm_fd;
	void *ptr;
	std::string planeMessage;
	std::string fileName;

	// Roles
	int initialize(); // initialize thread and shm
	void *flyPlane(void); // Update Plane Position & Speed
	void answerComm(); // Check Comms for Commands
	void updatePosition(); // Update position
	void toString();
	int checkLimits(); // check airspace limits for operation termination
	void print(); // print plane info

public:
	// Constructor & Destructor
	Aircraft(int ID, int arrivalTime, int position[3], int speed[3]);
	~Aircraft();

	// Set & Get
	int getArrivalTime() const;
	void setArrivalTime(int arrivalTime);
	int getCommandCounter() const;
	void setCommandCounter(int commandCounter);
	bool isCommandInProgress() const;
	void setCommandInProgress(bool commandInProgress);
	std::string getFileName() const;
	void setFileName(std::string fileName);
	int getId() const;
	void setId(int id);
	std::string getPlaneMessage() const;
	void setPlaneMessage(std::string planeMessage);
	int getSpeedX() const;
	void setSpeedX(int speedX);
	int getSpeedY() const;
	void setSpeedY(int speedY);
	int getSpeedZ() const;
	void setSpeedZ(int speedZ);
	int getX() const;
	void setX(int x);
	int getY() const;
	void setY(int y);
	int getZ() const;
	void setZ(int z);

	// Role
	int start();  // start thread
	bool stop(); // join execution thread
	static void *startThread(void *context); // start Execution of Thread
	const char *getFD();


};
#endif /* AIRCRAFT_H_ */

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

// C++ Libraries
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <vector>
#include <string>
#include <sys/neutrino.h>

// Class Objects
#include "Timer.h"

// Definitons
#define ATTACH_POINT "my_channel"
#define MIN_AIRSPACE_X_REGION 0
#define MAX_AIRSPACE_X_REGION 100000
#define MIN_AIRSPACE_Y_REGION 0
#define MAX_AIRSPACE_Y_REGION 100000
#define MIN_AIRSPACE_Z_REGION 15000
#define MAX_AIRSPACE_Z_REGION 25000

// Structures
typedef struct _pulse msg_header_t;

typedef struct Data {
	msg_header_t hdr;
	double data;
} Data_t;

typedef struct Plane_Data {
	msg_header_t hdr;
	int id;
	int velocity[3];
	int position[3];
    bool outsideAirspace;
    bool ArrivedYet;
} Plane_Data_t;

typedef struct Time_Data {
	msg_header_t hdr;
	int time;
} Time_Data_t;

// doube t=0;

class Aircraft{
	private:
		int posX,posY,posZ;
		int velX,velY,velZ;
		int aircraftId;
		int arrivalTime;

	public:
		Aircraft();
		Aircraft(int aircraftId, int posX, int posY, int posZ, int velX, int velY, int velZ, int arrivalTime);
		~Aircraft();

		int getAircraftId() const;
		void setAircraftId(int aircraftId);
		int getArrivalTime() const;
		void setArrivalTime(int arrivalTime);
		int getPosX() const;
		void setPosX(int posX);
		int getPosY() const;
		void setPosY(int posY);
		int getPosZ() const;
		void setPosZ(int posZ);
		int getVelX() const;
		void setVelX(int velX);
		int getVelY() const;
		void setVelY(int velY);
		int getVelZ() const;
		void setVelZ(int velZ);

		void updatePosition();
		bool outsideAirSpace();
		void ThreadPlaneTimerStart();
		void update();
		bool IsCollidingWith(Aircraft& otherAircraft);
};

std::vector<Aircraft> readAircraftFromFile(std::string fileName);
void* AircraftClientThread(void *);
void* TimerReceiverThread(void *);

#endif /* AIRCRAFT_H */

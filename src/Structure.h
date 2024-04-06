#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

typedef struct _pulse msg_header_t;

typedef enum {
	OperatorSetAirplaneSpeed,
	OperatorRequestAirplaneData,
	AirplaneRadarUpdate,
	ClockTimerUpdate,
	AddAirplane,
	RemoveAirplane
} MsgToCommSysType;

typedef enum {
	CommSysSetAirplaneSpeed,
	CommSysRequestAirplaneData,
	ClockTimeUpdate,
	AirplaneScan
} AirplaneMsgType;

typedef struct  {
	msg_header_t hdr;
	MsgToCommSysType type;
	int id;
	float posX, posY, posZ;
	float velX, velY, velZ;
} MsgToComputerSys;

typedef struct  {
	msg_header_t hdr;
	AirplaneMsgType type;
	int id;
	float posX, posY, posZ;
	float velX, velY, velZ;
} MsgToAirplane;



#endif

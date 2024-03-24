#ifndef RADAR_H
#define RADAR_H

/* C++ Library */
#include <vector>
#include <array>
#include <map>

#include <stdio.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>


/* Definitions */
#define CHANNEL "channel"

#define MIN_X_REGION 0
#define MIN_Y_REGION 0
#define MIN_Z_REGION 15000

#define MAX_X_REGION 100000
#define MAX_Y_REGION 100000
#define MAX_Z_REGION 25000

#define ROW 10
#define COLUMN 10
#define DEPTH 10

/* Global */
int grid[ROW][COLUMN][DEPTH];

/* Structures */
typedef struct _pulse msg_header_t;

struct MessageData{
	msg_header_t header;
	double data;
};

struct RadarData{
	msg_header_t header;
	int id;
	int positionX, positionY, positionZ;
	int velocityX, velocityY, velocityZ;
	bool outsideAirSpace;
	bool arrivedyet;
};

class Radar {
private:
public:
	Radar();
	virtual ~Radar();
};

#endif /* RADAR_H */

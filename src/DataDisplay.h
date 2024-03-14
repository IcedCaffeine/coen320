#ifndef DATADISPLAY_H
#define DATADISPLAY_H

/* Include Libraries */
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/neutrino.h>

/* Class Object */
#include "Aircraft.h"

/* Defintions */
#define ROW 50
#define COLUMN 50
#define CELLSIZE 25

#define WARNING 1
#define PLANE 2
#define LOG 3
#define GRID 4


class DataDisplay {
	// Structures
	typedef struct Plane{
		std::vector<Aircraft> planes;
		Aircraft onePlane;
		int command;
	}planeMessage;

private:
	// Data
	int channelId, fileId;

	// Roles
	std::string makeGrid(std::vector <Aircraft> aircrafts);

public:
	// Constructor
	DataDisplay();
	virtual ~DataDisplay();

	// Set & Get
	int getChannelId() const;
	void setChannelId(int channelId);
	int getFileId() const;
	void setFileId(int fileId);
	
	// Role
	void run();
	void getMessage();


};

#endif /* SRC_DATADISPLAY_H_ */

#ifndef DATADISPLAY_H_
#define DATADISPLAY_H_

#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#include "Limits.h"
#include "Timer.h"

// Global Variables
const int blockCount = (int)MARGIN / (int)SCALER + 1;

class DataDisplay {
private:
	// execution time members
	time_t startTime;
	time_t finishTime;

	// threads
	pthread_t dataDisplayThread;
	pthread_attr_t attr;
	pthread_mutex_t mutex; // mutex for display

	// Temporary values
	std::string grid[blockCount][blockCount] = {{""}}; // Shrink 100k by 100k map to 10 by 10, each block is 10k by 10k
	std::string displayedHeight = "";

	// shm members
	int shm_display; // Display required info
	void *ptr_display;

public:
	// Constructor & Destructor
	DataDisplay();
	~DataDisplay();
	
	// Set & Get
	std::string getDisplayedHeight() const;
	void setDisplayedHeight(std::string displayedHeight);
	void setGrid(int row, int column, std::string value);

	// Roles
	int initialize();
	void start();
	int stop();
	static void *startDisplay(void *context);
	void *updateDisplay(void);
	void displayMap();
	void resetMap();
};

#endif /* DATADISPLAY_H_ */

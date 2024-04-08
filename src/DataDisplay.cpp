#include "DataDisplay.h"

#include <iostream>

DataDisplay::DataDisplay() {
	initialize(); // Construction Initialization Call
}

DataDisplay::~DataDisplay() {
	shm_unlink("display"); // Unlink shared memory on termination
}

int DataDisplay::initialize() {
	/*Make threads in detached state*/
	int rc = pthread_attr_init(&attr);
	if (rc) {
		printf("ERROR, RC from pthread_attr_init() is %d \n", rc);
	}

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (rc) {
		printf("ERROR; RC from pthread_attr_setdetachstate() is %d \n", rc);
	}

	/*Create shared memory for display*/
	// open list of waiting planes shm
	shm_display = shm_open("display", O_RDWR, 0666);
	if (shm_display == -1) {
		perror("in shm_open() Display");
		exit(1);
	}

	ptr_display = mmap(0, SIZE_SHM_DISPLAY, PROT_READ | PROT_WRITE, MAP_SHARED, shm_display, 0);

	if (ptr_display == MAP_FAILED) {
		perror("in map() Display");
		exit(1);
	}

	return 0;
}

void DataDisplay::start() {
	time(&startTime);
	if (pthread_create(&dataDisplayThread, &attr, &DataDisplay::startDisplay, (void *)this) != EOK) {
		dataDisplayThread = 0;
	}
}

int DataDisplay::stop() {
	pthread_join(dataDisplayThread, NULL); // Join thread on stop

	return 0;
}

void *DataDisplay::startDisplay(void *context) {
	((DataDisplay *)context)->updateDisplay();
	return NULL;
}

void *DataDisplay::updateDisplay(void) {
	// Link to channel
	int channelId = ChannelCreate(0);
	if (channelId == -1) {
		std::cout << "couldn't create display channel!\n";
	}

	// Setup period of 5 seconds
	Timer timer(channelId);
	timer.setTimer(DATA_DISPLAY_PERIOD, DATA_DISPLAY_PERIOD);

	// Receive data
	int receiveId;
	Message message;
	int logging_counter = 1;
	std::ofstream out("log");

	while (true) {
		if (receiveId == 0) {
			pthread_mutex_lock(&mutex);

			// Axis and Altitude
			int axis = 0;
			int z = 0; // 0=ID, 1=X, 2=Y, 3=Z, 4= Height;
			std::string x = "";
			std::string y = "";

			// Parsing buffers
			std::string buffer = "";
			std::string displayBit = "";
			std::string id = "";

			// Read from shared memory pointer
			for (int i = 0; i < SIZE_SHM_DISPLAY; i++) {
				// Read Character
				char readChar = *((char *)ptr_display + i);
				if (readChar == 't') {
					std::cout << "display done\n";
					time(&finishTime);
					double execTime = difftime(finishTime, startTime);
					std::cout << "display execution time: " << execTime << " seconds\n";
					ChannelDestroy(channelId);
					out.close();
					return 0;
				}

				// Check Ending Character
				if (readChar == ';') {
					break;
				}
				else if(readChar == ','){
					// Update Based on Buffer
					if (buffer.length() > 0) {
						switch (axis) {
						case 0:
							id = buffer;
							break;
						case 1:
							x = buffer;
							break;
						case 2:
							y = buffer;
							break;
						case 3:
							z = stoi(buffer);
							z += ALTITUDE;
							break;
						case 4:
							displayBit = buffer;
							break;
						}
					}
					axis++;
					buffer = "";
				}
				else if(readChar == '/'){
					// Update Based on Buffer
					if (buffer.length() > 0) {
						switch (axis) {
						case 0:
							id = buffer;
							break;
						case 1:
							x = buffer;
							break;
						case 2:
							y = buffer;
							break;
						case 3:
							z = stoi(buffer);
							z += ALTITUDE;
							break;
						case 4:
							displayBit = buffer;
							break;
						}
					}

					// Add id
					if(this->grid[(100000 - stoi(y)) / SCALER][stoi(x) / SCALER] == ""){
						this->setGrid((100000 - stoi(y)) / SCALER, stoi(x) / SCALER, id);
					}
					else{
						this->setGrid((100000 - stoi(y)) / SCALER, stoi(x) / SCALER, "\\" + id);
					}

					// Update based on Display COntrol
					if(displayBit == "1"){
						this->getDisplayedHeight() += "Plane " + id;
						this->getDisplayedHeight() += " has height of " + std::to_string(z);
						this->getDisplayedHeight()	+= " ft\n";
					}
					x=""; y=""; z=0;
					displayBit = ""; axis = 0; buffer = "";
				}
				else{
					buffer += readChar; // Load Buffer with Character
				}
			}
			pthread_mutex_unlock(&mutex);

			// Logging the airspace into a log file
			if (logging_counter == 6) {
				std::cout << "Logging current airspace..." << std::endl;
				std::streambuf *coutbuf = std::cout.rdbuf(); // save old buf
				std::cout.rdbuf(out.rdbuf()); // redirect std::cout to out.txt!
				this->displayMap(); // Display map and height command
				std::cout << std::endl;

				std::cout.rdbuf(coutbuf); // reset to standard output again
				logging_counter = 1;
			}

			// if not during a multiple of 30 second period, just print to stdout normally
			else {
				this->displayMap(); // Display map and height command
			}
			logging_counter++;

			this->setDisplayedHeight(""); // Reset buffer

			// Reset map array
			this->resetMap();
		}
		receiveId = MsgReceive(channelId, &message, sizeof(message), NULL);
	}
	ChannelDestroy(channelId);
	return 0;
}


// Printing grid
void DataDisplay::displayMap() {
	// Display Values in Grid
	for (int i = 0; i < blockCount; i++) {
		for (int j = 0; j < blockCount; j++) {
			// Print Empty block if no item
			if (this->grid[i][j] == "") {
				std::cout << "_|";
			}
			else {
				// print plane ID if there are items
				std::cout << this->grid[i][j] << "|";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	// Display height
	printf("%s\n\n", this->getDisplayedHeight().c_str());
}

void DataDisplay::resetMap(){
	for (int i = 0; i < blockCount; i++) {
		for (int j = 0; j < blockCount; j++) {
			this->grid[i][j] = "";
		}
	}
}

std::string DataDisplay::getDisplayedHeight() const {
	return displayedHeight;
}

void DataDisplay::setDisplayedHeight(std::string displayedHeight) {
	this->displayedHeight = displayedHeight;
}

void DataDisplay::setGrid(int row, int column, std::string value){
	this->grid[row][column] = value;
}
